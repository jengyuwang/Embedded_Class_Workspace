/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

int main() 
{
    while(1) 
    {
        myled1 = 1;
        wait(0.334);
        myled1 = 0;

        myled2 = 1;
        wait(0.333);
        myled2 = 0;

        myled3 = 1;
        wait(0.333);
        myled3 = 0;

        myled4 = 1;
        wait(0.334);
        myled4 = 0;

        myled3 = 1;
        wait(0.333);
        myled3 = 0;

        myled2 = 1;
        wait(0.333);
        myled2 = 0;
    }
}
