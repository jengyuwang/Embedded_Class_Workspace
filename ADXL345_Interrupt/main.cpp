/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define USE_TAP
#define USE_INT_PIN

#define SAMPLING_TIME_MS  200

#define REG_DATA_FORMAT   0x31
#define REG_POWER_CTL     0x2D
#define REG_INT_MAP 	  0x2F
#define REG_INT_ENABLE	  0x2E
#define REG_THRESH_ACT	  0x24
#define REG_ACT_INACT_CTL 0x27
#define REG_FIFO_CTL      0x38
#define REG_INT_SOURCE    0x30
#define REG_THRESH_TAP    0x1D
#define REG_DUR           0x21
#define REG_TAP_AXES      0x2A
#define REG_ACT_TAP_STATUS 0x2B
#define REG_Latent        0x22
#define REG_Window        0x23


#define ACT_THRESHOLD     20						// 5g with 62.5 mg/LSB scale factor

const float AccSensitivity = 0.0039;	   			// Maximum resolution of 3.9 mg/LSB (per ADXL345 Datasheet)

const int addrADXL345 = (0x53 << 1); 	   			// Address when grounding ALT ADDRESS pin

I2C sensor(p9,p10);
DigitalOut redLed(p20);
Serial pc(USBTX,USBRX);

#ifdef USE_INT_PIN
InterruptIn tap(p15);
#endif

char    buffer[6];
char	config_t[5];

int16_t data[3];

float   accX, accY, accZ;

void initialize(void)
{
	// Initialize ADXL345
	config_t[0] = REG_DATA_FORMAT;						// set pointer register to 'DATA_FORMAT'
	config_t[1] = 0x0B;									// format +/-16g, FULL_RES with 0.0039g/LSB
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_POWER_CTL;						// set pointer register to 'POWER_CTL'
	config_t[1] = 0x08;									// select measure mode
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_INT_MAP;							// set point register to 'INT_MAP'
#ifdef USE_TAP
	config_t[1] = 0x9F;									// send SINGLE_TAP and DOUBLE_TAP to INT1 pin
#else
	config_t[1] = 0xEF;								    // send Activity to INT1 pin
#endif
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_ACT_INACT_CTL;					// set point register to 'ACT_INACT_CTL'
	config_t[1] = 0x70;									// select dc-coupled operation on all axes
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_THRESH_ACT;						// set point register to 'THRESH_ACT'
	config_t[1] = ACT_THRESHOLD;						// set threshold of 5 g
	sensor.write(addrADXL345, config_t, 2);

#ifdef USE_TAP
	config_t[0] = REG_THRESH_TAP;						// set point register to 'THRESH_TAP'
	config_t[1] = ACT_THRESHOLD;					    // set threshold of 5 g
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_DUR;		         				// set point register to 'DUR'
	config_t[1] = 13;									// set 8ms duration (625 us/LSB)
	config_t[2] = 48;									// set 60ms latent (1.25 ms/LSB)
	config_t[2] = 240;									// set 300ms window (1.25 ms/LSB)
	sensor.write(addrADXL345, config_t, 4);

	config_t[0] = REG_TAP_AXES;		         			// set point register to 'TAP_AXES'
	config_t[1] = 0x07;									// select all axes for tap detection
	sensor.write(addrADXL345, config_t, 2);
#endif
/*
	config_t[0] = REG_FIFO_CTL;						    // set point register to 'FIFO_CTL'
	config_t[1] = 0x62;									// set trigger mode on INT1 with 1 sample before trigger
	sensor.write(addrADXL345, config_t, 2);
*/
	config_t[0] = REG_FIFO_CTL;						    // set point register to 'FIFO_CTL'
	config_t[1] = 0x00;									// bypass FIFO mode
	sensor.write(addrADXL345, config_t, 2);


	config_t[0] = REG_INT_ENABLE;						// set point register to 'INT_ENABLE'
#ifdef USE_TAP
	config_t[1] = 0x60;									// enable SINGLE_TAP and DOUBLE_TAP
#else
	config_t[1] = 0x10;									// enable Activity
#endif
	sensor.write(addrADXL345, config_t, 2);
}

void readAccToTerminal(void)
{
	config_t[0] = 0x32;								// set pointer register to 'DATAX0'
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 12);			// send pointer to multi-read 'DATAX0'

	// Interpret the raw data bytes into meaningful results
	data[0] = buffer[1]<<8 | buffer[0]; 			// Combine MSB with LSB
	data[1] = buffer[3]<<8 | buffer[2];
	data[2] = buffer[5]<<8 | buffer[4];

	accX = data[0]*AccSensitivity;					// x-axis acceleration in G's
	accY = data[1]*AccSensitivity;					// y-axis acceleration in G's
	accZ = data[2]*AccSensitivity;					// z-axis acceleration in G's

	pc.printf("accX:  %+4.2f g (%+4d), accY:  %+4.2f g (%+4d), accZ:  %+4.2f g (%+4d)\r\n",
			accX, data[0], accY, data[1], accZ, data[2]);
}

#ifdef USE_INT_PIN
void ReadTap(void)
{
	config_t[0] = REG_INT_SOURCE;					// set pointer register to 'INT_SOURCE'
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 1);			// send pointer to read 'INT_SOURCE'

	pc.printf("INT_SOURCE: 0x%02X\r\n", buffer[0]);

#ifdef USE_TAP
	if (buffer[0] & 0x40)
#else
	if (buffer[0] & 0x10)
#endif
	{
		// Activity triggered
		// Read the value in the terminal
		readAccToTerminal();

		// Light up LED
		redLed = 1;

		config_t[0] = REG_ACT_TAP_STATUS;
		sensor.write(addrADXL345, config_t, 1);
		sensor.read(addrADXL345, buffer, 1);
		pc.printf("ACT_TAP_STATUS: 0x%02X\r\n", buffer[0]);
	}
}
#endif

int main()
{
	pc.baud(115200);
	pc.printf("Starting ADXL345 Test...\r\n\r\n");

    initialize();

#ifdef USE_INT_PIN
	tap.rise(&ReadTap);
#endif

	while(1)
	{

#ifdef USE_INT_PIN

		wait_ms(SAMPLING_TIME_MS);
		redLed = 0;

#else
		config_t[0] = REG_INT_SOURCE;					// set pointer register to 'INT_SOURCE'
		sensor.write(addrADXL345, config_t, 1);
		sensor.read(addrADXL345, buffer, 1);			// send pointer to read 'INT_SOURCE'

		//pc.printf("INT_SOURCE: 0x%02X\r\n", buffer[0]);

#ifdef USE_TAP
		if (buffer[0] & 0x40)
#else
		if (buffer[0] & 0x10)
#endif
		{
			// Activity triggered
			// Read the value in the terminal
			readAccToTerminal();

			// Light up LED
			redLed = 1;

			config_t[0] = REG_ACT_TAP_STATUS;
			sensor.write(addrADXL345, config_t, 1);
			sensor.read(addrADXL345, buffer, 1);
			pc.printf("ACT_TAP_STATUS: 0x%02X\r\n", buffer[0]);

		}
		wait_ms(SAMPLING_TIME_MS);
		redLed = 0;
#endif
	}
}
