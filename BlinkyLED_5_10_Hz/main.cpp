/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

DigitalOut greenLed(p8);
DigitalIn  switchInput(p21, PullUp);

#define SQUARE_SIGNAL_FREQUENCY_HZ			5
#define SQUARE_SIGNAL_FREQUENCY_HZ_CNTL		10

int main() 
{
	int frequency = SQUARE_SIGNAL_FREQUENCY_HZ;
    while(1) 
    {
    	frequency = (switchInput == 0) ?
    			SQUARE_SIGNAL_FREQUENCY_HZ_CNTL : SQUARE_SIGNAL_FREQUENCY_HZ;
   		greenLed = 0;
   		wait(0.5/frequency);
   		greenLed = 1;
   		wait(0.5/frequency);
    }
}
