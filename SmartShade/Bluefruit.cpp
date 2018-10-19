/*
 * Bluefruit.cpp
 *
 *  Created on: Oct 18, 2018
 *      Author: jengyu
 */

#include "Bluefruit.h"
#include "APDS9960_I2C.h" // for direction enumeration

#ifdef SS_DEBUG
Serial pc(USBTX, USBRX);
#endif

Bluefruit::Bluefruit(PinName rx, PinName tx) : Serial (tx, rx), blue(tx, rx, NULL, 9600)//, myled(LED1,LED2,LED3,LED4)
{
	bnum = 0;
	bhit = 0;
}

Bluefruit::~Bluefruit(void)
{

}

bool Bluefruit::IsDataAvailable(void)
{
	bool ckSum = false;

    if (blue.getc()=='!')
    {
    	if (blue.getc()=='B')
    	{
    		//button data packet
    		bnum = blue.getc(); //button number
            bhit = blue.getc(); //1=hit, 0=release

            if (blue.getc()==char(~('!' + 'B' + bnum + bhit)))
            {
                //myled = bnum - '0'; //current button number will appear on LEDs
            	ckSum = true;
            }
    	}
    }

#ifdef SS_DEBUG
		pc.printf("BLE: ckSum = %d !\r\n", ckSum);
#endif

    return ckSum;
}

/*
bool Bluefruit::ParseData(void)
{
    switch (state)
    {
    case start:
    	if (blue.getc()=='!') state = got_exclm;
        else state = start;
        break;
    case got_exclm:
        if (blue.getc() == 'B') state = got_B;
        else state = start;
        break;
    case got_B:
        bnum = blue.getc();
        state = got_num;
        break;
    case got_num:
        bhit = blue.getc();
        state = got_hit;
        break;
    case got_hit:
        if (blue.getc() == char(~('!' + ' B' + bnum + bhit))) button_ready = 1;
        state = start;
        break;
    default:
        blue.getc();
        state = start;
    }

    return button_ready;
}
*/
int Bluefruit::ReadInput(void)
{
	int dir;

	switch (bnum)
    {
    case '5': //button 5 up arrow
    	dir = DIR_UP;
       	break;
    case '6': //button 6 down arrow
    	dir = DIR_DOWN;
    	break;
    case '7': //button 7 left arrow
    	dir = DIR_LEFT;
    	break;
    case '8': //button 8 right arrow
    	dir = DIR_RIGHT;
    	break;
    default:
    	dir = DIR_ALL;
    	break;
    }

#ifdef SS_DEBUG
		pc.printf("BLE got direction: %d !\r\n", dir);
#endif

	//button_ready = 0;
	return dir;
}
