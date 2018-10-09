/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define SPI_CLOCK_RATE    2000000
#define SAMPLING_TIME_MS  200
#define REG_DATA_FORMAT   0x31
#define REG_POWER_CTL     0x2D

const float AccSensitivity = 0.0039;	// Maximum resolution of 3.9 mg/LSB (per ADXL345 Datasheet)

SPI acc(p11,p12,p13);  // MOSI-SDA, MISO-SDO, SCK-SCL
DigitalOut cs(p14);

Serial pc(USBTX,USBRX);

char    buffer[6];
int16_t data[3];
float   accX, accY, accZ;

int main()
{
	pc.baud(115200);
	pc.printf("Starting ADXL345 Test...\r\n\r\n");

	// Initialize ADXL345
	cs = 1;									// ADXL345 not selected
	acc.format(8,3);						// 8 bit data, SPI Mode 3
	acc.frequency(SPI_CLOCK_RATE);

	// Set resolution in DATA_FORMAT
	cs = 0;									// Select the ADXL345
	acc.write(REG_DATA_FORMAT);
	acc.write(0x0B);						// format +/-16g, FULL_RES with 0.0039g/LSB
	cs = 1;									// End of Transmission

	// Set measurement mode in POWER_CTL
	cs = 0;									// Start a new Transmission
	acc.write(REG_POWER_CTL);
	acc.write(0x08);						// Select measure mode
	cs = 1;									// End of Transmission

	while(1)
	{
		wait_ms(SAMPLING_TIME_MS);

		cs = 0;								// Start a transmission
		acc.write(0x80|0x40|0x32);			// RW bit high (to read), MB bit high (burst), plus address
		for(int i = 0;i <= 5; ++i)
		{
			buffer[i] = acc.write(0x00);	// Read back 6 data bytes (DATAX0..DATAZ1)
		}
		cs = 1; 							// End of Transmission

		// Interpret the raw data bytes into meaningful results
		data[0] = buffer[1]<<8 | buffer[0]; 	// Combine MSB with LSB
		data[1] = buffer[3]<<8 | buffer[2];
		data[2] = buffer[5]<<8 | buffer[4];

		accX = data[0]*AccSensitivity;		// x-axis acceleration in G's
		accY = data[1]*AccSensitivity;		// y-axis acceleration in G's
		accZ = data[2]*AccSensitivity;		// z-axis acceleration in G's
		pc.printf("accX:  %+4.2f g (%+4d), accY:  %+4.2f g (%+4d), accZ:  %+4.2f g (%+4d)\r\n",
				accX, data[0], accY, data[1], accZ, data[2]);
	}
}
