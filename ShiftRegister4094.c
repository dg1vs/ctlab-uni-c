/*
 *   74HCT4094 shift register driver
 *
 *	 Copyright (C) 2014 Karsten Schmidt <karsten.schmidt@bu3sch.de>
 *   Copyright (C) 2009 Michael Buesch <mb@bu3sch.de>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2 as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 */

#include "ShiftRegister4094.h"
#include "IOPins.h"
#include "main.h"
#include "helper_macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void ShiftRegister4094_ShiftOut(void)
{
    uint8_t sreg = SREG;
    cli();

    uint8_t spcr = SPCR;            // save SPI settings for SD card
    uint8_t spsr = SPSR;            // save SPI settings for SD card
	    
    SPCR = 0;						// disable SPI interface

	/*
	SPI BUS CONFIGURATION 4094
	-----------------------------
	*Master Mode
	*MSB first
	*CPOL=0
	*CPHA=0
	*Above two implies SPI MODE0
	*SCK Freq = FCPU/16 i.e. 1,x MHz
	*/
			
	SPCR|=(1<<SPE)|(1<<MSTR)|(1<<SPR0);

	// Strobe low	
	set_output_bitval(SPI_STR_SHIFT_REGISTER,0);

	// TODO DKS for loop	
	// shift out
	SPDR = Params.RelayPortArray[3];
	while (!(SPSR & (1<<SPIF)));

	SPDR = Params.RelayPortArray[2];
	while (!(SPSR & (1<<SPIF)));

	SPDR = Params.RelayPortArray[1];
	while (!(SPSR & (1<<SPIF)));

	SPDR = Params.RelayPortArray[0];
	while (!(SPSR & (1<<SPIF)));

	// Strobe high -> move to output register
	set_output_bitval(SPI_STR_SHIFT_REGISTER,1);
	nop(); nop(); nop(); nop(); 
	// further data has no effect
	set_output_bitval(SPI_STR_SHIFT_REGISTER,0);
	
	// Restore settings for SD mode
	SPCR = spcr;                    
	SPSR = spsr;					
	// Restore IRQ
	SREG = sreg;					
}

void ShiftRegister4094_Init(void)
{
	activate_output_bit(SPI_STR_SHIFT_REGISTER);
	set_output_bitval(SPI_STR_SHIFT_REGISTER,0);
}

void ShiftRegister4094_DeInit(void)
{
	deactivate_output_bit(SPI_STR_SHIFT_REGISTER);

}

/* ADA Code
//----------------------------------------------------------------------------
// ShiftOut4094
//
// Shifts out the given value to the four (!) 4094 shift registers
// (only used if no IO32 is present; for compatibilty reasons to Pascal firmware)
//
// -> uValue
//----------------------------------------------------------------------------
void ShiftOut4094(uint32_t uValue)
{
	uint8_t i;
	uint8_t sreg = SREG;

	cli();

	// it might look strange using no loop, but this is the fastest
	// way of shifting out the data
	PORTB &= ~SR4094_STROBE;        // strobe low

	i = (uint8_t) (uValue >> 24);

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x80) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x40) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x20) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x10) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x08) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x04) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x02) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x01) PORTB |= SPI_DATA;
	CLK_HI();

	i = (uint8_t) (uValue >> 16);

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x80) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x40) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x20) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x10) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x08) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x04) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x02) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x01) PORTB |= SPI_DATA;
	CLK_HI();

	i = (uint8_t) (uValue >> 8);

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x80) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x40) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x20) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x10) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x08) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x04) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x02) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x01) PORTB |= SPI_DATA;
	CLK_HI();

	i = (uint8_t) uValue;

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x80) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x40) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x20) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x10) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x08) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x04) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x02) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB &= ~SPI_DATA;
	CLK_LO();
	if(i & 0x01) PORTB |= SPI_DATA;
	CLK_HI();

	PORTB |= SR4094_STROBE;         // strobe high
	nop();
	PORTB &= ~SR4094_STROBE;        // strobe low

	SREG = sreg;
}


*/

