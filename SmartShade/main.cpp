
/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/

#include "mbed.h"
#include "TextLCD.h"
#include "TMP102.h"
#include "APDS9960_I2C.h"
#include "ServoMotor.h"

#define TEMPERATURE_SAMPLING_TIME_S         5.
#define SAMPLING_TIME_MS                    100
#define IDLE_TIME_FOR_TEMP_CONTROL_S        10.
#define SET_COMFTEMP_TIMEOUT_TIME_S         3.

#define SS_DEBUG

//
// State used to remember previous characters read in a button message from BLE
//
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};
statetype state = start;

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
//Serial          pc(USBTX,USBRX);
Ticker          tkTemp;
Timer           timerCmd;
Timeout         setComfTemp;
InterruptIn     gsInt(p6);

//
// Prototype for functions
//
void tempToLCD(float temp);
void strToLCD(char *string);
void motionToLCD(void);
void driveMotor(void);

bool prepareGestSensor(void);

//
// Interrupt Service Routines
//
void task_ReadTemperature(void);
void task_updateComfTemp(void);
void task_Movement(void);
void task_ReadBLE(void);

/**
 * Print temperature reading to LCD
 *
 */
void tempToLCD(float temp)
{
    LCD.locate(0,1);
    LCD.printf("%.1fF",temp);
    LCD.locate(8,1);
    LCD.printf("(%.1fF)",comfTempF);
    /*
    LCD.cls();
    LCD.programCharacter(0, customChars[0]);
    */
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
        strcpy(dir, "    L-Paused    ");
        break;
    case DIR_RIGHT:
        strcpy(dir, "    R-Paused    ");
        break;
    case DIR_UP:
        strcpy(dir, "   Roll Up      ");
        break;
    case DIR_DOWN:
        strcpy(dir, "   Roll Down    ");
        break;
    case DIR_NEAR:
    case DIR_NONE:
        strcpy(dir, "* Smart Shade * ");
        break;
    case DIR_FAR:
        strcpy(dir, "    * Bye *     ");
        break;
    case DIR_HOT:
        strcpy(dir, " Hot->Roll Down ");
        break;
    default:
        strcpy(dir, "  Unknown Dir!  ");
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

    return (true);
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
            motion = DIR_LEFT;          // No matter
            SvoMotor.Pause();           // pause motor in the ISR (only set up stop bit of SvoMotor
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
        }
    }

    // TBD: backlight control based on proximity (not reliable yet)
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

    // Reset LCD screen
    LCD.cls();

    // Initialize Temperature Sensor
    TempSensor.Initialize();
    tempF = TempSensor.GetTemperatureInF();
    comfTempF = TempSensor.GetComfTempInF();

    // Setup interrupt for gesture sensor
    gsInt.mode(PullUp);
    gsInt.fall(&task_Movement);

    // Initialize Gesture Sensor
    prepareGestSensor();

    // Sample temperature every TEMPERATURE_SAMPLING_TIME_S second
    tkTemp.attach(&task_ReadTemperature, TEMPERATURE_SAMPLING_TIME_S);
    timerCmd.start();

    // Set up serial interrupt for Bluefruit
    BLE.attach(&task_ReadBLE, Serial::RxIrq);

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
