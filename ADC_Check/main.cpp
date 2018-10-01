/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define V_REF            3.3
#define TRIMMER_LEVEL    11
#define CHANGE_LEVEL     (V_REF / TRIMMER_LEVEL / 4)

AnalogIn Vmeas(p20);

Serial pc(USBTX, USBRX);

int main() 
{
	float prevVolt = 0.0;
	float currentVolt = Vmeas * V_REF;
	int nCurVolt = 0;

	pc.printf("Current Voltage = %.2f\r\n", currentVolt);

    while(1) 
    {
    	currentVolt = Vmeas * V_REF;
    	if (abs(currentVolt - prevVolt) > CHANGE_LEVEL)
    	{
    		prevVolt = currentVolt;
        	pc.printf("Current Voltage = %.2f\r\n", currentVolt);
        	nCurVolt = Vmeas.read_u16();
        	pc.printf("Current Voltage = %d\r\n", nCurVolt);
    	}
    }
}
