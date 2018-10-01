/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define DEBOUNCE_TIME_CLOSE_US    570
#define DEBOUNCE_TIME_OPEN_US     210

InterruptIn SPDT(p15);
DigitalOut GreenLed(p20);
Timer debounce;

Serial pc(USBTX, USBRX);

volatile unsigned int toggleCount = 0;

void ToggleOn(void)
{
	if (debounce.read_us() > DEBOUNCE_TIME_CLOSE_US)
		toggleCount++;

	debounce.reset();
}

void ToggleOff(void)
{
	if (debounce.read_us() > DEBOUNCE_TIME_OPEN_US)
		toggleCount++;

	debounce.reset();
}

int main() 
{
	debounce.start();

	SPDT.rise(&ToggleOn);
	SPDT.fall(&ToggleOff);

    while(1)
    {
    	if (toggleCount == 10)
    	{
    		GreenLed = 1;
    		wait(0.2);
    		GreenLed = 0;
    	}
    }
}
