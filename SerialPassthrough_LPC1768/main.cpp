/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include "mbed.h"
Serial Blue(p28,p27);
DigitalOut myled(LED1);
DigitalOut myled4(LED4);

Serial pc(USBTX, USBRX);

//global variables for main and interrupt routine
volatile bool button_ready = 0;
volatile int  bnum = 0;
volatile int  bhit  ;
//state used to remember previous characters read in a button message
enum statetype {start = 0, got_exclm, got_B, got_num, got_hit};
statetype state = start;
//Interrupt routine to parse message with one new character per serial RX interrupt
void parse_message()
{
    switch (state) {
        case start:
            if (Blue.getc()=='!') state = got_exclm;
            else state = start;
            break;
        case got_exclm:
            if (Blue.getc() == 'B') state = got_B;
            else state = start;
            break;
        case got_B:
            bnum = Blue.getc();
            state = got_num;
            break;
        case got_num:
            bhit = Blue.getc();
            state = got_hit;
            break;
        case got_hit:
            if (Blue.getc() == char(~('!' + ' B' + bnum + bhit))) button_ready = 1;
            state = start;
            break;
        default:
            Blue.getc();
            state = start;
    }

	pc.printf("Serial interrupt on BLE. state = %d, button_ready = %d \r\n", state, button_ready);
}

int main()
{
//attach interrupt function for each new Bluetooth serial character
    Blue.attach(&parse_message,Serial::RxIrq);
    while(1) {
        //check for a new button message ready
        if(button_ready && (bnum=='4')) { // button 4 changed
            myled4 = bhit - '0'; //turn led4 on/off
            button_ready = 0; //reset flag after reading button message
        }
        //do other tasks in main - interrupts will process button message characters
        myled = 1;
        wait(0.1);
        myled = 0;
        wait(0.1);
    }
}

/*
#include "mbed.h"

RawSerial  pc(USBTX, USBRX);
RawSerial  dev(p28,p27);
DigitalOut led1(LED1);
DigitalOut led4(LED4);

void dev_recv()
{
    led1 = !led1;
    while(dev.readable()) {
        pc.putc(dev.getc());
    }
}

void pc_recv()
{
    led4 = !led4;
    while(pc.readable()) {
        dev.putc(pc.getc());
    }
}

int main()
{
    pc.baud(9600);
    dev.baud(9600);

    pc.attach(&pc_recv, Serial::RxIrq);
    dev.attach(&dev_recv, Serial::RxIrq);

    while(1) {
        sleep();
    }
}
*/
