/*
 * Bluefruit.h
 *
 *  Created on: Oct 18, 2018
 *      Author: jengyu
 */

#ifndef BLUEFRUIT_H_
#define BLUEFRUIT_H_

#define SS_DEBUG
#include "mbed.h"


// State used to remember previous characters read in a button message from BLE
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};

class Bluefruit : public RawSerial
{
public:

    Bluefruit(PinName rx, PinName tx);
    ~Bluefruit();

    bool IsDataAvailable(void);
    bool ParseData(void);

    int ReadInput(void);
    //void ReadData(void);

private:
    RawSerial    blue;
    //BusOut    myled;

    //char bnum;
    //char bhit;

    statetype state = start;

    //global variables for main and interrupt routine
    volatile bool button_ready = 0;
    volatile int  bnum = 0;
    volatile int  bhit;

};




#endif /* BLUEFRUIT_H_ */
