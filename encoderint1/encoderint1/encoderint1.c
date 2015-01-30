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
#define F_CPU 8000000UL 

#include <avr/io.h>
#include <avr/interrupt.h>  
#include <util/delay.h>

#define processor AT90CAN128
#define OSCSPEED	8000000		/* in Hz */
#define DataPort	PORTC	// Using PortC as our Dataport
#define DataDDR		DDRC
#define encoder		PORTD  //portD  0,1 are encoder ab
#define CHA PD0
#define CHB PD1
volatile long enc_count = 63;



void PORT_Init()
{
	PORTA = 0b00000000;
	DDRA = 0b00000000;

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
	static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
	static uint8_t enc_val = 0;
	enc_val = enc_val <<2;
	enc_val = enc_val | (PIND & 0b0011) ;
	enc_count = enc_count + lookup_table[enc_val & 0b1111];
}
ISR(INT1_vect)
{
	//ISR(INT0_vect);
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
	DataPort = 0x01;	// Initialise Dataport to 1

	EIFR  = 1 << INTF6 | 1 << INTF0 | 1<< INTF1 ;					// Enable INT6,int 0, int1
	
	EICRB = 1 << ISC61 | 1 << ISC60 ;
	EICRA = 1 << ISC00 | 0 << ISC01 | 1 << ISC10 | 0 << ISC11 ;
	EIMSK = 1 << INT6  | 1 << INT0  | 1 << INT1 ;
	sei();				//Enable Global Interrupt
	unsigned char Ch;
	WDT_Off();
	
	UART_Init(9600);
	// enc_count = 255;
    while(1)
    {
		DataPort = !(enc_count & 0xFF) ;  // encoder display
		
	//	if(DataPort >= 0x80)
	//		DataPort = 1;
	//	else
	//		DataPort = DataPort << 1;	// Shift to the left
 
		_delay_ms(5);	// Wait .05 seconds
		
		asm("WDR");				//avoiding reset of the program
		
		Ch = UART_Receive();	//receiving char
		if (Ch)
		{
			UART_Transmit(Ch);	//returning char
		}
    }

	
}