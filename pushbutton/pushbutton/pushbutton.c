/*
 * bush_button.c
 *
 * Created: 1/15/2015 7:25:23 PM
 *  Author: Ben
 */ 

/*	Sample program for Olimex AVR-CAN with AT90CAN128 processor
 *	Press the BUTTON the turn LED on
 *	Compile with AVRStudio+WinAVR (gcc version 3.4.6)
 */

#define	__AVR_AT90CAN128__	1
#define OSCSPEED	16000000		/* in Hz */

#include "avr/io.h"
extern signed short angle = 0;
static unsigned char ab = 0;
const signed short table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

void PORT_Init()
{
	PORTA = 0b00000000;
	DDRA = 0b00000000;

	PORTB = 0b00000000;
	DDRB = 0b00000000;

	PORTC = 0b00000000;
	DDRC = 0b00000000;

	PORTD = 0b00000000;
	DDRD = 0b00000000;

	PORTE = 0b00000000;
	DDRE = 0b00010000;		//Led set as output (Bit4 = 1) , Button set as input(Bit 5 = 0)

	PORTF = 0b00000000;
	DDRF = 0b00000000;
}

void WDT_Off(void)
{
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = 0x07;				//increase Watchdog time-out period (Bit0,1,2 = 1)
}

int main()
{
	
	
	WDT_Off();
	PORT_Init();
	while (1)
	{
		asm("WDR");			//avoiding reset of the program
		ab = ab << 2;
		ab |= (PORTD & 0x3);
		angle += table[(ab & 0xf)];
		if ((!(PINE & 0b00100000)) | (!(PINE & 0b01000000))	)		//if button is pressed (Bit5 = 0) or encoder button
			PORTE = PORTE & 0b11101111;		//led on (Bit4 = 0)
		else
			PORTE = PORTE | 0b00010000;		//led off (Bit4 = 1)
	}
}
