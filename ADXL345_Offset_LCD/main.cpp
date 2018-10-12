/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"
#include "TextLCD.h"

#define SAMPLING_TIME_MS  10

#define CAL_SAMPLES		  128

#define REG_DATA_FORMAT   0x31
#define REG_POWER_CTL     0x2D

const float AccSensitivity = 0.0039;	// Maximum resolution of 3.9 mg/LSB (per ADXL345 Datasheet)

const int addrADXL345 = (0x53 << 1); 	// Address when grounding ALT ADDRESS pin

TextLCD lcd(p25, p26, p21, p22, p23, p24); // rs, e, d4-d7

I2C sensor(p9,p10);

Serial pc(USBTX,USBRX);

char    buffer[6];
char	config_t[5];

int16_t data[3];

float   accX, accY, accZ;

void printAccToLCD(float x, float y, float z)
{
	lcd.cls();
	lcd.printf("x:%+2.2f y:%+2.2f", x, y);

	lcd.locate(0,1);
	lcd.printf("z:%+2.2f", z);
}

int16_t round(int16_t value, int16_t division)
{
	if (value > 0)
	{
		return ((value + (division / 2)) / division);
	}
	else
	{
		return ((value - (division / 2)) / division);
	}
}

void resetOffsetADXL345(void)
{
    pc.printf("Resetting current offsets... \r\n");

	config_t[0] = 0x1E;									// set pointer register to 'OFSX'
	config_t[1] = 0x00;
	config_t[2] = 0x00;
	config_t[3] = 0x00;

	sensor.write(addrADXL345, config_t, 4);
	pc.printf("\r\n");
}

void calibrateAt1g(int16_t* pX, int16_t* pY, int16_t* pZ)
{
	pc.printf("Now collecting X0g, Y0g, and Z1g\r\n\r\n");
    pc.printf("Place z = 1g orientation, then press 'r' when ready \r\n\r\n");

    while(pc.getc() != 'r') {}
    wait(1);

	for (unsigned short i = 0; i < CAL_SAMPLES; i++)
	{
		config_t[0] = 0x32;								// set pointer register to 'DATAX0'
		sensor.write(addrADXL345, config_t, 1);
		sensor.read(addrADXL345, buffer, 6);			// send pointer to multi-read 'DATAX0'

		*pX += (buffer[1]<<8 | buffer[0]); 			// Combine MSB with LSB
		*pY += (buffer[3]<<8 | buffer[2]);
		*pZ += (buffer[5]<<8 | buffer[4]);
		wait_ms(SAMPLING_TIME_MS);
	}

	*pX = round(*pX, CAL_SAMPLES);
	*pY = round(*pY, CAL_SAMPLES);
	*pZ = round(*pZ, CAL_SAMPLES);

	pc.printf("\r\n");
}

void measureZRange(int16_t* pZ)
{
	for (unsigned short i = 0; i < CAL_SAMPLES; i++)
	{
		config_t[0] = 0x32;								// set pointer register to 'DATAX0'
		sensor.write(addrADXL345, config_t, 1);
		sensor.read(addrADXL345, buffer, 6);			// send pointer to multi-read 'DATAX0'

		*pZ += (buffer[5]<<8 | buffer[4]);

		wait_ms(SAMPLING_TIME_MS);
	}

	*pZ = round(*pZ, CAL_SAMPLES);
}

void calibrateZs(int16_t* pZs)
{
	int16_t Zmeas1g = 0;
	int16_t ZmeasN1g = 0;

	pc.printf("Calibrating sensitivity in z = 1g orientation... \r\n");
    pc.printf("\r\nPlace z = 1g orientation, then press 'r' when ready \r\n");

    while(pc.getc() != 'r') {}
    wait(.1);
    measureZRange(&Zmeas1g);

    pc.printf("\r\nPlace z = -1g orientation, then press 'r' when ready \r\n");

    while(pc.getc() != 'r') {}
    wait(.2);
    measureZRange(&ZmeasN1g);

    *pZs = round(abs(Zmeas1g)+abs(ZmeasN1g), 2);

    pc.printf("\r\nSensitivity Zs = %+4d\r\n", *pZs);
	pc.printf("\r\n");
}

