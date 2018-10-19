/*
 * TMP102.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: jengyu
 */

#include "TMP102.h"

TMP102::TMP102(PinName sda, PinName scl) : tempSensor(sda, scl)
{

}

TMP102::~TMP102(void)
{

}

void TMP102::Initialize(void)
{
	// Initialize TMP102
	config_t[0] = 0x01;									// set pointer register to 'config register' (Table 7 data sheet)
	config_t[1] = 0x60;									// config temperature measurements to 12-bit resolution (Table 10 data sheet)
	config_t[2] = 0xA0;									// configure temperature conversion rate to 4 Hz, AL to normal (Table 11 data sheet)
	tempSensor.write(addrTMP102, config_t, 3);			// write 3 bytes to device at address addrTMP102
}

float TMP102::GetTemperatureInC(void)
{
	measure();
	return (TempSensitivity * 							// convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
			(((temp_read[0] << 8) + temp_read[1]) >> 4));
}

float TMP102::GetTemperatureInF(void)
{
	measure();
	return (TempSensitivity * 							// convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
			(((temp_read[0] << 8) + temp_read[1]) >> 4) * 9. / 5. + 32.);
}

void TMP102::measure(void)
{
	config_t[0] = 0x00;									// set pointer register to 'data register' (Table 7 datasheet)
	tempSensor.write(addrTMP102, config_t, 1);			// send to pointer 'read temp'
	tempSensor.read(addrTMP102, temp_read, 2);			// read the 2-byte temperature data
}
