/* Lcd_Custom_Char

Author: Lluis Nadal.
Date: 15th October 2010.

Up to 8 characters can be programmed in CGRAM, but because CGRAM is a RAM, characters are lost
on power off and must be reloaded on power on.
Assuming 2x16 LCD with HD44780 Hitachi controller compatible.

*/

#include "mbed.h"
#include "TextLCD.h"


// Defines LCD connections.
//TextLCD lcd(p24, p26, p27, p28, p29, p30); // rs, e, d4, d5, d6, d7
TextLCD lcd(p12, p20, p21, p22, p23, p24); // rs, e, d4-d7

// Defines 8 custom characters.
char custom_char[8][8] = {
    {0x07,0x08,0x1F,0x08,0x1F,0x08,0x07,0x00}, // Euro sign.
    {0x00,0x0E,0x11,0x11,0x11,0x0A,0x1B,0x00}, // Ohm sign.
    {0x00,0x00,0x00,0x12,0x12,0x12,0x1C,0x10}, // Micro sign.
    {0x00,0x00,0x1F,0x0A,0x0A,0x0A,0x0A,0x00}, // Pi sign.
    {0x0C,0x12,0x12,0x0C,0x00,0x00,0x00,0x00}, // Degree sign.
    {0x0E,0x04,0x0E,0x15,0x015,0x0E,0x04,0x0E},// Phi sign.
    {0x04,0x0E,0x15,0x04,0x04,0x04,0x04,0x04}, // Arrow up.
    {0x04,0x04,0x04,0x04,0x04,0x15,0x0E,0x04}  // Arrow down.
};


// Defines LCD bus to write data.
BusOut Lcd_pins(p27, p28, p29, p30); // d4, d5, d6, d7

DigitalOut rs_pin(p24); // LCD pin rs (register select.)
DigitalOut e_pin(p26);  // LCD pin e (enable).


// Because we use 4 bit LCD, data must be sent in two steps.
void writePort(int value) {

    Lcd_pins = value >> 4;  // Shifts 4 bit right.
    wait(0.000040f); // Wait 40us.
    e_pin = 0;
    wait(0.000040f);
    e_pin = 1;
    Lcd_pins = value;
    wait(0.000040f);
    e_pin = 0;
    wait(0.000040f);
    e_pin = 1;
}



int main() {


    lcd.cls();
    lcd.printf("Loading...");
    wait(2);


    for (int j=0; j<8; j++) {

        rs_pin = 0; // We send a command.

        /* 0X40 is the initial CGRAM address. Because each character needs a total amount of 8 memory
        locations, we increment addres in 8 units after each character.
        */
        writePort(0x40+8*j);


// Writes data.
        rs_pin = 1; // We send data.


        for (int i=0; i<8; i++) {
            writePort(custom_char[j][i]);
        }
    }



    lcd.cls();
    wait(0.010);

    lcd.printf("Custom character");
    lcd.locate(0,1);

// Prints loaded custom characters. ASCII codes 0 to 7.
    for (int j=0; j<8; j++) {
        lcd.putc(j);
    }

}
