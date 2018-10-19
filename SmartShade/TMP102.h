/*
 * TMP102.h
 *
 *  Created on: Oct 16, 2018
 *      Author: jengyu
 */

#ifndef TMP102_H_
#define TMP102_H_

#include "mbed.h"

class TMP102
{
public:

	TMP102(PinName sda, PinName scl);
	 ~TMP102();

	void Initialize(void);
	float GetTemperatureInF(void);
	float GetTemperatureInC(void);

protected:

	void measure(void);

private:

	const float	TempSensitivity = 0.0625;	// degrees C/LSB (per TMP102 Data sheet)
	const int	addrTMP102 = 0x90;			// Default I2C address of TMP102

	char		config_t[3];
	char		temp_read[2];
	I2C			tempSensor;
};

#endif /* TMP102_H_ */
