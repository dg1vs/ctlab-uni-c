/**
* @file Port.h
* @author dg1vs
* @date 2010/07/09
* @brief Port handler
*
*/

#ifndef _PORT_H_
#define _PORT_H_

#include <avr/io.h>
#include <stdint.h>
#include <inttypes.h>



/*
#define LED        B,5
#define SENSOR     B,4

#define _SET(type,name,bit)      type##name |= _BV(bit)
#define _CLR(type,name,bit)      type##name &= ~_BV(bit)
#define _GET(type,name,bit)      (type##name & _BV(bit))
#define SET_AS_OUTPUT(pin)       _SET(DDR,pin)
#define SET_AS_INPUT(pin)        _CLR(DDR,pin)
#define SET_HIGH(pin)            _SET(PORT,pin)
#define SET_LOW(pin)             _CLR(PORT,pin)
#define TOGGLE(pin)              _SET(PIN,pin)
#define IS_HIGH(pin)             _GET(PIN,pin)
*/

// in stdint.h __CONCAT should be found, why not....
//#define __CONCAT(left, right) left ## right


/* Macro-functions for IO-bits */
#define set_output_bit(NAME) __CONCAT(NAME,_PORT)|=_BV(__CONCAT(NAME,_BIT))
#define clear_output_bit(NAME) __CONCAT(NAME,_PORT)&=~(unsigned char)_BV(__CONCAT(NAME,_BIT))
#define toggle_output_bit(NAME) __CONCAT(NAME,_PORT)^=(unsigned char)_BV(__CONCAT(NAME,_BIT))

// Todo implify and remove Led_Set
#define set_output_bitval(NAME, VAL) __CONCAT(NAME,_PORT)=(__CONCAT(NAME,_PORT)&~(unsigned char)_BV(__CONCAT(NAME,_BIT)))|((VAL)?_BV(__CONCAT(NAME,_BIT)):0)
#define Led_Set(NAME, VAL) __CONCAT(NAME,_PORT)=(__CONCAT(NAME,_PORT)&~(unsigned char)_BV(__CONCAT(NAME,_BIT)))|((VAL)?_BV(__CONCAT(NAME,_BIT)):0)

#define activate_output_bit(NAME) __CONCAT(NAME,_DDR)|=_BV(__CONCAT(NAME,_BIT))
#define deactivate_output_bit(NAME) __CONCAT(NAME,_DDR)&=~(unsigned char)_BV(__CONCAT(NAME,_BIT))

#define get_input_bit(NAME) (__CONCAT(NAME,_PIN)&_BV(__CONCAT(NAME,_BIT)))
#define get_output_bit(NAME) (__CONCAT(NAME,_PORT)&_BV(__CONCAT(NAME,_BIT)))


// TODO this part should code-generated


/* ACTIVITY_LED */
#define ACTIVITY_LED_PORT  PORTD
#define ACTIVITY_LED_BIT   2
#define ACTIVITY_LED_DDR   DDRD
#define ACTIVITY_LED_PIN   PIND


/* PANEL_LED */
#define PANEL_LED_PORT  PORTD
#define PANEL_LED_BIT   3
#define PANEL_LED_DDR   DDRD
#define PANEL_LED_PIN   DDRD


/* SPI-Bits mainly CS-signal and some triggers */
#define SPI_CS_MCP3208_PORT 	PORTC
#define SPI_CS_MCP3208_BIT		7
#define SPI_CS_MCP3208_DDR 		DDRC
#define SPI_CS_MCP3208_PIN		PINC

#define SPI_CS_MCP4822_A_PORT 	PORTC
#define SPI_CS_MCP4822_A_BIT	2
#define SPI_CS_MCP4822_A_DDR 	DDRC
#define SPI_CS_MCP4822_A_PIN	PINC

#define SPI_CS_MCP4822_B_PORT 	PORTC
#define SPI_CS_MCP4822_B_BIT	3
#define SPI_CS_MCP4822_B_DDR 	DDRC
#define SPI_CS_MCP4822_B_PIN	PINC

#define SPI_LDAC_MCP4822_PORT	PORTC
#define SPI_LDAC_MCP4822_BIT	4
#define SPI_LDAC_MCP4822_DDR 	DDRC
#define SPI_LDAC_MCP4822_PIN	PINC

/* Invert */
#define DAC_INVERT_OUT_0_PORT  PORTD
#define DAC_INVERT_OUT_0_BIT   5
#define DAC_INVERT_OUT_0_DDR   DDRD
#define DAC_INVERT_OUT_0_PIN   DDRD

#define DAC_INVERT_OUT_1_PORT  PORTD
#define DAC_INVERT_OUT_1_BIT   4
#define DAC_INVERT_OUT_1_DDR   DDRD
#define DAC_INVERT_OUT_1_PIN   DDRD

#define DAC_INVERT_OUT_2_PORT  PORTD
#define DAC_INVERT_OUT_2_BIT   7
#define DAC_INVERT_OUT_2_DDR   DDRD
#define DAC_INVERT_OUT_2_PIN   DDRD

#define DAC_INVERT_OUT_3_PORT  PORTD
#define DAC_INVERT_OUT_3_BIT   6
#define DAC_INVERT_OUT_3_DDR   DDRD
#define DAC_INVERT_OUT_3_PIN   DDRD

/* SPI-Bits 4094 shift register CS-signal  and  */
#define SPI_STR_SHIFT_REGISTER_PORT PORTC
#define SPI_STR_SHIFT_REGISTER_BIT	5
#define SPI_STR_SHIFT_REGISTER_DDR 	DDRC
#define SPI_STR_SHIFT_REGISTER_PIN	PINC

/* SPI-Bits 4094 shift register OE-signal  */
#define SPI_OE_SHIFT_REGISTER_PORT 	PORTC
#define SPI_OE_SHIFT_REGISTER_BIT	6
#define SPI_OE_SHIFT_REGISTER_DDR 	DDRC
#define SPI_OE_SHIFT_REGISTER_PIN	PINC

#ifdef __cplusplus
extern "C" {
#endif

void IOPins_Init(void);
void IOPins_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _PORT_H_ */
