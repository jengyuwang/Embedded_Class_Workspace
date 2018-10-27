
/* Smart Shade Project uses mbed controller with GCC compiler
 * Capability:
 * -> Roll up / down / pause the Roman shade by either hand gesture or through smart device with Bluetooth control (U, D, L, R)
 * -> LCD to display status (current temperature, shade is rolling up or down, etc.)
 * -> Automatically roll down the shape when temperature exceeds the preset comfortable temperature
 * -> User can change the comfortable temperature through Bluetooth.
 *    The configured value will be saved to the flash and still applied when power cycled.
 *
*/

#include "mbed.h"
#include "TextLCD.h"
#include "TMP102.h"
#include "APDS9960_I2C.h"
#include "ServoMotor.h"

#define TEMPERATURE_SAMPLING_TIME_S         5.
#define SAMPLING_TIME_MS                    100
#define PAUSE_SHADE_WAIT_TIME_MS            1000
#define IDLE_TIME_FOR_TEMP_CONTROL_S        10.
#define SET_COMFTEMP_TIMEOUT_TIME_S         3.
#define NUM_OF_CUSTOM_CHARS                 6

//
// State used to remember previous characters read in a button message from BLE
//
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};
statetype state = start;


//
// Custom characters to be displayed to LCD
//
unsigned int customChars[NUM_OF_CUSTOM_CHARS][8] = {
{0x00, 0x04, 0x1F, 0x0E, 0x0E, 0x0A, 0x11, 0x00},   // 0x01 Star
{0x00, 0x0A, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00},   // 0x02 Heart
{0x0C, 0x12, 0x12, 0x0C, 0x00, 0x00, 0x00, 0x00},   // 0x03 Degree sign
{0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x04},   // 0x04 Up arrow
{0x04, 0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04},   // 0x05 Down arrow
{0x08, 0x04, 0x06, 0x0E, 0x1F, 0x1F, 0x0E, 0x04}    // 0x06 Fire sign
};


//
// Global variables for main and interrupt routine
//
volatile bool   button_ready = 0;
volatile bool   no_cmd = 1;
volatile bool   setNewComfTemp = 0;
volatile bool   needToSaveFlash = 0;

volatile int    bnum = 0;
volatile int    bhit;
volatile int    motion = DIR_NONE;

volatile float  comfTempF = 75.0;
volatile float  sign = 1.;
volatile float  comfTempAdj = 0.;
volatile float  tempF = 0.;


//
// Object Declaration
//
TextLCD         LCD(p12, p20, p21, p22, p23, p24); // rs, e, d4-d7
TMP102          TempSensor(p9, p10);
APDS9960_I2C    GestSensor(p9, p10);
ServoMotor      SvoMotor(p26);

Serial          BLE(p28, p27);
Serial          pc(USBTX,USBRX);
Ticker          tkTemp;
Timer           timerCmd;
Timeout         setComfTemp;
InterruptIn     gsInt(p6);


//
// Prototype for functions
//
void prepareLCD(void);
void prepareTempSensor(void);
bool prepareGestSensor(void);
void prepareHWInterrupts(void);

void tempToLCD(float temp);
void strToLCD(char *string);
void motionToLCD(void);
void driveMotor(void);


//
// Interrupt Service Routines
//
void task_ReadTemperature(void);
void task_updateComfTemp(void);
void task_Movement(void);
void task_ReadBLE(void);

/**
* Prepare interrupts, timer, and ticker
*
*/
void prepareHWInterrupts(void)
{
    // Setup interrupt for gesture sensor
    gsInt.mode(PullUp);
    gsInt.fall(&task_Movement);

    // Set up serial interrupt for Bluefruit
    BLE.attach(&task_ReadBLE, Serial::RxIrq);
}

/**
 * Prepare LCD including custom character setup
 *
 */
void prepareLCD(void)
{
    for (int i = 0; i < NUM_OF_CUSTOM_CHARS; i++)
    {
        LCD.programCharacter(i+1, customChars[i]);
    }
    LCD.cls();
}

