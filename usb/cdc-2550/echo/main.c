// Open source PIC USB stack echo demo
// USB stack by JTR and Honken
// CC-BY
//
// USB driver files should be in '../dp_usb/'
// Enter a USB VID and PID in prj_usb_config.h

//USB stack
#include "../dp_usb/usb_stack_globals.h"    // USB stack only defines Not function related.
#include "descriptors.h"	// JTR Only included in main.c
#include "configwords.h"	// JTR only included in main.c

// PIC18F Move reset vectors for bootloader compatibility
#ifdef __18CXX
	#define REMAPPED_RESET_VECTOR_ADDRESS		0x800
	#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
	#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
#endif

void InterruptHandlerHigh();
void InterruptHandlerLow();
void USBSuspend(void);

#pragma udata
extern BYTE usb_device_state;

#pragma code
#ifdef PIC_18F
void main(void)
#else
int main(void)
#endif
{
    BYTE RecvdByte;
    char cell[10];
    int count = 0, i;

    initCDC(); // setup the CDC state machine
    usb_init(cdc_device_descriptor, cdc_config_descriptor, cdc_str_descs, USB_NUM_STRINGS); // initialize USB. TODO: Remove magic with macro
    usb_start(); //start the USB peripheral

// PIC18 INTERRUPTS
// It is the users resposibility to set up high, low or legacy mode
// interrupt operation. The following macros for high and low interrupt
// setup have been removed:

//#define EnableUsbHighPriInterrupt()             do { RCONbits.IPEN = 1; IPR2bits.USBIP = 1; INTCONbits.GIEH = 1;} while(0) // JTR new
//#define EnableUsbLowPriInterrupt()              do { RCONbits.IPEN = 1; IPR2bits.USBIP = 0; INTCONbits.GIEL = 1;} while(0)  // JTR new

// By default, the interrupt mode will be LEGACY (ISR Vector 0x08)
// (Same as high priority vector wise but the operation (latency) is
// not the same. Consult the data sheet for details.)

// If a priority mode is enabled then this affects ALL other interrupt
// sources therefore it does not belong to the usb stack to be
// doing this. It is a global, user application choice.

#if defined USB_INTERRUPTS // See the prj_usb_config.h file.
    EnableUsbPerifInterrupts(USB_TRN + USB_SOF + USB_UERR + USB_URST);
#if defined __18CXX //turn on interrupts for PIC18
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
#endif
    EnableUsbGlobalInterrupt(); // Only enables global USB interrupt. Chip interrupts must be enabled by the user (PIC18)
#endif


// Wait for USB to connect
    do {
#ifndef USB_INTERRUPTS
        usb_handler();
#endif
    } while (usb_device_state < CONFIGURED_STATE);

    usb_register_sof_handler(CDCFlushOnTimeout); // Register our CDC timeout handler after device configured

// Main echo loop
    do {

// If USB_INTERRUPT is not defined each loop should have at least one additional call to the usb handler to allow for control transfers.
#ifndef USB_INTERRUPTS
        usb_handler();
#endif



// Receive and send method 1
// The CDC module will call usb_handler each time a BULK CDC packet is sent or received.
// If there is a byte ready will return with the number of bytes available and received byte in RecvdByte
        count = 0;
        while (poll_getc_cdc(&RecvdByte)) {
            cell[count++] = RecvdByte;
        }
        for ( i = 0; i < count; i++ )
          putc_cdc(cell[i]);

   //         putc_cdc(RecvdByte+1); //

// Receive and send method 2
// Same as poll_getc_cdc except that byte is NOT removed from queue.
// This function will wait for a byte and return and remove it from the queue when it arrives.
   //     if (peek_getc_cdc(&RecvdByte)) {
   //         RecvdByte = getc_cdc();
   //         putc_cdc(RecvdByte+1);
   //     }

// Receive and send method 3
// If there is a byte ready will return with the number of bytes available and received byte in RecvdByte
// use CDC_Flush_In_Now(); when it has to be sent immediately and not wait for a timeout condition.
    //    if (poll_getc_cdc(&RecvdByte)) {
    //        putc_cdc(RecvdByte+1); //
    //        CDC_Flush_In_Now();
    //    }

    } while (1);

} //end main

//board hardware setup
//add your hardware here

// USB suspend not yet enabled
void USBSuspend(void) {}

//interrupt routines for PIC 18 and PIC24
#if defined(USB_INTERRUPTS)

//PIC 24F type USB interrupts
#if defined(__PIC24FJ64GB106__) || defined(__PIC24FJ128GB106__) || defined(__PIC24FJ192GB106__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB108__) || defined(__PIC24FJ128GB108__) || defined(__PIC24FJ192GB108__) || defined(__PIC24FJ256GB108__) || defined(__PIC24FJ64GB110__) || defined(__PIC24FJ128GB110__) || defined(__PIC24FJ192GB110__) || defined(__PIC24FJ256GB110__) 
#pragma interrupt _USB1Interrupt
void __attribute__((interrupt, auto_psv)) _USB1Interrupt() {
    //USB interrupt
    //IRQ enable IEC5bits.USB1IE
    //IRQ flag	IFS5bits.USB1IF
    //IRQ priority IPC21<10:8>
    usb_handler();
    ClearGlobalUsbInterruptFlag();
}
#elif defined (__18CXX) //PIC18F style interrupts with remapping for bootloader
//	Interrupt remap chain
//
//This function directs the interrupt to
// the proper function depending on the mode
// set in the mode variable.
//USB stack on low priority interrupts,
#pragma interruptlow InterruptHandlerLow nosave= PROD, PCLATH, PCLATU, TBLPTR, TBLPTRU, TABLAT, section (".tmpdata"), section("MATH_DATA")
void InterruptHandlerLow(void) {
    usb_handler();
    ClearGlobalUsbInterruptFlag();
}

#pragma interrupt InterruptHandlerHigh nosave= PROD, PCLATH, PCLATU, TBLPTR, TBLPTRU, TABLAT, section (".tmpdata"), section("MATH_DATA")
void InterruptHandlerHigh(void) { //Also legacy mode interrupt.
        if(INTCONbits.TMR0IF){
            WriteTimer0(65536-23531); //23531 = 500ms
            INTCONbits.TMR0IF=0;
        }
	usb_handler();
    ClearGlobalUsbInterruptFlag();
}

//these statements remap the vector to our function
//When the interrupt fires the PIC checks here for directions
#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS

void Remapped_High_ISR(void) {
    _asm goto InterruptHandlerHigh _endasm
}

#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS

void Remapped_Low_ISR(void) {
    _asm goto InterruptHandlerLow _endasm
}

//relocate the reset vector
extern void _startup(void);
#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS

void _reset(void) {
    _asm goto _startup _endasm
}
//set the initial vectors so this works without the bootloader too.
#pragma code HIGH_INTERRUPT_VECTOR = 0x08

void High_ISR(void) {
    _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code LOW_INTERRUPT_VECTOR = 0x18

void Low_ISR(void) {
    _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
}
#endif
#endif

#pragma code

