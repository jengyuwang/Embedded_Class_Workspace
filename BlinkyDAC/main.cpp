/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define V_REF            3.3
#define TRIMMER_LEVEL    11
#define CHANGE_LEVEL     (V_REF / TRIMMER_LEVEL)

AnalogIn Vmeas(p20);
AnalogOut greenLed(p18);

Serial pc(USBTX, USBRX);

int main() 
{
	float prevVolt = 0.0;
	float currentVolt = Vmeas * V_REF;
	pc.printf("Current Voltage = %.2f\r\n", currentVolt);

    while(1) 
    {
    	currentVolt = Vmeas * V_REF;
    	if (abs(currentVolt - prevVolt) > CHANGE_LEVEL)
    	{
    		prevVolt = currentVolt;
        	pc.printf("Current Voltage = %.2f\r\n", currentVolt);
    	}
    	greenLed = Vmeas;
    }
}
