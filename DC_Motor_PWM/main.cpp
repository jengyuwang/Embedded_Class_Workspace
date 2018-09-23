/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

DigitalIn  switchInput(p17, PullNone);
PwmOut MOSFET(p21);

int main() 
{
	MOSFET.period_us(500);
	MOSFET = 0.0;
    while(1)
    {
    	for (int i = 0; i <= 10; i++)
    	{
    		if (switchInput == 1)
    		{
        		MOSFET = i/10.;
        		wait(2.0);
    		}
    		else
    			MOSFET = 0.0;
   		}
   		for (int i = 2; i <= 8; i += 2)
   		{
    		if (switchInput == 1)
    		{
       			MOSFET = (10-i)/10.;
       			wait(2.0);
    		}
    		else
    			MOSFET = 0.0;
   		}
    }
}
