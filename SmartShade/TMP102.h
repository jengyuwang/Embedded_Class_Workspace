/*
 * TMP102.h
 *
 *  Created on: Oct 16, 2018
 *      Author: jengyu
 */

#ifndef TMP102_H_
#define TMP102_H_

#include "mbed.h"
#include "IAP.h"

#define MEM_SIZE                  256       // memory buffer size must be either 256, 512, 1024 or 4096 when copying to flash
#define TARGET_SECTOR             29        // use sector 29 as target sector if it is on LPC1768

struct T_DATA
{
    float comfTemp;
};

class TMP102
{
public:

    TMP102(PinName sda, PinName scl);
     ~TMP102(void);

    void Initialize(void);

    float GetTemperatureInF(void);
    float GetTemperatureInC(void);
    float GetComfTempInF(void){return comfTemp;};

    int SetConmfTempInF(float temp);

protected:

    void measure(void);
    void checkConfigData(void);

private:

    const float     TempSensitivity = 0.0625;    // degrees C/LSB (per TMP102 Data sheet)
    const int       addrTMP102 = 0x90;           // Default I2C address of TMP102

    float           comfTemp;

    struct T_DATA   configData;

    char            config_t[4];
    char            temp_read[2];

    I2C             tempSensor;
    IAP             iap;
};

#endif /* TMP102_H_ */
