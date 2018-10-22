/*
 * TMP102.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: jengyu
 */

#include "TMP102.h"

Serial pc(USBTX, USBRX);

/**
 * TMP102 constructor
 *
 * @param sda       PinName for sda line
 * @param scl       PinName for scl line
 */
TMP102::TMP102(PinName sda, PinName scl) : tempSensor(sda, scl)
{

}

/**
 * TMP102 destrcutor
 */
TMP102::~TMP102(void)
{

}

/**
 * Initialization
 */
void TMP102::Initialize(void)
{
    // Initialize TMP102
    config_t[0] = 0x01;                                   // set pointer register to 'config register' (Table 7 data sheet)
    config_t[1] = 0x60;                                   // config temperature measurements to 12-bit resolution (Table 10 data sheet)
    config_t[2] = 0xA0;                                   // configure temperature conversion rate to 4 Hz, AL to normal (Table 11 data sheet)
    tempSensor.write(addrTMP102, config_t, 3);            // write 3 bytes to device at address addrTMP102

    checkConfigData();
}

/**
 * Check if there is comfortable temperature stored in flash
 */
void TMP102::checkConfigData(void)
{
    int rc;

    // Check if calibration results are in the flash
    if ((rc = iap.blank_check(TARGET_SECTOR, TARGET_SECTOR)) == SECTOR_NOT_BLANK)
    {
        // Calibration result is available. Read from the flash
        configData = *(struct T_DATA *) FLASH_SECTOR_29;
        comfTemp = configData.comfTemp;
        //pc.printf("! Found saved configuration results: comfTemp = %+2.1f", configData.comfTemp);
    }
    else
    {
        // No data -> set to default
        comfTemp = 75.0;
        //pc.printf("! No saved configuration result. Use default: comfTemp = %+2.1f", comfTemp);
    }
}

/**
 * User defined comfort temperature. Update and save to the flash
 *
 * @param temp      comfortable temperature
 *
 * @return int      CMD_SUCCESS (0) or others for failures
 */
int TMP102::SetConmfTempInF(float temp)
{
    char mem[MEM_SIZE];                                     // RAM memory buffer to use when copying data to flash (removed static to make word aligned)

    int         rc;
    char*       p;

    comfTemp = temp;
    configData.comfTemp = comfTemp;

    memset(&mem[0], 0, sizeof(mem));                        // Set all elements of mem array to 0

    // Copy data struct into mem array
    p = (char *)&configData;
    for(uint i = 0; i< sizeof(T_DATA); ++i)
        mem[i] = *(p + i);

    // Check to see if TARGET_SECTOR is BLANK
    rc = iap.blank_check( TARGET_SECTOR, TARGET_SECTOR );
    // Erase Target Sector if NOT BLANK
    if (rc == SECTOR_NOT_BLANK ) {
        //pc.printf("TARGET SECTOR is NOT BLANK!  Erasing...\r\n");
        iap.prepare( TARGET_SECTOR, TARGET_SECTOR );        // Always must prepare sector before erasing or writing
        rc   = iap.erase( TARGET_SECTOR, TARGET_SECTOR );
        //pc.printf( "erase result       = 0x%08X\r\n", rc );
    }

    // copy RAM to Flash
    iap.prepare(TARGET_SECTOR, TARGET_SECTOR);              // Always must prepare sector before erasing or writing
    rc = iap.write(mem, sector_start_adress[TARGET_SECTOR], MEM_SIZE);

    //pc.printf("\r\nCopied: SRAM(0x%08X)->Flash(0x%08X) for %d bytes. (result=0x%08X)\r\n",
    //        mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE, rc);

    // compare
    rc = iap.compare(mem, sector_start_adress[TARGET_SECTOR], MEM_SIZE);
    //pc.printf("\r\nCompare result     = \"%s\"\r\n", rc ? "FAILED - Sector was probably not Blank before writing" : "OK");

    return rc;
}

/**
 * Get temperature in Celsius
 *
 * @return float    temperature in Celsius
 */
float TMP102::GetTemperatureInC(void)
{
    measure();
    return (TempSensitivity *                             // convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
            (((temp_read[0] << 8) + temp_read[1]) >> 4));
}

/**
 * Get temperature in Fahrenheit
 *
 * @return float    temperature in Fahrenheit
 */
float TMP102::GetTemperatureInF(void)
{
    measure();
    return (TempSensitivity *                             // convert to 12-bit temp data (see Tables 8 & 9 in data sheet)
            (((temp_read[0] << 8) + temp_read[1]) >> 4) * 9. / 5. + 32.);
}

/**
 * Measure the temperature
 *
 */
void TMP102::measure(void)
{
    config_t[0] = 0x00;                                   // set pointer register to 'data register' (Table 7 datasheet)
    tempSensor.write(addrTMP102, config_t, 1);            // send to pointer 'read temp'
    tempSensor.read(addrTMP102, temp_read, 2);            // read the 2-byte temperature data
}
