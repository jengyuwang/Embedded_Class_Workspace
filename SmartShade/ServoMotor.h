/*
 * ServoMotor.h
 *
 *  Created on: Oct 17, 2018
 *      Author: jengyu
 */

#ifndef SERVOMOTOR_H_
#define SERVOMOTOR_H_

#include "mbed.h"

enum stateShade
{
    up = 0,
    middle,
    down
};

class ServoMotor
{
public:

    ServoMotor(PinName pwmOut);
     ~ServoMotor();

    bool    RollUp(void);
    bool    RollDown(void);
    void    Pause(void);
    int     GetCurrentState(void);

private:
    const float delay_sec = 1.;
    const float step_degree = 20.;
    const float min_pulse_width = 0.0004;
    const float max_pulse_width = 0.0021;
    const float delta_width = (max_pulse_width - min_pulse_width) / (180./step_degree);

    const unsigned int pwm_period_ms = 20;

    PwmOut          svoMotor;
    volatile float  currentDuty;
    volatile bool   stop;
};


#endif /* SERVOMOTOR_H_ */
