/*
 * myADXL345.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jengyu
 */
#include "myADXL345.h"
#include "IAP.h"

I2C 	sensor(p9,p10);
IAP     iap;

char    buffer[6];
char	config_t[5];

volatile int16_t data[3];

struct CAL_DATA calData;

volatile bool fKeyInput = false;

void Rx_interrupt(void)
{
	char c = pc.getc();

	if(!fKeyInput && (c == 'r' || c == 'R'))
	{
		fKeyInput = true;
	}
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

void read1gData(volatile int16_t *pData, int dir)
{
	wait(WAIT_TIME_FOR_CAL_S);
	for (unsigned short i = 0; i < CAL_SAMPLES; i++)
	{
		config_t[0] = 0x32;								// set pointer register to 'DATAX0'
		sensor.write(addrADXL345, config_t, 1);
		sensor.read(addrADXL345, buffer, 6);			// send pointer to multi-read 'DATAX0'

		*pData += (buffer[dir+1]<<8 | buffer[dir]); 	// Combine MSB with LSB
		wait_ms(SAMPLING_TIME_MS);
	}

	*pData = round(*pData, CAL_SAMPLES);

	//pc.printf("\r\nDEBUG %d orientation. pData = %+d\r\n", dir, *pData);
}

void calibrateAt1g(float *pGain, float *pOffset, int dir)
{
	volatile int16_t p1g = 0;
	volatile int16_t n1g = 0;

	char sDir;

	switch(dir)
	{
	case X_DIR:
		sDir = 'x';
		break;
	case Y_DIR:
		sDir = 'y';
		break;
	case Z_DIR:
	default:
		sDir = 'z';
		break;
	}

	pc.printf("Calibrating sensitivity in %c = 1g orientation... \r\n", sDir);
    pc.printf("\r\nPlace %c = 1g orientation, then press 'r' or 'R' when ready \r\n", sDir);

    while(!fKeyInput) {}
    fKeyInput = false;

    read1gData(&p1g, dir);

    pc.printf("\r\nPlace %c = -1g orientation, then press 'r' or 'R' when ready \r\n", sDir);

    while(!fKeyInput) {}
    fKeyInput = false;

    read1gData(&n1g, dir);

	*pGain = (float)(p1g - n1g) * AccSensitivity * 0.5;
	*pOffset = (float)(p1g + n1g) * AccSensitivity * 0.5;

    //pc.printf("\r\nDEBUG %c/%d orientation. p1g = %+d, n1g = %+d, gain=%2.2f, offset=%2.2f\r\n", sDir, dir, p1g, n1g, *pGain, *pOffset);

	pc.printf("\r\n");
}

void updateCalDataToFlash(void)
{
    static char	mem[MEM_SIZE];    						//  RAM memory buffer to use when copying data to flash
    int			rc;
    char*		p;

	memset(&mem[0], 0, sizeof(mem));					// Set all elements of mem array to 0

	// Copy data struct into mem array
	p = (char *)&calData;
	for(uint i = 0; i< sizeof(CAL_DATA); ++i)
		mem[i] = *(p + i);

	// copy RAM to Flash
	iap.prepare(TARGET_SECTOR, TARGET_SECTOR);			// Always must prepare sector before erasing or writing
	rc = iap.write(mem, sector_start_adress[TARGET_SECTOR], MEM_SIZE);
	pc.printf("\r\nCopied: SRAM(0x%08X)->Flash(0x%08X) for %d bytes. (result=0x%08X)\r\n", mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE, rc);

	// compare
	rc = iap.compare(mem, sector_start_adress[TARGET_SECTOR], MEM_SIZE);
	pc.printf("\r\nCompare result     = \"%s\"\r\n", rc ? "FAILED - Sector was probably not Blank before writing" : "OK");
}

void calibrateADXL345(void)
{
	lcd.cls();
	lcd.printf("! Cal Mode !");

	lcd.locate(0,1);
	lcd.printf("See Minicom->");

	memset(&calData, 0, sizeof(CAL_DATA));

	pc.printf("Not Calibrated. Entering Calibrating Mode...\r\n\r\n");

	calibrateAt1g(&(calData.zGain), &(calData.zOffset), Z_DIR);
	calibrateAt1g(&(calData.xGain), &(calData.xOffset), X_DIR);
	calibrateAt1g(&(calData.yGain), &(calData.yOffset), Y_DIR);

	pc.printf("! Calibrated Results (%d samples): xGain = %+2.2f, xOffset = %+2.2f\r\n", CAL_SAMPLES, calData.xGain, calData.xOffset);
	pc.printf("                                 : yGain = %+2.2f, yOffset = %+2.2f\r\n", calData.yGain, calData.yOffset);
	pc.printf("                                 : zGain = %+2.2f, zOffset = %+2.2f\r\n", calData.zGain, calData.zOffset);

	updateCalDataToFlash();
}

void checkCalResult(void)
{
	int rc;

	// Check if calibration results are in the flash
    if ((rc = iap.blank_check(TARGET_SECTOR, TARGET_SECTOR)) == SECTOR_NOT_BLANK)
    {
    	// Calibration result is available. Read from the flash
		calData = *(struct CAL_DATA *) FLASH_SECTOR_29;

		pc.printf("! Saved Calibration Results: xGain = %+2.2f, xOffset = %+2.2f\r\n", calData.xGain, calData.xOffset);
		pc.printf("                           : yGain = %+2.2f, yOffset = %+2.2f\r\n", calData.yGain, calData.yOffset);
		pc.printf("                           : zGain = %+2.2f, zOffset = %+2.2f\r\n", calData.zGain, calData.zOffset);

		pc.printf("\r\nDevice Calibrated. Entering Measuring Mode...\r\n\r\n");
    }
    else
    {
    	// No data -> need to calibrate
    	calibrateADXL345();
    }
}

void ADXL345_init(void)
{
	// Initialize ADXL345
	config_t[0] = REG_DATA_FORMAT;						// set pointer register to 'DATA_FORMAT'
	config_t[1] = 0x0B;									// format +/-16g, FULL_RES with 0.0039g/LSB
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = REG_BW_RATE;							// set pointer register to 'BW_RATE'
	config_t[1] = 0x0E;									// set 1600 Hz
	sensor.write(addrADXL345, config_t, 2);

	config_t[0] = 0x1E;									// set pointer register to 'OFSX'
	config_t[1] = 0x00;									// clear offset register values
	config_t[2] = 0x00;
	config_t[3] = 0x00;
	sensor.write(addrADXL345, config_t, 4);

	config_t[0] = REG_POWER_CTL;						// set pointer register to 'POWER_CTL'
	config_t[1] = 0x08;									// select measure mode
	sensor.write(addrADXL345, config_t, 2);

	checkCalResult();

	lcd.cls();
	lcd.printf("* Meas Mode *");
}

void ADXL345_meas()
{
	config_t[0] = 0x32;								// set pointer register to 'DATAX0'
	sensor.write(addrADXL345, config_t, 1);
	sensor.read(addrADXL345, buffer, 6);			// send pointer to multi-read 'DATAX0'

	// Interpret the raw data bytes into meaningful results
	data[0] = buffer[1]<<8 | buffer[0]; 			// Combine MSB with LSB
	data[1] = buffer[3]<<8 | buffer[2];
	data[2] = buffer[5]<<8 | buffer[4];
}

float GetAccX(void)
{
	float actual = data[0]*AccSensitivity;
	actual = (actual - calData.xOffset) / calData.xGain;
	return actual;	// x-axis acceleration in G's
}

float GetAccY(void)
{
	float actual = data[1]*AccSensitivity;
	actual = (actual - calData.yOffset) / calData.yGain;
	return actual;	// y-axis acceleration in G's
}

float GetAccZ(void)
{
	float actual = data[2]*AccSensitivity;
	actual = (actual - calData.zOffset) / calData.zGain;
	return actual;	// y-axis acceleration in G's
}
