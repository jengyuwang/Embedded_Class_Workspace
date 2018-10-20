
/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"
#include "TextLCD.h"
#include "TMP102.h"
#include "APDS9960_I2C.h"
#include "ServoMotor.h"
//#include "Bluefruit.h"

#define TEMPERATURE_SAMPLING_TIME_S         5.
#define SAMPLING_TIME_MS                    100
#define IDLE_TIME_FOR_TEMP_CONTROL_S        10.

#define SS_DEBUG


// State used to remember previous characters read in a button message from BLE
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};
statetype state = start;


// Global variables for main and interrupt routine
volatile bool   button_ready = 0;
volatile bool   no_cmd = 1;
volatile int    bnum = 0;
volatile int    bhit;
volatile float  confTempF = 75.0;


// Object Declaration
TextLCD         LCD(p12, p20, p21, p22, p23, p24); // rs, e, d4-d7

TMP102          TempSensor(p9, p10);
APDS9960_I2C    GestSensor(p9, p10);
ServoMotor      SvoMotor(p26);
//Bluefruit BLE(p27, p28);

Serial          BLE(p28, p27);
Serial          pc(USBTX,USBRX);
Ticker          tkTemp;
Timer           timerCmd;
InterruptIn     gsInt(p5);

volatile float  tempF = 0.;
volatile bool   bleDataAvailable = false;

void helloToLCD(void)
{
    LCD.cls();
    LCD.printf("Hello, Jeng-Yu");
}

void tempToLCD(float temp)
{
    //LCD.cls();
    LCD.locate(10,1);
    LCD.printf("%.2fF",temp);
}

void errToLCD(char *string)
{
    LCD.cls();
    LCD.printf("%s", string);
}

void task_ReadTemperature(void)
{
    tempF = TempSensor.GetTemperatureInF();
}

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
}


void motionToLCD(char *string)
{
    LCD.cls();
    LCD.printf("%s", string);
}

void driveMotor(int motion)
{
    switch (motion)
    {
    case DIR_LEFT:
    case DIR_RIGHT:
        SvoMotor.Pause();
        break;
    case DIR_UP:
        SvoMotor.RollUp();
        break;
    case DIR_DOWN:
        SvoMotor.RollDown();
        break;
    default:
        // Do nothing
        break;
    }
}

void task_Movement(void)
{
    int motion = GestSensor.readGesture();
    uint8_t proximity;
    char dir[15];

    // TBD: backlight control based on proximity
    GestSensor.readProximity(proximity);

#ifdef SS_DEBUG
    pc.printf("motion = %d detected !\r\n", motion);
    pc.printf("proximity = %d \r\n", proximity);
#endif

    switch (motion)
    {
    case DIR_LEFT:
        strcpy(dir, "L-Paused");
        break;
    case DIR_RIGHT:
        strcpy(dir, "R-Paused");
        break;
    case DIR_UP:
        strcpy(dir, "Roll Up");
        break;
    case DIR_DOWN:
        strcpy(dir, "Roll Down");
        break;
    case DIR_NEAR:
        strcpy(dir, "Hi, Jeng-Yu!");
        break;
    case DIR_FAR:
        strcpy(dir, "Bye, Jeng-Yu!");
        break;
    default:
        strcpy(dir, "Unknown Dir!");
        break;
    }

    motionToLCD(dir);

    driveMotor(motion);

    helloToLCD();
}

bool prepareGestSensor(void)
{
    char errMsg[16];

    if (GestSensor.init() == false)
    {
        strcpy(errMsg, "E init GS");
        errToLCD(errMsg);
        return (false);
    }

    if (GestSensor.enableGestureSensor(true) == false)
    {
        strcpy(errMsg, "E EN GS");
        errToLCD(errMsg);
        return (false);
    }

#ifdef SS_DEBUG
    pc.printf("gs interrupt enable: %d\r\n", GestSensor.getGestureIntEnable());
    pc.printf("motion = %d\r\n", GestSensor.readGesture());
#endif

    // clear any unintended interrupt
    //GestSensor.readGesture();

    return (true);
}

void checkGestureControl(void)
{
    if (GestSensor.isGestureAvailable())
    {
        task_Movement();
        timerCmd.reset();
        timerCmd.start();
    }
}

void checkBLEControl(void)
{
    if (button_ready)
    {
        if (bnum == '5')
            driveMotor(DIR_UP);
        else if (bnum == '6')
            driveMotor(DIR_DOWN);
        else if ((bnum == '7') || (bnum == '8'))
            driveMotor(DIR_LEFT);
        button_ready = 0;               //reset flag after reading button message
        timerCmd.reset();
        timerCmd.start();
    }
}

void checkTemperature(void)
{
    if (tempF > confTempF)
    {
        if (timerCmd.read() > IDLE_TIME_FOR_TEMP_CONTROL_S)
        {
            if (SvoMotor.GetCurrentState() != down)
            {
                char str[15];
                strcpy(str, "Hot->Roll Down");
                motionToLCD(str);
                driveMotor(DIR_DOWN);
                helloToLCD();
            }
            timerCmd.reset();
        }
    }
    tempToLCD(tempF);
}

int main()
{
    pc.baud(9600);
    pc.printf("\r\n Starting Smart Shade Test...\r\n\r\n");

    // Clear LCD screen
    LCD.cls();
    helloToLCD();

    // Initialize Temperature Sensor
    TempSensor.Initialize();

    // Initialize Gesture Sensor
    prepareGestSensor();

    // Sample temperature every TEMPERATURE_SAMPLING_TIME_S second
    tkTemp.attach(&task_ReadTemperature, TEMPERATURE_SAMPLING_TIME_S);
    timerCmd.start();

    // Set up serial interrupt for Bluefruit
    BLE.attach(&task_ReadBLE, Serial::RxIrq);

    // TBD
    //gsInt.fall(&task_Movement);

    while(1)
    {
        // Check for new gesture
        checkGestureControl();

        // Check for a new button message ready from BLE
        checkBLEControl();

        // Update / handle current temperature
        checkTemperature();

        wait_ms(SAMPLING_TIME_MS);
    }
}
