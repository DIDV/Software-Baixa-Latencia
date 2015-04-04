#include "init_usb.h"
#include "tlc5940.h"
#include <delays.h>

void usb_install(void)
{

    initCDC(); // setup the CDC state machine
    usb_init(cdc_device_descriptor, cdc_config_descriptor, cdc_str_descs, USB_NUM_STRINGS); // initialize USB. TODO: Remove magic with macro
    usb_start(); //start the USB peripheral

    do {
        usb_handler(); //only needed for polling!!
    } while (usb_device_state < CONFIGURED_STATE);
    usb_register_sof_handler(CDCFlushOnTimeout);

}

void USBSuspend(void) {} // declarado para a stack funcionar.

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

void tlc_test (int brightness,unsigned char channel)
{
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
}


// #pragma code


