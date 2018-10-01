/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define TOGGLE_GL1_MS	500
#define TOGGLE_RL1_MS	1000
#define TOGGLE_RL2_MS	4.0

DigitalOut GreenLed1(p27);
DigitalOut RedLed1(p15);
DigitalOut RedLed2(p21);

Timer TimerForGL1;
Timer TimerForRL1;
Ticker TickerForRL2;

void ToggleGreenLed1(void)
{
	GreenLed1 = !GreenLed1;
}

void ToggleRedLed1(void)
{
	RedLed1 = !RedLed1;
}

void ToggleRedLed2(void)
{
	RedLed2 = !RedLed2;
}

int main() 
{
	TimerForGL1.start();
	TimerForRL1.start();
	TickerForRL2.attach(&ToggleRedLed2, TOGGLE_RL2_MS);

    while(1) 
    {
    	if (TimerForGL1.read_ms() >= TOGGLE_GL1_MS)
    	{
    		ToggleGreenLed1();
    		TimerForGL1.reset();
    	}

    	if (TimerForRL1.read_ms() >= TOGGLE_RL1_MS)
    	{
    		ToggleRedLed1();
    		TimerForRL1.reset();
    	}
    }
}
