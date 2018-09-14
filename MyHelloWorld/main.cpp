/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define NUM_OF_LEDS				4												// How many LEDs
#define BLINKY_LED_CYCLE_IN_S	2.0												// Cycle in second
#define BLINKY_LED_INTERVAL		((2 * NUM_OF_LEDS) - 2)							// How many intervals in one cycle
#define BLINKY_LED_ON_TIME		(BLINKY_LED_CYCLE_IN_S / BLINKY_LED_INTERVAL)	// On time for each LED

BusOut myleds(LED1, LED2, LED3, LED4);

int main() 
{
    while(1) 
    {
        for (int i = 0; i < NUM_OF_LEDS; i++)
    	{
    		// Forward lightening
    		myleds[i] = 1;
    		wait(BLINKY_LED_ON_TIME);
    		myleds[i] = 0;
    	}

        for (int i = (NUM_OF_LEDS - 2); i > 0; i--)
    	{
    		// Backward lightening
    		myleds[i] = 1;
    		wait(BLINKY_LED_ON_TIME);
    		myleds[i] = 0;
    	}
    }
}
