#include <p18f4550.h>
#include "tlc5940.h"
#include <timers.h>
#include <delays.h>
#include "hardware_init.h"
#include "usb_includes/usb_functions.h"




void high_isr(void);
// #pragma code high_vector=0x08
 void interrupt_at_high_vector(void)
 {
  _asm GOTO high_isr _endasm
 } 

#pragma code

#pragma interrupt high_isr
 void high_isr(void) {
     if(INTCONbits.TMR0IF){
         WriteTimer0(XLATCOUNTTIMER); //23531 = 500ms
         processXLATinterrupt();
         INTCONbits.TMR0IF=0;
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
  int brightness=0;
  unsigned char channel = 0;
  char received_char=0xFF;
  TRISA = 0x00;
  TRISB = 0x00;
  TRISC = 0x00;
  PORTA = 0x00;
  PORTB = 0x00;
  PORTC = 0x00;
  ADCON1 = 0x0F;

  usb_install();

  SSPCON1bits.SSPEN = 1;
  initialiseTlc5940();

  while(1) {
      channel=0;
      brightness=3891;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=1;
      brightness=3879;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=2;
      brightness=3867;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=3;
      brightness=3855;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=4;
      brightness=3843;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=5;
      brightness=3831;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=6;
      brightness=3819;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      channel=7;
      brightness=3807;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=8;
      brightness=3795;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=9;
      brightness=3783;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=10;
      brightness=3771;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=11;
      brightness=3759;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=12;
      brightness=3747;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=13;
      brightness=3735;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=14;
      brightness=3723;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

       channel=15;
      brightness=3686;
      setGrayScaleValue(channel, brightness);
      updateTlc5940();

      poll_getc_cdc(&received_char);
  }
}
