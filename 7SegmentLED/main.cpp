/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define VALUE_PORT_MASK     0x00000FF0
#define VALUE_PORT_SHIFT    4
#define CONTROL_PORT_MASK   0x0000003C
#define CONTROL_PORT_SHIFT  2
#define FLASH_RATE_HZ       10
#define TIME_SLICE_MS       5
#define IDLE_COUNT          (1000 / FLASH_RATE_HZ / (3 * TIME_SLICE_MS))

PortOut ValuePort(Port0, VALUE_PORT_MASK);     // a = P0.4, b = P0.5, c = P0.6, d = P0.7, e = P0.8, f = P0.9, g = P0.10, dp = P0.11
PortOut ControlPort(Port2, CONTROL_PORT_MASK); // Digit1 = P2.2, Digit2 = P2.3, Digit3 = P2.4, Digit4 = P2.5

int Value[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

void Display(int digitShift, int valueIndex)
{
	ControlPort = ControlPort & ~(CONTROL_PORT_MASK);
	ControlPort = ControlPort | (1 << (CONTROL_PORT_SHIFT + digitShift));
	ValuePort = ValuePort & ~(VALUE_PORT_MASK);
	ValuePort = ValuePort | (Value[valueIndex] << VALUE_PORT_SHIFT);
	wait_ms(TIME_SLICE_MS);
}

int main() 
{
    while(1) 
    {
    	for (int Digit2 = 0; Digit2 <= 0xF; Digit2++)
    	{
    		for (int Digit3 = 0; Digit3 <= 0xF; Digit3++)
    		{
    			for (int Digit4 = 0; Digit4 <= 0xF; Digit4++)
    			{
    				for (int i = 0; i < IDLE_COUNT; i++)
    				{
    					Display(1, Digit2);
       					Display(2, Digit3);
       					Display(3, Digit4);
    				}
    			}
    		}
    	}
    }
}
