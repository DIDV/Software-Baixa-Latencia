#include <p18f4550.h>
#include "tlc5940.h"
#include <delays.h>
#include <pwm.h>
#include <timers.h>
#include "hardware_init.h"


void high_isr(void);
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
  _asm GOTO high_isr _endasm
}

#pragma code
#pragma interrupt high_isr
void high_isr (void) {
 if (INTCONbits.TMR0IF){  // Interrupt Check
     INTCONbits.TMR0IF = 0;				// Clear the timer0 interrupt flag
     processXLATinterrupt();
     WriteTimer0(XLATCOUNTTIMER);
    }
}

void Delay_uS(char uSec) {

   do {
       Delay1TCY();            // 1
       Delay1TCY();            // 1
       Delay1TCY();            // 1
       Delay1TCY();            // 1
       Nop();                        // 1
       Nop();                        // 1
       Nop();                        // 1
        } while(--uSec);        // 3
}

void Delay_ms(int mSec){
    int i;
    for(i=0; i<mSec; i++)
    Delay100TCYx(120);
}

void main (void)
{

    int brightness=250;
    unsigned char channel = 0;
    //unsigned char data[32];
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    ADCON1 = 0x0F;
    initialiseTlc5940();
    
    //while (1) {
        channel = 0;
        for (;channel < 16;channel++){
        setGrayScaleValue(channel, brightness);
        brightness++;
        updateTlc5940();
        }
    //}
}
