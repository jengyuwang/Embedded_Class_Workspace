/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"
#include "myADXL345.h"


void printAccToLCD(float x, float y, float z)
{
    lcd.cls();
    lcd.printf("x:%+2.2f y:%+2.2f", x, y);

    lcd.locate(0,1);
    lcd.printf("z:%+2.2f", z);
}

int main()
{
    float   accX, accY, accZ;

    pc.baud(9600);
    pc.printf("\r\n<............\t Starting ADXL345 \t............>\r\n\r\n");
    pc.attach(&Rx_interrupt, Serial::RxIrq);        // use serial interrupt to interact with user

    // Clear LCD screen
    lcd.cls();

    // Initialize ADXL345
    ADXL345_init();

    while(1)
    {
        wait_ms(1000);
        ADXL345_meas();

        accX = GetAccX();
        accY = GetAccY();
        accZ = GetAccZ();

        printAccToLCD(accX, accY, accZ);
    }
}