void calibrateADXL345(void)
{
	int16_t	X0g = 0;
	int16_t	Y0g = 0;
	int16_t	Z0g = 0;
	int16_t Z1g = 0;
	int16_t	Xoff = 0;
	int16_t	Yoff = 0;
	int16_t	Zoff = 0;
	int16_t Zs = 0;

	pc.printf("+++ Calibrating ADXL345 Offset +++\r\n\r\n");

    resetOffsetADXL345();

    calibrateZs(&Zs);

    calibrateAt1g(&X0g, &Y0g, &Z1g);

    Z0g = Z1g - Zs;
	Xoff = -round(X0g, 4);									// 3.9 mg/LSB -> 15.6 mg/LSB
	Yoff = -round(Y0g, 4);
	Zoff = -round(Z0g, 4);

	pc.printf("! Calibrated Results (%d samples): X0g = %+d, Y0g = %+d, Z0g = %+d\r\n",
			CAL_SAMPLES, X0g, Y0g, Z0g);

	pc.printf("                                 : Z1g = %+d, Zs = %+d\r\n\r\n", Z1g, Zs);

	// Update the Offset registers
	config_t[0] = 0x1E;									// set pointer register to 'OFSX'
	config_t[1] = Xoff & 0xFF;
	config_t[2] = Yoff & 0xFF;
	config_t[3] = Zoff & 0xFF;

	pc.printf("! Input Offset: OFSX = 0x%02X, OFSY = 0x%02X, OFSZ = 0x%02X\r\n\r\n", config_t[1], config_t[2], config_t[3]);

	sensor.write(addrADXL345, config_t, 4);
}

int main()
{
	int16_t demo = 0;

	pc.baud(9600);
	pc.printf("\r\n\r\n<............\t Starting ADXL345 \t............>\r\n\r\n");

	// Clear LCD screen
	lcd.cls();

	// Initialize ADXL345
	config_t[0] = REG_DATA_FORMAT;						// set pointer register to 'DATA_FORMAT'
	config_t[1] = 0x0B;									// format +/-16g, FULL_RES with 0.0039g/LSB
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_POWER_CTL;						// set pointer register to 'POWER_CTL'
	config_t[1] = 0x08;									// select measure mode
	sensor.write(addrADXL345, config_t, 2);

	calibrateADXL345();

    pc.printf("\r\nCalibration completed. Press 'r' when ready to do some real measurement.\r\n\r\n");

    while(pc.getc() != 'r') {}
    wait(0.1);

	while(demo++ < 8)
	{
		wait_ms(1000);

		//
		// Sample acceleration first
		//
		config_t[0] = 0x32;								// set pointer register to 'DATAX0'
		sensor.write(addrADXL345, config_t, 1);
		sensor.read(addrADXL345, buffer, 6);			// send pointer to multi-read 'DATAX0'

		// Interpret the raw data bytes into meaningful results
		data[0] = buffer[1]<<8 | buffer[0]; 			// Combine MSB with LSB
		data[1] = buffer[3]<<8 | buffer[2];
		data[2] = buffer[5]<<8 | buffer[4];

		accX = data[0]*AccSensitivity;					// x-axis acceleration in G's
		accY = data[1]*AccSensitivity;					// y-axis acceleration in G's
		accZ = data[2]*AccSensitivity;					// z-axis acceleration in G's

		pc.printf("accX:  %+4.2f g (%+4d), accY:  %+4.2f g (%+4d), accZ:  %+4.2f g (%+4d)\r\n",
				accX, data[0], accY, data[1], accZ, data[2]);

        //
		// Display to LCD
		//
		printAccToLCD(accX, accY, accZ);
	}
}
