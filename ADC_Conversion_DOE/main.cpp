/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define SOURCE_FREQUENCY_HZ    10000
#define SAMPLE_SIZE            200

#define ADC_CLK_EN (1<<12)
#define SEL_AD0_0  (1<<0) //Select Channel AD0.0
#define CLKDIV     1 // ADC clock-divider (ADC_CLOCK=PCLK/CLKDIV+1)
#define PWRUP      (1<<21) //setting it to 0 will power it down
#define START_CNV  (1<<24) //001 for starting the conversion immediately
#define ADC_DONE   (1U<<31) //define it as unsigned value or compiler will throw #61-D warning
#define ADCR_SETUP_SCM ((CLKDIV<<8) | PWRUP)

AnalogIn Ain(p15);
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
	wait(1.);

	LPC_SC->PCONP |= ADC_CLK_EN; //Enable ADC clock
	LPC_ADC->ADCR =  ADCR_SETUP_SCM | SEL_AD0_0;

	for (int i = 0; i < SAMPLE_SIZE; i++)
	{
		LPC_ADC->ADCR |= START_CNV;
		while((LPC_ADC->ADGDR & ADC_DONE) == 0); //this loop will end when bit 31 of AD0DR6 changes to 1.
		ADCdata[i] = (LPC_ADC->ADGDR >> 4) & 0xFFF;
	}

	PrintData(ADCdata);
}
