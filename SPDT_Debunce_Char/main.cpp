/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"

#define MAX_TRIALS_TO_PRINT    20

InterruptIn SPDT(p15);

Timer bounceTimer;

volatile bool bounceStarted = false;
volatile bool switchedOn = false;

Serial pc(USBTX, USBRX);

volatile unsigned int riseCount = 0;
volatile unsigned int fallCount = 0;

volatile int riseTime;
volatile int fallTime;

int totalRiseTimePool[MAX_TRIALS_TO_PRINT];
int totalFallTimePool[MAX_TRIALS_TO_PRINT];

int curRiseTimeIndex = 0;
int curFallTimeIndex = 0;

void CountingRise(void)
{
	if (bounceStarted == false)
	{
		// Off -> On case
		bounceTimer.reset();
		bounceStarted = true;
		switchedOn = true;
	}
	riseCount++;
	riseTime = bounceTimer.read_us();
}

void CountingFall(void)
{
	if (bounceStarted == false)
	{
		// On->Off case
		bounceTimer.reset();
		bounceStarted = true;
		switchedOn = false;
	}
	fallCount++;
	fallTime =  bounceTimer.read_us();
}

void ResetCollectedData(void)
{
	if (curRiseTimeIndex >= MAX_TRIALS_TO_PRINT)
	{
		pc.printf("\r\n**********************************************\r\n");
		pc.printf("Data Collected So Far for Switch On Bouncing DOE:");
		for (int i = 0; i < curRiseTimeIndex; i++)
		{
			if ((i % 10) == 0)
			{
				pc.printf("\r\n");
			}
			pc.printf("%d ", totalRiseTimePool[i]);
		}
		curRiseTimeIndex = 0;
		memset(totalRiseTimePool, 0, sizeof(totalRiseTimePool));
		pc.printf("\r\n");
	}

	if (curFallTimeIndex >= MAX_TRIALS_TO_PRINT)
	{
		pc.printf("\r\n**********************************************\r\n");
		pc.printf("Data Collected So Far for Switch Off Bouncing DOE:");
		for (int i = 0; i < curFallTimeIndex; i++)
		{
			if ((i % 10) == 0)
			{
				pc.printf("\r\n");
			}
			pc.printf("%d ", totalFallTimePool[i]);
		}
		curFallTimeIndex = 0;
		memset(totalFallTimePool, 0, sizeof(totalFallTimePool));
		pc.printf("\r\n");
	}
}

int main() 
{
	bounceTimer.start();
	bounceStarted = false;

	SPDT.rise(&CountingRise);
	SPDT.fall(&CountingFall);

    while(1)
    {
    	if (bounceTimer.read_ms() >= 1000)
    	{
    		// Should be settled
    		if ((riseCount != 0) || (fallCount != 0))
    		{
    			// Only print out when user toggle the switch
        		if (switchedOn)
        		{
            		pc.printf("+++++\tSwtiched On Bounces\t+++++\r\n");
            		pc.printf("riseCount = %d\t fallCount = %d\r\n", riseCount, fallCount);
            		pc.printf("last rise time (us) = %d\r\n", riseTime);
            		totalRiseTimePool[curRiseTimeIndex++] = riseTime;
        		}

        		else
        		{
        			pc.printf("-----\tSwtiched Off Bounces\t-----\r\n");
            		pc.printf("riseCount = %d\t fallCount = %d\r\n", riseCount, fallCount);
            		pc.printf("last fall time (us) = %d\r\n", fallTime);
            		totalFallTimePool[curFallTimeIndex++] = fallTime;
        		}

        		riseCount = 0;
        		fallCount = 0;
        		bounceStarted = false;
    		}

    		bounceTimer.reset();
    		ResetCollectedData();
    	}
    }
}
