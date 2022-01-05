/**
* @file Port.c
* @author dg1vs
* @date 2010/07/09
* @brief Port handler
*
*
*/

#include "IOPins.h"
#include "Rtc.h"

//----------------------------------------------------------------------------
// InitIO
//
// Initialize all static IO lines
//
// -> --
// <- --
//----------------------------------------------------------------------------
void IOPins_Init(void)
{

}

void IOPins_DeInit(void);

/*
#define X_FPGA_SPI_DS   PB3
#define X_FPGA_SPI_RS   PB1
#define X_FPGA_BIT_SPI_DS   3
#define X_FPGA_BIT_SPI_RS   1
*/
	/*
	// Port A
	DDRA = 0x00;                                             // Encoder input only
	PORTA = 0x00;
	
	DDRA  |= (1<<FPGA_DIN) | (1<<FPGA_CCLK) | (1<<FPGA_PROG); // FPGA Config
	PORTA |= (1<<FPGA_PROG);                                  // FPGA Config
	PORTA &= ~(1<<FPGA_DIN) | ~(1<<FPGA_CCLK);
	// DDRA  = (1<<FPGA_SPI_DS) | (1<<FPGA_SPI_RS);             // FPGA SPI dopplet belegt

	DDRA  |= (1<<DS1302_RST);
	PORTA &= ~(1<<DS1302_RST);                                  // RTC chip select low(!)

	// TODO check Port B und alle Ports
	
	// Port B
	DDRB  = (1<<X_FPGA_SPI_DS) | (1<<X_FPGA_SPI_RS);             // FPGA SPI
	DDRB |= (1<<PORTB4);                                     // SD SS chip select high(!) (LED off)
	DDRB |= (1<<DS1302_CLK) | (1<<DS1302_IO);       // RTC setup



	PORTB = (1<<X_FPGA_SPI_DS) | (1<<X_FPGA_SPI_RS);             // FPGA SPI
	PORTB |= (1<<PORTB2);                                    // INT2
	PORTB |= (1<<PORTB4);                                    // SD SS chip select
	PORTB |= (1<<DS1302_CLK) | (1<<DS1302_IO);                     // RTC setup
	

	// Port C

	// Port D
	DDRD = 0x0C;                                             // RxD+TxD, LEDs and module address
	PORTD = 0xFC;
	*/