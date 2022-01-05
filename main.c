/*
* Copyright (c) 2012-13 by Paul Schmid
* partially based on work by
* Copyright (c) 2007 by Thoralt Franz, Joerg Wilke, Hartmut Birr
*
* SD/MMC routines are Copyright (c) 2006-2012 by Roland Riegel (GPL v2)
*
* Project inspired by
* "c't-Lab by Carsten Meyer, c't magazin"
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

//============================================================================
// IO MAP
//
// See IO_Init Init_IO
//
//============================================================================


#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <util/delay.h>
#include <math.h>
#include <string.h>

#include "main.h"
#include "debug.h"

#include "fat.h"
#include "fat_config.h"
#include "partition.h"
#include "sd_raw.h"
#include "sd_raw_config.h"
#include "sd_commands.h"

#include "panel.h"
#include "fpga.h"
#include "fpga_hw_config.h"
#include "unic_hw.h"
#include "helper_macros.h"
#include "helper.h"
#include "parser.h"

#include "Uart.h"
#include "Timer.h"
#include "Rtc.h"
#include "I2CRegister.h"


//----------------------------------------------------------------------------
// global variables
//----------------------------------------------------------------------------

STATUS Status;

PARAMS Params;
PARAMS *pParams = &Params;

PARAMS LastParams;
PARAMS *pLastParams = &LastParams;

uint8_t g_ucScriptMCH;
uint8_t g_ucScriptSCH;
uint8_t g_ucHex;

uint8_t g_ucSlaveCh;
uint8_t g_ucErrCount;
uint8_t ActivityTimer;
uint8_t g_ucLCDpresent;

float g_dScriptRegister[10];
float g_dCompareParam = 0.0f;

uint32_t g_ulCurrentINIfilePos;
uint32_t g_ulLabelPointer[32];
uint8_t g_ucLabelValid[32];

uint8_t g_ucSearchLabel = FALSE;
uint8_t g_ucGotoLabel = 0;
int32_t g_lIniFileSeekPos = 0;

const PROGMEM char g_cVersStrLong_P[] = VERSSTRLONG;
const PROGMEM char g_cVersStrShort_P[] = VERSSTRSHORT;

//uint8_t g_ucModifiedParams = 0;

float g_dWaitResponse = 0.0;

uint16_t xg_uiScriptDelay = 0;

uint8_t gucBuffer[256];             // General Purpose buffer for FPGA transfer, must be multiple of 4(!)
char gucIRQBuffer[256];             // Circular Buffer for characters read from FPGA in the interrupt

uint8_t gucIRQrBufferPtr = 0;

uint8_t g_ucCoreCommand = FALSE;
uint8_t g_newEEMode = FALSE;
//----------------------------------------------------------------------------
// global variables (used in ISR)
//----------------------------------------------------------------------------

volatile uint8_t g_ucSDpresent = 0;
volatile uint8_t g_ucSDdisable = 0;
volatile uint8_t g_ucModuleBusy = 0;    // Semaphore: During handling of time-consuming tasks (e.g. SD card access) opto bus commands are rejected. However, opto bus commands/responses for other modules are forwarded.
// Note: the semaphore blocks only opto bus commands. I.e during the execution of an SD-card script another SD command (like loading the FPGA) is allowed.
volatile char g_cWaitForResponse = FALSE;
volatile uint8_t gucIRQwBufferPtr = 0;

//----------------------------------------------------------------------------
// uart_putchar
//
// write one character to the UART, wait if UART buffer is full
// '\n' is replaced by '\r'+'\n'
//
// -> c = character to send
//    stream = open file (unused, only for compatibility with I/O streams
//             and printf)
// <- 0
//----------------------------------------------------------------------------
int uart_putchar(char c, FILE* stream __attribute__((unused)))
{
	if (c == '\n')
	{
		c = '\r';
		while (1 != Uart_SetTxData((uint8_t*)&c, 1, 0));
		c = '\n';
	}
	while(1 != Uart_SetTxData((uint8_t*)&c, 1, 0));
	return 0;
}

// file pointer for UART output (used by printf)
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


void IO_Init(void)
{
	// PA0 ADC0 (panel/encoder)		ok
	// PA1 ADC1 (panel/encoder)		ok
	// PA2 /RESET DS1302			new
	// PA3 SD_CDSW2 SD_CardSwitch   new
	// PA4 FPGA_DIN					new
	// PA5 FPGA_CCLK				new
	// PA6 FPGA_PROG				new
	// PA7 FPGA_DONE				new
	/* Docu_HW_Init */	
	DDRA  = 0b01110100;     
	PORTA = 0b00001011;     // CDSW, RTC-Reset, IncrGeber

	// PB0 
	// PB1 Input Frequenz / Input FPGA
	// PB2 
	// PB3 
	// PB4 SD_CS					ok
	// PB5 FPGA/SD_MOSI				ok - FPGA?
	// PB6 FPGA/SD_MISO				ok - FPGA?
	// PB7 FPGA/SD_SCK				ok - FPGA?
	/* Docu_HW_Init */
	DDRB  = 0b10011001;     
    PORTB = 0b10111101;     // SPI, Trigger und FrequCount

	// PC0 I2C_SCL					ok
	// PC1 I2C_SDA					ok
	// PC2 MCP 4822	/CS	0-1			new
	// PC3 MCP 4822	/CS	2-3			changed SD_CardSwitch
	// PC4 MCP 4822 /LDAC			changed FPGA_DIN
	// PC5 4094 Relais G			changed FPGA_CCLK
	// PC6 4094 Relais OE			changed FPGA_PROG
	// PC7 MCP 3208 /CS				changed FPGA_DONE	
	/* Docu_HW_Init */
	DDRC  = 0b11111100;     
    PORTC = 0b10011100;     // Strobes DAC und SR, I2C

	// PD0 RxD						ok
	// PD1 TxD						ok
	// PD2 LED_Activity				ok
	// PD3 LED_Panel				ok
	// PD4 Polarity DE/A ch 1		new
	// PD5 Polarity DE/A ch 0		new
	// PD6 Polarity DE/A ch 3		new
	// PD7 Polarity DE/A ch 2		new
	/* Docu_HW_Init */
	DDRD  = 0b11111100;     
	PORTD = 0b00001100;     // Enables/Invert-Bits, LED
	
}

