#include "usb_functions.h"

/* TASKS
 */

void main(void)
{
    char received_char=0xFF,mask,love;
    int  masked_char, contador;
    usb_install(); // and usb is working :)

    do
    {
        love = poll_getc_cdc(&received_char);

        masked_char = received_char & 0b11000000;


        if ( masked_char == 0 )
        {
            contador=0;
            mask = 0b00100000;
            while(contador<6) {
                masked_char = received_char & mask;
                if( masked_char == 0 )
                {
                    putc_cdc('L');
                } else {
                    putc_cdc('H');
                }
                putc_cdc(love);
                mask = mask >> 1;
                contador ++;
            }
        }


    } while(1);
}
