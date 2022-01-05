/*
 * Uni_C_hw.c
 *
 * Created: 31.03.2014 21:37:01
 *  Author: ks
 */ 
 #include <math.h>

#include "unic_hw.h"
#include "Mcp4822.h"
#include "Mcp3208.h"
#include "ShiftRegister4094.h"
#include "IOPins.h"
#include "debug.h"
#include "main.h"

void RelayPort_Init(void)
{
	ShiftRegister4094_Init();
	/* Docu_HW_Init */
	activate_output_bit(SPI_OE_SHIFT_REGISTER);
	set_output_bitval(SPI_OE_SHIFT_REGISTER,0);
	// TODO Restore setting from e2prom
	ShiftRegister4094_ShiftOut();
	// the activate output
	set_output_bitval(SPI_OE_SHIFT_REGISTER,1);
	// TODO write default value
}

void RelayPort_DeInit(void)
{
	/* Docu_HW_Init */
	set_output_bitval(SPI_OE_SHIFT_REGISTER,0);
	deactivate_output_bit(SPI_OE_SHIFT_REGISTER);
	ShiftRegister4094_DeInit();
}

void RelayPort_Set(uint8_t port, uint8_t val)
{
	//// TODO check port < 4
	//// This is not very nice, from the architectural standpoint
	//// TODO remove the Rarams.xxx handling to the unic_Parser stuff
	Params.RelayPortArray[port] = val;
	ShiftRegister4094_ShiftOut();
}



void Dac_Init()
{
	Mcp4822_Init();
	/* Docu_HW_Init */
	activate_output_bit(DAC_INVERT_OUT_0);
	activate_output_bit(DAC_INVERT_OUT_1);
	activate_output_bit(DAC_INVERT_OUT_2);
	activate_output_bit(DAC_INVERT_OUT_3);
	for (uint8_t i=0; i < 4; i++)
	{
		Dac_Output(i, pParams->DACValues[i].val);
	}
}

void Dac_DeInit()
{
	/* Docu_HW_Init */
	deactivate_output_bit(DAC_INVERT_OUT_3);
	deactivate_output_bit(DAC_INVERT_OUT_2);
	deactivate_output_bit(DAC_INVERT_OUT_1);
	deactivate_output_bit(DAC_INVERT_OUT_0);
	Mcp4822_DeInit();
}

	
void Dac_Output(uint8_t dacChannel, float val)
{
	
	int8_t instanz;
	uint8_t channel; 
	uint8_t gain = 0; 
	uint8_t outputenable = 1;
	uint16_t data;
	uint8_t offset=0;
	// uint8_t test;
	bool s=0;
	static bool lastSign=0;
	
	instanz = dacChannel / 2;
	channel  = dacChannel % 2;


	/**
    The signbit() function returns a nonzero value if the value of \a __x
    has its sign bit set.  This is not the same as `\a __x < 0.0',
    because IEEE 754 floating point allows zero to be signed. The
    comparison `-0.0 < 0.0' is false, but `signbit (-0.0)' will return a
    nonzero value.
	*/
	s = signbit(val); // negative sign the 4 BS 170
	if (s) 
	{
		val = -1.f * val;	// change sign of the vale
		offset = 4;			// Scale and offset for negative output voltage
	}

	// InitDACBaseScale means 10.0 V <===>  3710
	// just keep the old and huge vale for InitDACBaseScale
	data = (uint16_t) ( (100000000/(pParams->DACBaseScale) * val * pParams->DACscale[dacChannel+offset]) + pParams->DACOffset[dacChannel+offset]) ;

	
	// Nulldurchgang? Dann vor dem Polaritätswechsel Nullwert ausgeben
	if (lastSign != s)
	{
		Mcp4822_Transmit(instanz, channel, gain, outputenable, 0);
	}
	lastSign = s;	
	
	// now the real value
	// instanz, channel, gain, output_enable, data);
	Mcp4822_Transmit(instanz, channel, gain, outputenable, data);	

	switch(dacChannel)
	{
		case 0:
			set_output_bitval(DAC_INVERT_OUT_0,s);  
		    break;
		case 1:
			 set_output_bitval(DAC_INVERT_OUT_1,s);   
			break;
		case 2:
			set_output_bitval(DAC_INVERT_OUT_2,s);    
			break;
		case 3:
			set_output_bitval(DAC_INVERT_OUT_3,s);  
			break;
		// TODO	-> deafult oder ensure 0..3
		}
	return;
}


void Adc_Init()
{
	/* Docu_HW_Init */
	activate_output_bit(SPI_CS_MCP3208);
	Mcp3208_Init();
}

void Adc_DeInit()
{
	Mcp3208_DeInit();
	/* Docu_HW_Init */
	deactivate_output_bit(SPI_CS_MCP3208);
}

float Adc_Read(uint8_t ch)
{
	// TODO KS add check ch 0..7
	 
	uint16_t tempadc; 

	//Achtung: kehrt Reihenfolge 0..7 um wg. Layout
	ch = 7 - ch;
	
	if (0) //pParams->ucInitIntegrateADC)
	{
		tempadc = Mcp3208_StartConvReadResult(ch);	
		tempadc = tempadc + Mcp3208_StartConvReadResult(ch);	
		tempadc = tempadc + Mcp3208_StartConvReadResult(ch);	
		tempadc = tempadc + Mcp3208_StartConvReadResult(ch);	
		tempadc = tempadc/4;
	}
	else
	{
		tempadc = Mcp3208_StartConvReadResult(ch);	
	}

#ifdef JTAG_DEBUG // return some usefulls values in case of attached jtag-debugger
	return (1.2342f + (float) ch);    
#else
	return  (tempadc + pParams->ADCoffsets[ch]) * pParams->ADCbaseScale* pParams->ADCscales[ch]; //Grundskalierung 12 Bit	
#endif	

}

uint16_t Adc_RawRead(uint8_t ch)
{
	//Achtung: kehrt Reihenfolge 0..7 um wg. Layout
	ch = 7 - ch;
	
#ifdef JTAG_DEBUG // return some usefulls values in case of attached jtag-debugger
	return (100 * ch);
#else
	return Mcp3208_StartConvReadResult(ch);
#endif

}



