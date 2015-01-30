/*
 *  Written in AVR Studio 5 / AVR Studio 6
 *  Compiler: AVR GNU C Compiler (GCC)
 *
 *  Author: AVR Tutorials
 *  Website: www.AVR-Tutorials.com
*/
 
#include <avr/io.h>
#include <avr/interrupt.h>
 
#define F_CPU 8000000UL
#include <util/delay.h>
 
#define DataPort	PORTC	// Using PortC as our Dataport
#define DataDDR		DDRC
 
//Interrupt Service Routine for INT0
ISR(INT6_vect)
{
	unsigned char i, temp;
	
	_delay_ms(500); // Software debouncing control
	
	temp = DataPort;	// Save current value on DataPort
	
	/* This for loop blink LEDs on Dataport 5 times*/
	for(i = 0; i<5; i++)
	{
		DataPort = 0x00;
		_delay_ms(500);	// Wait 5 seconds
		DataPort = 0xFF;
		_delay_ms(500);	// Wait 5 seconds
	}
	
	DataPort = temp;	//Restore old value to DataPort
}

ISR(INT1_vect)
{
	INT6_vect();
}

 
int main(void)
{
	DDRE = 1<< PE6;		// Set PD2 as input (Using for interupt INT0)
	PORTE = 1<< PE6;		// Enable PD2 pull-up resistor
 
	DataDDR = 0xFF;		// Configure Dataport as output
	DataPort = 0x01;	// Initialise Dataport to 1

	EIFR = 1 << INTF6;					// Enable INT0
	//MCUCR = 1<<ISC01 | 1<<ISC00;	// Trigger INT0 on rising edge
	EICRB = 1 << ISC61 | 1 << ISC60 ;
	EIMSK = 01 << INT6 ;
	sei();				//Enable Global Interrupt
	
    while(1)
    {
		if(DataPort >= 0x80)
			DataPort = 1;
		else
			DataPort = DataPort << 1;	// Shift to the left
 
		_delay_ms(500);	// Wait 5 seconds
    }
}