/**
 * Prepare temperature sensor
 *
 */
void prepareTempSensor(void)
{
    TempSensor.Initialize();
    tempF = TempSensor.GetTemperatureInF();
    comfTempF = TempSensor.GetComfTempInF();
}

/**
 * Prepare gesture sensor. Print error message to LCD if needed
 *
 * @return bool    success (true) or fail (false)
 */
bool prepareGestSensor(void)
{
    char errMsg[16];

    if (GestSensor.init() == false)
    {
        strcpy(errMsg, "  Err init GS ! ");
        strToLCD(errMsg);
        return (false);
    }

    if (GestSensor.enableGestureSensor(true) == false)
    {
        strcpy(errMsg, "  Err enbl GS ! ");
        strToLCD(errMsg);
        return (false);
    }

    GestSensor.readGesture();

    return (true);
}

/**
 * Prepare timer and ticker
 *
 */
void prepareTimerAndTicker(void)
{
    // Sample temperature every TEMPERATURE_SAMPLING_TIME_S second
    tkTemp.attach(&task_ReadTemperature, TEMPERATURE_SAMPLING_TIME_S);

    timerCmd.start();
}

/**
 * Print temperature reading to LCD
 *
 */
void tempToLCD(float temp)
{
    LCD.printf("%.1f\337F",temp);
    LCD.locate(8,1);
    LCD.printf("(%.1f\337F)",comfTempF);
}

/**
 * Print general string to LCD
 *
 */
void strToLCD(char *string)
{
    LCD.locate(0,0);
    LCD.printf("%s", string);
}

/**
 * Print smart shade action to LCD
 *
 */
void motionToLCD(void)
{
    char dir[16];

     switch (motion)
     {
     case DIR_LEFT:
         sprintf(dir, "    L-Paused    ");
         break;
     case DIR_RIGHT:
         sprintf(dir, "    R-Paused    ");
         break;
     case DIR_UP:
         sprintf(dir, "    Roll Up     ");
         dir[1] = 4;
         dir[13] = 4;
         break;
     case DIR_DOWN:
         sprintf(dir, "    Roll Down   ");
         dir[1] = 5;
         dir[15] = 5;
         break;
     case DIR_NEAR:
     case DIR_NONE:
         sprintf(dir, "* Smart Shade * ");
         break;
     case DIR_FAR:
         sprintf(dir, "    * Bye *     ");
         dir[4] = 2;
         dir[10] = 2;
         break;
     case DIR_HOT:
         sprintf(dir, "Hot  Roll Down  ");
         dir[3] = 6;
         dir[15] = 5;
         break;
     default:
         sprintf(dir, "  Unknown Dir!  ");
         break;
     }

     strToLCD(dir);
     tempToLCD(tempF);
}

/**
 * Drive motor if needed. Update smart shade action to LCD accordingly
 *
 */
void driveMotor(void)
{
    bool resetTimer = true;

    motionToLCD();

    switch (motion)
    {
    case DIR_UP:
        SvoMotor.RollUp();
        break;
    case DIR_DOWN:
    case DIR_HOT:
        SvoMotor.RollDown();
        break;
    case DIR_LEFT:
    case DIR_RIGHT:
        // To show pause status on LCD since SvoMotor is not running and exit fase
        wait_ms(PAUSE_SHADE_WAIT_TIME_MS);
        break;
    default:
        // Do nothing
        resetTimer = false;
        break;
    }

    motion = DIR_NONE;

    if (resetTimer == true)
    {
        // reset timer only when shade is just rolled up or down
        timerCmd.reset();
        timerCmd.start();
    }
}

/**
 * Update comfort temperature (accumulated effect)
 *
 */
void ComfTempUpdate(void)
{
    if (bnum == '1')
    {
        sign = -1.;
    }
    else
    {
        comfTempAdj += ((uint8_t)bnum - 48);    // convert '2' to 2, etc.
    }
}

/**
 * Get BLE input and react accordingly
 *
 */
