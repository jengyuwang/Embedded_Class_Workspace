/*
 * myADXL345.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jengyu
 */

#ifndef MYADXL345_H_
#define MYADXL345_H_

#include "mbed.h"
#include "TextLCD.h"

#define SAMPLING_TIME_MS        100
#define CAL_SAMPLES                16
#define WAIT_TIME_FOR_CAL_S     2.

#define REG_DATA_FORMAT           0x31
#define REG_POWER_CTL             0x2D
#define REG_BW_RATE               0x2C
#define MEM_SIZE                  256       // memory buffer size must be either 256, 512, 1024 or 4096 when copying to flash
#define TARGET_SECTOR             29        //  use sector 29 as target sector if it is on LPC1768

const float AccSensitivity = 0.0039;        // Maximum resolution of 3.9 mg/LSB (per ADXL345 Datasheet)

const int addrADXL345 = (0x53 << 1);        // Address when grounding ALT ADDRESS pin

enum dir
{
    X_DIR = 0,
    Y_DIR = 2,
    Z_DIR = 4
};

struct CAL_DATA
{
    float xGain;
    float xOffset;
    float yGain;
    float yOffset;
    float zGain;
    float zOffset;
};

void ADXL345_init(void);
void ADXL345_meas(void);
float GetAccX(void);
float GetAccY(void);
float GetAccZ(void);
void Rx_interrupt(void);

Serial     pc(USBTX,USBRX);
TextLCD lcd(p25, p26, p21, p22, p23, p24); // rs, e, d4-d7

#endif /* MYADXL345_H_ */
