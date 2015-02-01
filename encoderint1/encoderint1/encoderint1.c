/*
 * encoderint1.c
 *
 * Created: 1/20/2015 7:03:06 PM
 *  Author: Ben
 
 *  Written in AVR Studio 5 / AVR Studio 6
 *  Compiler: AVR GNU C Compiler (GCC)
 *
 *  Author: AVR Tutorials
 *  Website: www.AVR-Tutorials.com
*/
#define F_CPU 16000000UL 

#include <avr/io.h>
#include <avr/interrupt.h>  
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/atomic.h>
//#include <stdio.h>
#include <string.h>


#define processor AT90CAN128
#define OSCSPEED	16000000		/* in Hz */
#define ts_port		PORTA
#define ts_DDR		DDRA
#define DataPort	PORTC	// Using PortC as our Dataport
#define DataDDR		DDRC
#define encoder		PORTD  //portD  0,1 are encoder ab
#define CHA PD0
#define CHB PD1
volatile  int32_t enc_count = 0;
volatile long loop = 0;
volatile uint32_t a =0;
volatile uint32_t b =0;
const char lf = 10;
const char cr = 13;
const char sp = 32;
const int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
volatile uint8_t enc_val = 0;

void PORT_Init()
{
	//PORTA = 0b00000000;
	//DDRA = 0b11111111;

	PORTB = 0b00000000;
	DDRB = 0b00000000;

	//PORTC = 0b00000000;
	//DDRC = 0b11111111;

	PORTD = 0b00000000;
	DDRD = 0b00000000;

	PORTE = 0b00000000;		//Turn Led off (Bit4 = 1)
	DDRE = 0b00000010;		//, TX set as output (Bit1 = 1)

	PORTF = 0b00000000;
	DDRF = 0b00000000;
}
ISR(INT0_vect)
{
	//ts_port = ts_port | 0b00000001 ;
	
	enc_val = enc_val <<2;	 
	enc_val = enc_val | (PIND & 0b0011) ;
	//ts_port = (enc_val & 0b1111);
	enc_count = enc_count + lookup_table[enc_val & 0b1111];
	//DataPort = (enc_count &0xFF);
	 //if (PIND & 0b0001) enc_count++;
	//ts_port ;
	//a+= 1;
	//ts_port = ts_port & 0b11111110 ;
	
}
ISR(INT1_vect)
{
	//ts_port = ts_port | 0b00000010 ;
	//if (PIND & 0b0010) enc_count--;
	//b+= 1;
	enc_val = enc_val <<2;	 
	enc_val = enc_val | (PIND & 0b0011) ;
	enc_count = enc_count + lookup_table[enc_val & 0b1111];
	//ts_port = ts_port & 0b11111101 ;
}


//Interrupt Service Routine for INT6
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

void WDT_Off(void)
{
	asm("push r30");	// using GCC without optimizations results in
	asm("ldi r30, 0x18");	// a code not meeting the requirements
	asm("sts 0x60, r30");	// of the datasheet.
	asm("ldi r30, 0x7");
	asm("sts 0x60, r30");
	asm("pop r30");
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = 0x07;					//increase Watchdog time-out period (Bit0,1,2 = 1)
}

void UART_Init(uint32_t Baud)
{
	unsigned int BaudRate = OSCSPEED / (16 * Baud) - 1;	//calculate BaudRate
	//set BaudRate into registers
	UBRR0H = (unsigned char) (BaudRate>>8);
	UBRR0L = (unsigned char) BaudRate;

	UCSR0B = 0b00011000;		//enable TX(Bit4 = 1), RX(Bit3 = 1)

	UCSR0C = 0b00000110;		//set frame format (8 bits, 1 stop bit)
}

unsigned char UART_Receive()
{
	if (UCSR0A & 0b10000000)
	return UDR0;
	else
	return 0;
}

void UART_Transmit(unsigned char Data)
{
	while (!(UCSR0A & 0b00100000));
	UDR0 = Data;
}


 
int main(void)
{
	DDRE = 1<< PE6;		// Set PE6 as input (Using for interupt INT0)
	PORTE = 1<< PE6;		// Enable PE6 pull-up resistor
	PORT_Init();
	DataDDR = 0xFF;		// Configure Dataport as output
	DataPort = 0x01;
	ts_DDR = 0xFF;
	ts_port = 0x00;	// Initialise Dataport to 0

	EIFR  = 1 << INTF6 | 1 << INTF0 | 1<< INTF1 ;					// Enable INT6,int 0, int1
	
	EICRB = 1 << ISC61 | 1 << ISC60 ;
	EICRA = 1 << ISC00 | 0 << ISC01 | 1 << ISC10 | 0 << ISC11 ;
	EIMSK = 1 << INT6  | 1 << INT0  | 1 << INT1 ;
	sei();				//Enable Global Interrupt
	//unsigned char Ch;
	WDT_Off();
	int ctr ;
	UART_Init(9600);
	char txchar [13] = "000000000000";

	
    while(1)
    {
		{
		ATOMIC_BLOCK(ATOMIC_FORCEON);
		
		DataPort = (enc_count & 0xFF) ;  // encoder display	
		}	
		
		ltoa(enc_count,txchar,10);
		for (ctr =0;ctr <(strlen(txchar)); ctr ++)
		{
		UART_Transmit(txchar[ctr]);	
		
		}
		
		UART_Transmit(cr);
		UART_Transmit(lf);
		ultoa(a,txchar,10);
		for (ctr =0;ctr < (strlen(txchar)); ctr ++)
		{
			UART_Transmit(txchar[ctr]);
		}
		for (ctr =0; ctr<10;ctr++)
		{
			UART_Transmit(sp);
		}
		
		ultoa(b,txchar,10);
		for (ctr =0;ctr < (strlen(txchar)); ctr ++)
		{
			UART_Transmit(txchar[ctr]);
		}
		UART_Transmit(cr);
		UART_Transmit(lf);
		//if(ts_port >= 0x80)
		//	ts_port = 1;
		//else
		//	ts_port = ts_port << 1;	// Shift to the left
 
		//_delay_ms(50);	// Wait .05 seconds
		
		//asm("WDR");				//avoiding reset of the program
		
		//Ch = UART_Receive();	//receiving char
		//if (Ch)
		//{
		//	UART_Transmit(Ch);	//returning char
		//}
    }

	
}