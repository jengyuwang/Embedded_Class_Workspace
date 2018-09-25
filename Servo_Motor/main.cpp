/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

float delay_sec = 3.;
float step_degree = 20.;
float min_pulse_width = 0.0004;
float max_pulse_width = 0.0021;
float delta_width = (max_pulse_width - min_pulse_width) / (180./step_degree);

unsigned int pwm_period_ms = 20;

PwmOut ServoMotor(p26);

int main() 
{
	ServoMotor.period_ms(pwm_period_ms);

    while(1) 
    {
    	for (float duty = min_pulse_width; duty <= max_pulse_width; duty += delta_width)
    	{
    		ServoMotor.pulsewidth(duty);
    		wait(delay_sec);
    	}
    }
}