void ActivityTimer_SetValue(uint8_t Value)
{
	if (Value > ActivityTimer)
	{
		ActivityTimer = Value;
		SetLED(ACTIVITY_LED, 1);             // Activity LED on
	}
}

void ActivityTimer_MainFunction(void)
{
	if (ActivityTimer)
	{
		ActivityTimer--;
		if (ActivityTimer == 0)
		{
			SetLED(ACTIVITY_LED,0);          // Activity LED off
		}
	}
}




/**
 * 
 * Checks all members of Params
 *
 *  This function checks all members of Params against their limits and
 *  corrects them if necessary.
 * 
 */
void CheckLimits(void)
{
	LIMIT_UINT8(&Params.ucEncoderPrescaler, 1, 32);
	LIMIT_UINT8(&Params.ucDefaultChannel, 0, 1);
	// sprintf %f or %g can take no more than 6 precision digits.
	if (Params.ucFloatPrecision > 6)
	{
		Params.ucFloatPrecision = 255;
	}
}

void PrintIDNstring(void)
{
	printf_P(PSTR("#%d:254=%S\n"), g_ucSlaveCh, (char*)g_cVersStrLong_P );
}

int __attribute__((OS_main)) main(void)
{
	uint8_t i;

	pParams = &Params;

	// set up IO lines
	IO_Init();
	Adc_Init();
	Dac_Init();
	RelayPort_Init();
	// initialize timer
	Timer0_Init();
	Timer2_Init();

	// TODO KS make card change great again
	//PinChangeInterruptPA3_Init();

	// enable interrupts
	sei();

	// wait for capacitors at MAX232 to charge
	wait_ms(20);

	// fix value for uni-c
	g_ucSlaveCh = 2;
	g_ucErrCount = 0;

	I2C_Init();
	g_ucLCDpresent = 0;
	if(Lcd_Init())
	{
		g_ucLCDpresent = 1;
		Panel_SplashScreen();
	}

	SetLED(ACTIVITY_LED, 1);    // Activity LED on
	wait_ms(1000);
	SetLED(ACTIVITY_LED, 0);    // Activity LED off
	wait_ms(500);

	// Display address of card with blinking LED
	for (i = 0; i < g_ucSlaveCh; i++)
	{
		SetLED(ACTIVITY_LED, 1);    // Activity LED on
		wait_ms(150);
		SetLED(ACTIVITY_LED, 0);    // Activity LED off
		wait_ms(150);
	}
	Lcd_OverWrite_P(0, 0, 8, PSTR( VERSSTRADD ));

	// initialize parameters, 
	Params_Init();
	
	// The first value in the PARAM Struct contain the EEPROM Version
	if(EEPROM_MAGIC_NUMBER == eeprom_read_word(EEPROM_PARAMS))
	//load from EEPROM if it contains valid data
	{
		eeprom_read_block(&Params, EEPROM_PARAMS, sizeof(Params));
	}
	else
	// write default data to EEPROM if not
	{
		LOG_INFO(PSTR("Init EEP\n"));  
		Lcd_OverWrite_P(0, 1, 8, PSTR("Init EEP"));
		eeprom_write_block(&Params, EEPROM_PARAMS, sizeof(Params));
		eeprom_write_word(EEPROM_PARAMS, EEPROM_MAGIC_NUMBER);
	}

	g_ucHex = Params.ucHex_Init;
	strcpy(Params.cp_FNA_SaveFileName, Params.cp_OPT31_SaveDefaultFileName);

	Uart_InitUBRR(pParams->ucSerBaudReg);
	stdout = &mystdout; // set stream pointer (for printf)

	PrintIDNstring();
	
	Rtc_CheckChipPresent();
	Rtc_ReadChipTime();

	SD_IsCardPresent();

	if (g_ucSDpresent)
	{
		// if running on an 1248p device the fpga-image for the small device 50 k can be located in the avr-flash
		FPGA_LoadImageFromProgmem();
		
		LOG_INFO(PSTR("Loading default: %s\n"), pParams->cp_OPT30_AutoLoadFileName);
		g_ucModuleBusy++;
		CFG_LoadFile(pParams->cp_OPT30_AutoLoadFileName, BUSMODE);
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	}

	jobExecute();

	while(1)
	{
		MainFunction();
	}

	// just in case :-)
	RelayPort_DeInit();
	Dac_DeInit();
	Adc_DeInit();
}


