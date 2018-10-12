
/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"
#include "TextLCD.h"

#define SPI_CLOCK_RATE    2000000
#define SAMPLING_TIME_MS  500

#define REG_DATA_FORMAT   0x31
#define REG_POWER_CTL     0x2D

const float AccSensitivity = 0.0039;	// Maximum resolution of 3.9 mg/LSB (per ADXL345 Datasheet)
const float TempSensitivity = 0.0625;	// degrees C/LSB (per TMP102 Data sheet)

const int addrTMP102 = 0x90;			// Default I2C address of TMP102
const int addrADXL345 = (0x53 << 1); 	// Address when grounding ALT ADDRESS pin

TextLCD lcd(p25, p26, p21, p22, p23, p24); // rs, e, d4-d7

I2C sensor(p9,p10);

Serial pc(USBTX,USBRX);

char    buffer[6];
char	config_t[3];
char	temp_read[2];

int16_t data[3];

float   accX, accY, accZ;
float	temp;

void printAccAndTempToLCD(float x, float y, float z, float temp)
{
	lcd.cls();
	lcd.printf("x:%+2.2f y:%+2.2f", x, y);

	lcd.locate(0,1);
	lcd.printf("z:%+2.2f", z);

	lcd.locate(10,1);
	lcd.printf("%.2fF",temp);
}

int main()
{
	pc.baud(9600);
	pc.printf("Starting ADXL345/TMP102 Test...\r\n\r\n");

	// Clear LCD screen
	lcd.cls();

	// Initialize ADXL345
	config_t[0] = REG_DATA_FORMAT;						// set pointer register to 'DATA_FORMAT'
	config_t[1] = 0x0B;									// format +/-16g, FULL_RES with 0.0039g/LSB
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_POWER_CTL;						// set pointer register to 'POWER_CTL'
	config_t[1] = 0x08;									// select measure mode
	sensor.write(addrADXL345, config_t, 2);

	// Initialize TMP102
	config_t[0] = 0x01;									// set pointer register to 'config register' (Table 7 data sheet)
	config_t[1] = 0x60;									// config temperature measurements to 12-bit resolution (Table 10 data sheet)
	config_t[2] = 0xA0;									// configure temperature conversion rate to 4 Hz, AL to normal (Table 11 data sheet)
	sensor.write(addrTMP102, config_t, 3);				// write 3 bytes to device at address addrTMP102

	while(1)
	{
		wait_ms(SAMPLING_TIME_MS);

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
		// Sample temperature now
		//
		config_t[0] = 0x00;								// set pointer register to 'data register' (Table 7 datasheet)
		sensor.write(addrTMP102, config_t, 1);			// send to pointer 'read temp'
		sensor.read(addrTMP102, temp_read, 2);			// read the 2-byte temperature data

		temp = TempSensitivity * 						// convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
				(((temp_read[0] << 8) + temp_read[1]) >> 4);
        temp = temp * 9. / 5. + 32.;

        //
		// Display to LCD
		//
		printAccAndTempToLCD(accX, accY, accZ, temp);
	}
}
