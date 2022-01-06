/*
* Copyright (c) 2012-13 by Paul Schmid
* partially based on work by
* Copyright (c) 2007 by Hartmut Birr, Thoralt Franz, Jörg Wilke
*
* This program is free software; you can redistribute it and/or
* mmodify it under the terms of the GNU General Public License
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
*/

#ifndef MAIN_H_
#define MAIN_H_

#include <inttypes.h>
#include <avr/eeprom.h>
#include "helper_macros.h"





//----------------------------------------------------------------------------
// EEPROM addresses
//----------------------------------------------------------------------------

// one word for validation
//#define EEPROM_VALID    (void*)0
// complete PARAMS structure
#define EEPROM_PARAMS   (void*)0

#define EEPROM_MAGIC_NUMBER 0x5607

// time to wait between EEPROM writes (60000 means 30 seconds)
#define MINIMUM_EEPROM_TIME 60000
//#define EEPROM_WRITE_CACHED() {ATOMIC_RW(xg_uEEPROM_Timer, MINIMUM_EEPROM_TIME); g_ucModifiedParams = 1;}

//----------------------------------------------------------------------------
// global defines and typedefs
//----------------------------------------------------------------------------

#define NORMAL 0
#define REVERSE 1
#define FILENAMEBUFFERSIZE 32

typedef union
{
	uint8_t u8;
	struct
	{
		uint8_t Unused      : 1;
		uint8_t FuseBlown   : 1;
		uint8_t OverVolt    : 1;
		uint8_t OverTemp    : 1;
		uint8_t EEUnlocked  : 1;
		uint8_t CurrentMode : 1;
		uint8_t UserSRQ     : 1;
		uint8_t Busy        : 1;
	};
} STATUS;

typedef union
{
	uint32_t u32;
	uint8_t u8[4];
} U32_4B;

typedef struct
{
	uint8_t idx;
	float val;
} DAC;

typedef struct
{
	uint16_t MagicValue;
	uint8_t ucDisplayedMenu;                    // DSP 0, currently displayed menu on panel
	uint8_t ucEncoderPrescaler;                 // DSP 9  EEP, prescaler value for panel encoder

	uint8_t ucFloatPrecision;                   // OPT 7  EEP, float precision for returning values
	uint16_t uiGETtimeout;                      // OPT 9, timeout for GET 400 .. 409
	
	uint8_t ucHex_Init;                         // OPT 12 EEP, default value for g_ucHex (new parameter)
	char cp_OPT30_AutoLoadFileName[32];         // OPT 30 EEP, file name for auto loading at startup
	// twice ???
	char cp_OPT31_SaveDefaultFileName[32];      // OPT 31 EEP, default file name when saving data with FWR or FWV
	char cp_FNA_SaveFileName[32];               // FNA, file name when saving data with FWR or FWV (default is copied from OPT 31)

	uint8_t ucDefaultChannel;                   // COM 2, 0 is OPTO-Bus, 1 is FPGA COM
	
	uint8_t ucSerBaudReg;                       // SBD, serial baud rate setting

	uint32_t ulValue32[4];                      // for handling the values read back from registers 0..3 (internal only, for panel)
	uint32_t ulParam32[4];                      // for handling the values to write to registers 0..3 (internal only, for panel)
	uint8_t  ucParam8[7];

	// *** ??? ***
	uint8_t ucDummy1;                           // Dummy parameter for future sub menu usage (tbd)
	uint8_t ucDummy2;                           // Dummy parameter for future sub menu usage (tbd)

	uint8_t ucDummy3;                         // auto save time for future sub menu usage (tbd)

	int16_t iScrollSpeed;                       // Scroll speed on panel (internal only)

	// new
	uint8_t	ucInitIntegrateADC;					//
	int16_t ADCoffsets[8];						// Offsets ADC-Wert in ADC-Rohwerten
	float   ADCscales[8];						// Kanal-Skalierung AD-Wert; Messwert = (ADC-Rohwert + Offset) * Kanal-Skalierung * Basis-Skalierung
	float  ADCbaseScale;						// ADC-Basis-Skalierung; möglichste nicht ändern!
	
	// chech DAC type
	DAC		DACValues[4];
	float   DACBaseScale;						// berücksichtigt Grundverstärkung // TODO int??
	int16_t DACOffset[4];						// support also the negative part
	float	DACscale[4];
	
	uint8_t RelayPortArray[4];
	
	uint8_t ucI2CAddress;						// from ADA
}  PARAMS;

//----------------------------------------------------------------------------
// global variables
//----------------------------------------------------------------------------

extern STATUS Status;
extern PARAMS Params;
extern PARAMS *pParams;
extern PARAMS LastParams;
extern PARAMS *pLastParams;

extern uint8_t g_ucScriptMCH;
extern uint8_t g_ucScriptSCH;
extern uint8_t g_ucHex;

extern uint8_t g_ucSlaveCh;
extern uint8_t g_ucErrCount;

extern uint8_t g_ucLCDpresent;

extern float g_dScriptRegister[10];
extern float g_dCompareParam;

extern uint32_t g_ulCurrentINIfilePos;
extern uint32_t g_ulLabelPointer[32];
extern uint8_t g_ucLabelValid[32];

extern uint8_t g_ucSearchLabel;
extern uint8_t g_ucGotoLabel;
extern int32_t g_lIniFileSeekPos;

extern float g_dWaitResponse;

extern uint16_t xg_uiScriptDelay;

extern char g_cSerInpStr[];

extern uint8_t gucBuffer[256];
extern char gucIRQBuffer[256];

extern uint8_t gucIRQrBufferPtr;
extern uint8_t g_ucCoreCommand;

extern uint8_t g_newEEMode;

//----------------------------------------------------------------------------
// global variables (used in ISR)
//----------------------------------------------------------------------------

extern volatile uint8_t g_ucSDpresent;
extern volatile uint8_t g_ucSDdisable;
extern volatile uint8_t g_ucModuleBusy;
extern volatile char    g_cWaitForResponse;
extern volatile uint8_t gucIRQwBufferPtr;



//----------------------------------------------------------------------------
// prototypes
//----------------------------------------------------------------------------

void Params_Init(void);
void MainFunction(void);
void jobExecute(void);
void CheckLimits(void);

void PrintIDNstring(void);

void ActivityTimer_SetValue(uint8_t Value);
void ActivityTimer_MainFunction(void);

void Panel_MainFunction(void);

#endif /*MAIN_H_*/
