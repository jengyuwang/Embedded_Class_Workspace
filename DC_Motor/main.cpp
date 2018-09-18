/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

DigitalIn  switchInput(p17, PullNone);
DigitalOut MOSFET(p21);

int main() 
{
    while(1) 
    {
    	MOSFET = (switchInput == 1) ? 1 : 0;
    }
}
