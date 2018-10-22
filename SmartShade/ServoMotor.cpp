/*
 * ServoMotor.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: jengyu
 */

#include "ServoMotor.h"

/**
 * ServoMotor constructor
 *
 * @param pwmout       PinName pwm output
 */
ServoMotor::ServoMotor(PinName pwmOut) : svoMotor(pwmOut)
{
    svoMotor.period_ms(pwm_period_ms);
    currentDuty = max_pulse_width;            // default shade up
    svoMotor.pulsewidth(currentDuty);
    stop = false;
}

/**
 * ServoMotor destructor
 */
ServoMotor::~ServoMotor(void)
{

}

/**
 * Roll up the shade
 *
 * @return bool    action taken or not
 */
bool ServoMotor::RollUp(void)
{
    if (currentDuty >= max_pulse_width)
    {
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

        return true;
    }
}

/**
 * Roll down the shade
 *
 * @return bool    action taken or not
 */
bool ServoMotor::RollDown(void)
{
    if (currentDuty <= min_pulse_width)
    {
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

        return true;
    }
}

/**
 * Pause the motor to keep the shade at the place
 *
 */
void ServoMotor::Pause(void)
{
    stop = true;

#ifdef SS_DEBUG
    pc.printf("Pausing Motor !\r\n");
#endif
}

/**
 * Get current shade location
 *
 * @return int    down/up/middle state
 */
int ServoMotor::GetCurrentState(void)
{
    int state;

    if (currentDuty <= (min_pulse_width + delta_width))
    {
        state = down;
    }
    else if (currentDuty >= (max_pulse_width - delta_width))
    {
        state = up;
    }
    else
    {
        state = middle;
    }

    return state;
}
