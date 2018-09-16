/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

DigitalOut greenLed(p8);

#define SQUARE_SIGNAL_FREQUENCY_HZ		5

int main() 
{
    while(1) 
    {
   		greenLed = 0;
   		wait(0.5/SQUARE_SIGNAL_FREQUENCY_HZ);
   		greenLed = 1;
   		wait(0.5/SQUARE_SIGNAL_FREQUENCY_HZ);
    }
}
