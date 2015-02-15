// PIC18F2550 Configuration Bit Settings

//Definitions About Frequency
#pragma config PLLDIV = 5
#pragma config CPUDIV = OSC1_PLL2
#pragma config USBDIV = 2
#pragma config  FOSC = HSPLL_HS

#pragma config	FCMEN = OFF			 //no fail safe clock monitor
#pragma config	IESO = OFF			  //oscillator switchover disabled
#pragma config  PWRT = OFF                          //oscillator power up timer disabled (release version only)
#pragma config  BOR = OFF
#pragma config	WDT = OFF			 //watchdog timer disabled
#pragma config  MCLRE = ON  		        //MCLR pin enabled
#pragma config	LVP = OFF  			  //low voltage programming disabled
#pragma config	XINST = OFF                    //do not allow PIC18 extended instructions
#pragma config  STVREN = ON                   //stack overflow will cause reset
#pragma config DEBUG = ON /* enable background debugger */
#pragma config ICPRT = OFF	// Dedicated In-Circuit Debug/Programming
#pragma config	CP0 = OFF
#pragma config	CP1 = OFF
#pragma config	CP2 = OFF
#pragma config	CP3 = OFF
#pragma config	CPB = OFF

