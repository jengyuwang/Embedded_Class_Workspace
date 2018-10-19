
/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"
#include "TextLCD.h"
#include "TMP102.h"
#include "APDS9960_I2C.h"
#include "ServoMotor.h"
#include "Bluefruit.h"

#define TEMPERATURE_SAMPLING_TIME_S		5.
#define SAMPLING_TIME_MS  				100

#define SS_DEBUG


//state used to remember previous characters read in a button message from BLE
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};
statetype state = start;

//global variables for main and interrupt routine
volatile bool button_ready = 0;
volatile int  bnum = 0;
volatile int  bhit;


// Object Declaration
TextLCD LCD(p12, p20, p21, p22, p23, p24); // rs, e, d4-d7

TMP102 TempSensor(p9, p10);
APDS9960_I2C GestSensor(p9, p10);
ServoMotor SvoMotor(p26);
//Bluefruit BLE(p27, p28);
Serial BLE(p28, p27);

Serial pc(USBTX,USBRX);
Ticker tkTemp;

InterruptIn gsInt(p5);

volatile float	tempF = 0.;
volatile bool	bleDataAvailable = false;

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
	/*
	bleDataAvailable = BLE.IsDataAvailable();
#ifdef SS_DEBUG
	pc.printf("Serial interrupt on BLE. bleDataAvaialble = %d \r\n", bleDataAvailable);
#endif

	bleDataAvailable = BLE.ParseData();
*/
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

#ifdef SS_DEBUG
	pc.printf("Serial interrupt on BLE. state = %d, button_ready = %d \r\n", state, button_ready);
#endif

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

#ifdef SS_DEBUG
		pc.printf("APDS9960 cannot be initialized !\r\n");
#endif
		strcpy(errMsg, "E init GS");
		errToLCD(errMsg);
		return (false);
	}

	if (GestSensor.enableGestureSensor(true) == false)
	{
#ifdef SS_DEBUG
		pc.printf("Gesture sensor cannot be enabled !\r\n");
#endif
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

int main()
{
	pc.baud(9600);
	pc.printf("\r\n Starting Smart Shade Test...\r\n\r\n");

	// Clear LCD screen
	LCD.cls();
    helloToLCD();

	// Initialize temperature sensor
	TempSensor.Initialize();

	// Sample temperature every TEMPERATURE_SAMPLING_TIME_S second
	tkTemp.attach(&task_ReadTemperature, TEMPERATURE_SAMPLING_TIME_S);

	// Set up serial interrupt for Bluefruit
	BLE.attach(&task_ReadBLE, Serial::RxIrq);

	// Initialize gesture sensor
    if (prepareGestSensor() == true)
    {
    	// TBD
    	//gsInt.fall(&task_Movement);

    	while(1)
    	{
    		if (GestSensor.isGestureAvailable())
    		{
#ifdef SS_DEBUG
    			pc.printf("Gest data is available. \r\n");
#endif
    			task_Movement();
    		}

            //check for a new button message ready
            if (button_ready)
            {
            	if (bnum == '5')
            		driveMotor(DIR_UP);
            	else if (bnum == '6')
            		driveMotor(DIR_DOWN);
            	else if ((bnum == '7') || (bnum == '8'))
            		driveMotor(DIR_LEFT);
                button_ready = 0; //reset flag after reading button message
            }
/*

    		if (bleDataAvailable == true)
    		{
    			driveMotor(BLE.ReadInput());
    			bleDataAvailable = false;
    		}
*/
/*
    		if (BLE.IsDataAvailable())
    		{
#ifdef SS_DEBUG
    			pc.printf("BLE data is available. \r\n");
#endif
    			driveMotor(BLE.ReadInput());
    		}
*/

    		tempToLCD(tempF);

    		wait_ms(SAMPLING_TIME_MS);
    	}
    }
    else
    {
#ifdef SS_DEBUG
    	pc.printf("Gest sensor faulty ! \r\n");
#endif
    }

}
