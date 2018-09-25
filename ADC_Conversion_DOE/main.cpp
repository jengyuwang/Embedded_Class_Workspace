/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define SOURCE_FREQUENCY_HZ    4000
#define SAMPLE_SIZE            1000

AnalogIn Ain(p20);
PwmOut source(p21);

Serial pc(USBTX, USBRX);

unsigned short ADCdata[SAMPLE_SIZE] = {0};

void PrintData(unsigned short *pdata)
{
	pc.printf("Source Frequency %d Hz:", SOURCE_FREQUENCY_HZ);

	for (int i = 0; i < SAMPLE_SIZE; i++)
	{
		if ((i % 10) == 0)
		{
			pc.printf("\r\n");
		}
		pc.printf("%d\t", *(pdata+i));
	}

	pc.printf("\r\n\r\n");
}

int main() 
{
	source.period(1./SOURCE_FREQUENCY_HZ);
	source = 0.5;

	for (int i = 0; i < SAMPLE_SIZE; i++)
	{
		ADCdata[i] = Ain.read_u16();
	}

	PrintData(ADCdata);
}
