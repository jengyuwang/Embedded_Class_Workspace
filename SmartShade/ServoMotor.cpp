/*
 * ServoMotor.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: jengyu
 */

#include "ServoMotor.h"

#ifdef SS_DEBUG
Serial pc(USBTX, USBRX);
#endif

ServoMotor::ServoMotor(PinName pwmOut) : svoMotor(pwmOut)
{
	svoMotor.period_ms(pwm_period_ms);
	currentDuty = max_pulse_width;			// default shade up
	svoMotor.pulsewidth(currentDuty);
	stop = false;
}

ServoMotor::~ServoMotor(void)
{

}

bool ServoMotor::RollUp(void)
{
	if (currentDuty >= max_pulse_width)
	{
#ifdef SS_DEBUG
		pc.printf("Already rolled up!\r\n");
#endif
		// already up
		return false;
	}
	else
	{
		for (float duty = currentDuty; duty <= max_pulse_width; duty += delta_width)
		{
			svoMotor.pulsewidth(duty);
			currentDuty = duty;
			if (stop)
			{
				stop = false;
				break;
			}
			else
			{
				wait(delay_sec);
			}
		}

#ifdef SS_DEBUG
		pc.printf("Rolling up with currentDuty = %2.4f\r\n", currentDuty);
#endif
		return true;
	}
}

bool ServoMotor::RollDown(void)
{
	if (currentDuty <= min_pulse_width)
	{
#ifdef SS_DEBUG
		pc.printf("Already rolled down!\r\n");
#endif
		// already down
		return false;
	}
	else
	{
		for (float duty = currentDuty; duty >= min_pulse_width; duty -= delta_width)
		{
			svoMotor.pulsewidth(duty);
			currentDuty = duty;
			if (stop)
			{
				stop = false;
				break;
			}
			else
			{
				wait(delay_sec);
			}
		}

#ifdef SS_DEBUG
		pc.printf("Rolling down with currentDuty = %2.4f\r\n", currentDuty);
#endif

		return true;
	}
}

void ServoMotor::Pause(void)
{
	stop = true;

#ifdef SS_DEBUG
	pc.printf("Pausing Motor !\r\n");
#endif
}




