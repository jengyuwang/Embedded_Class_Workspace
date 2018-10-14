/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define REG_DATA_FORMAT   0x31
#define REG_BW_RATE		  0x2C
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

#define ACT_THRESHOLD     128						// 8g with 62.5 mg/LSB scale factor

const float AccSensitivity = 0.0039;	   			// Maximum resolution of 3.9 mg/LSB (per ADXL345 Datasheet)

const int addrADXL345 = (0x53 << 1); 	   			// Address when grounding ALT ADDRESS pin

I2C sensor(p9,p10);
DigitalOut redLed(p20);
Serial pc(USBTX,USBRX);
InterruptIn tap(p15);

char    buffer[6];
char	config_t[5];

int16_t data[3];

float   accX, accY, accZ;

void initialize(void)
{
	// Initialize ADXL345
	config_t[0] = REG_POWER_CTL;						// set pointer register to 'POWER_CTL'
	config_t[1] = 0x00;									// select measure mode
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_DATA_FORMAT;						// set pointer register to 'DATA_FORMAT'
	config_t[1] = 0x0B;									// format +/-16g, FULL_RES with 0.0039g/LSB
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_BW_RATE;							// set pointer register to 'BW_RATE'
	config_t[1] = 0x0B;									// set 200 Hz
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_THRESH_ACT;						// set point register to 'THRESH_ACT'
	config_t[1] = ACT_THRESHOLD;						// set threshold of 5 g
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_ACT_INACT_CTL;					// set point register to 'ACT_INACT_CTL'
	config_t[1] = 0x70;									// select dc-coupled operation on all axes
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_FIFO_CTL;						    // set point register to 'FIFO_CTL'
	config_t[1] = 0xC1;									// set trigger mode on INT1 with 1 sample before trigger
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_INT_ENABLE;						// set point register to 'INT_ENABLE'
	config_t[1] = 0x10;     							// enable Activity
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_INT_MAP;							// set point register to 'INT_MAP'
	config_t[1] = 0xEF;								    // send Activity to INT1 pin
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_POWER_CTL;						// set pointer register to 'POWER_CTL'
	config_t[1] = 0x08;									// select measure mode
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_INT_SOURCE;					    // set pointer register to 'INT_SOURCE'
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 1);			    // read 'INT_SOURCE' to clear any unintended interrupt
}

void readShockToTerminal()
{
	config_t[0] = REG_ACT_TAP_STATUS;
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 1);

	pc.printf("\r\n!!! Shock Detected !!! On ");

	if (buffer[0] & 0x40)
		pc.printf("X ");
	if (buffer[0] & 0x20)
		pc.printf("Y ");
	if (buffer[0] & 0x10)
		pc.printf("Z ");

	pc.printf("axes\r\n");

	config_t[0] = 0x32;								// set pointer register to 'DATAX0'
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 6);			// send pointer to multi-read 'DATAX0'

	data[0] = buffer[1]<<8 | buffer[0]; 			// Combine MSB with LSB
	data[1] = buffer[3]<<8 | buffer[2];
	data[2] = buffer[5]<<8 | buffer[4];

	accX = data[0]*AccSensitivity;					// x-axis acceleration in G's
	accY = data[1]*AccSensitivity;					// y-axis acceleration in G's
	accZ = data[2]*AccSensitivity;					// z-axis acceleration in G's

	pc.printf("accX:  %+4.2f g (%+4d), accY:  %+4.2f g (%+4d), accZ:  %+4.2f g (%+4d)\r\n",
			accX, data[0], accY, data[1], accZ, data[2]);

	config_t[0] = REG_INT_SOURCE;					// set pointer register to 'INT_SOURCE'
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 1);			// clear the interrupt by reading 'INT_SOURCE'
}

void readTap(void)
{
	redLed = 1;
}

int main()
{
	pc.baud(9600);
	pc.printf("Starting ADXL345 Tap Test...\r\n\r\n");

	redLed = 0;

    initialize();

    tap.mode(PullDown);

    tap.rise(&readTap);

	while(1)
	{
		if (redLed == 1)
		{
			readShockToTerminal();
			redLed = 0;
		}
	}
}