void Params_Init()
{
	uint8_t i;

	memset(&Params, 0x0, sizeof(PARAMS));
	pParams->MagicValue	= EEPROM_MAGIC_NUMBER;
	pParams->ucDisplayedMenu        = MENU_FILESELECT;  // Current Menu on Panel (for DSP-Command)
	pParams->ucEncoderPrescaler     = 4;
	// ADC
	pParams->ucInitIntegrateADC		= 0;
	for (uint8_t i=0; i < 8; i++)
	{
		pParams->ADCoffsets[i] = 0;						// Offsets ADC-Wert in ADC-Rohwerten
		pParams->ADCscales[i] = 1.0F;					// Kanal-Skalierung AD-Wert; Messwert = (ADC-Rohwert + Offset) * Kanal-Skalierung * Basis-Skalierung
	}
	pParams->ADCbaseScale = 1/409.5;					// ADC-Basis-Skalierung; m�glichste nicht �ndern!

	//DAC
	for (uint8_t i=0; i < 4; i++)
	{
		pParams->DACOffset[i] = 0;						// Offsets DAC-Werte in DAC-Rohwerten
		pParams->DACscale[i] =1.0; //1.0F;				// Kanal-Skalierungen DAC-Werte; Ausgabewert = (Kanal-Skalierung * Spannung / Basis-Skalierung) + Kanal-Offset
	}
	pParams->DACBaseScale = 269541;						// DAC-Basis-Skalierung; m�glichste nicht �ndern!
	// DacValues
	for (uint8_t i=0; i < 4; i++)
	{
		pParams->DACValues[i].val = (float)i + 0.5;
		pParams->DACValues[i].idx = i;
	}
	
	pParams->ucDefaultChannel = 0;                      // COM 2, 0 is OPTO-Bus, 1 is FPGA COM

	// check
	strcpy_P(pParams->cp_OPT30_AutoLoadFileName,	PSTR("basic.ini"));             // OPT 30
	strcpy_P(pParams->cp_OPT31_SaveDefaultFileName, PSTR("DATAFILE.TXT"));          // OPT 31
	strcpy(  pParams->cp_FNA_SaveFileName, pParams->cp_OPT31_SaveDefaultFileName);  // FNA

	pParams->ucHex_Init = 0;

	pParams->ucFloatPrecision = 4;      
	pParams->uiGETtimeout = 500;            // 500 ms

	pParams->ucDummy1             = 0;
	pParams->ucDummy2             = 0;

	pParams->iScrollSpeed           = 500;
	pParams->ucDummy3				= 23;
	
	pParams->ucSerBaudReg           = INIT_UBRR % 256;
	pParams->ucI2CAddress           = 0x48;     // LM75 -> 72 dec

	pParams->RelayPortArray[0] = 0xB;
	pParams->RelayPortArray[1] = 0xE;
	pParams->RelayPortArray[2] = 0xE;
	pParams->RelayPortArray[3] = 0xF;
	
	// set parameters at once
	memset(&LastParams, 0xFF, sizeof(PARAMS));

	// preset RTC
	gRTC.ucSeconds = 59;
	gRTC.ucMinutes = 59;
	gRTC.ucHours   = 59;
	gRTC.ucDay     = 31;
	gRTC.ucMonth   = 12;
	gRTC.ucYear    = 99;
	gRTC.uiYearWord = 1999;

	for (i=0; i<10; i++)
	{
		g_dScriptRegister[i]= 0.0;          // Initialize Registers with 0.0
	}
	for (i=0; i<8; i++)
	{
		pParams->ucParam8[i] = i;
	}
	for (i=0; i<4; i++)
	{
		pParams->ulValue32[i] = 0x00000000UL;                   // Default
		pParams->ulParam32[i] = 0x00000000UL;
	}
};

void MainFunction()
{
	if (g_ucLCDpresent)
	{
		Panel_MainFunction();
	}

	jobParseData();
	jobExecute();
	SD_IsCardPresent();

	ActivityTimer_MainFunction();
}

void jobExecute()
{
	void (*pUpdateDisplayFunct)(uint8_t, void*);

	// one global function to process all limits
	CheckLimits();

	if (LastParams.ucDisplayedMenu != pParams->ucDisplayedMenu)
	{
		if (findMenuEntry(pParams->ucDisplayedMenu) != g_currentMenuItem)
		{
			g_currentMenuItem = findMenuEntry(pParams->ucDisplayedMenu);
			g_ucMenuItemChanged = 1;
		};
	};
	// Update Display in case of Changes
	pUpdateDisplayFunct = (void*) pgm_read_word(&g_currentMenuItem->pUpdateDisplayFunction);

	if ((pUpdateDisplayFunct) && (g_ucLCDpresent))
	{
		pUpdateDisplayFunct(0, (void*) pgm_read_word(&g_currentMenuItem->pParam));
	};

}