void checkBLEControl(void)
{
    if (button_ready)
    {
        if (bnum == '5')
            motion = DIR_UP;
        else if (bnum == '6')
            motion = DIR_DOWN;
        else if ((bnum == '7') || (bnum == '8'))
        {
            motion = (bnum == '7') ? DIR_LEFT : DIR_RIGHT;          // No matter
            SvoMotor.Pause();           // pause motor in the ISR (only set up stop bit of SvoMotor
            motionToLCD();
        }
        else if (bhit == '1')
        {
            if ((bnum == '1') || (bnum == '2') || (bnum == '3') || (bnum == '4'))
            {
                // number pushed for comfortable temperature setting
                ComfTempUpdate();
                if (setNewComfTemp == false)
                {
                    setNewComfTemp = true;
                    setComfTemp.attach(&task_updateComfTemp, SET_COMFTEMP_TIMEOUT_TIME_S);
                }
            }
        }

        button_ready = 0;               //reset flag after reading button message
    }
}

/**
 * Timeout ISR to update comfort temperature
 *
 */
void task_updateComfTemp(void)
{
    comfTempF += (sign) * comfTempAdj;
    needToSaveFlash = true;

    // Reset the adjustment parameters
    sign = 1.0;
    comfTempAdj = 0.0;
    setNewComfTemp = false;
}

/**
 * Ticker ISR to read the temperature and check if exceeding the comfort level.
 *
 */
void task_ReadTemperature(void)
{
    tempF = TempSensor.GetTemperatureInF();

    if (tempF > comfTempF)
    {
        if (timerCmd.read() > IDLE_TIME_FOR_TEMP_CONTROL_S)
        {
            if (SvoMotor.GetCurrentState() != down)
            {
                motion = DIR_HOT;
            }
        }
    }
}

/**
 * ISR to read the gesture sensor.
 *
 */
void task_Movement(void)
{
    uint8_t proximity = 0;

    if (GestSensor.isGestureAvailable())
    {
        motion = GestSensor.readGesture();

        if ((motion == DIR_LEFT) || (motion == DIR_RIGHT))
        {
             SvoMotor.Pause();           // pause motor in the ISR (only set up stop bit of SvoMotor
             motionToLCD();
        }
    }

    // TBD: backlight control based on proximity (not reliable yet) -> set pulsewidth_ms on backLight pin
    GestSensor.readProximity(proximity);
}

/**
 * Serial interrupt service routine for BLE.
 *
 */
void task_ReadBLE(void)
{
    switch (state)
    {
    case start:
        if (BLE.getc()=='!') state = got_exclm;
        else state = start;
        break;
    case got_exclm:
        if (BLE.getc() == 'B') state = got_B;
        else state = start;
        break;
    case got_B:
        bnum = BLE.getc();
        state = got_num;
        break;
    case got_num:
        bhit = BLE.getc();
        state = got_hit;
        break;
    case got_hit:
        if (BLE.getc() == char(~('!' + ' B' + bnum + bhit))) button_ready = 1;
        state = start;
        break;
    default:
        BLE.getc();
        state = start;
    }

    checkBLEControl();
}

/**
 * Initialize and start the kernel.
 *
 */
int main()
{
    //pc.baud(9600);
    //pc.printf("\r\n Starting Smart Shade Test...\r\n\r\n");

    // Prepare HW interrupt first
    prepareHWInterrupts();

    // Reset LCD screen
    prepareLCD();

    // Initialize Temperature Sensor
    prepareTempSensor();

    // Initialize Gesture Sensor
    prepareGestSensor();

    // Setup all the timer and ticker
    prepareTimerAndTicker();

    while(1)
    {
        // Long latency job below
        // Operate motor based on motion commanded
        driveMotor();

        // Update flash if needed
        if (needToSaveFlash)
        {
            TempSensor.SetConmfTempInF(comfTempF);
            needToSaveFlash = false;
        }

        wait_ms(SAMPLING_TIME_MS);
    }
}
