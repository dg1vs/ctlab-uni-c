/*
* Copyright (c) 2007 by Thoralt Franz
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
#ifndef _PANEL_H_
#define _PANEL_H_

#include "Lcd.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// menu structure
//----------------------------------------------------------------------------
typedef struct
{
	// pointer to function which is called after encoder is turned
	void (*pEncoderFunction)(int16_t iEncoderValue, void *pParam);
	// pointer to function which is called after encoder is pushed
	void (*pEnterFunction)(void);
	// pointer to function which is called to update the parameter display
	void (*pUpdateDisplayFunction)(uint8_t ucForceDisplay, void *pParam);
	// menupointers to submenu, previous or next menuitems
	void *pSubmenu, *pPrevious, *pNext;
	// encoder acceleration values for this parameter
	int16_t iEI1, iEI2, iEI3, iEI4;
	// additional pointer parameter to pass to the encoder function
	void *pParam;
	// Menu identifier for DSP command
	uint8_t ucIdentifier;
	// name to show on LCD
	char cName[];
} menu_entry_t;

//----------------------------------------------------------------------------
// DEFINEs
//----------------------------------------------------------------------------
#define null (void*)0

// time to avoid button bounce (value of 250 = 125 ms)
#define BUTTON_TIME 250
//----------------------------------------------------------------------------
// Menu identifiers
//----------------------------------------------------------------------------

#define MENU_FILESELECT  8

#define MENU_VALUE3      3
#define MENU_VALUE2      2
#define MENU_VALUE1      1
#define MENU_VALUE0      0

#define MENU_PARAM3      7
#define MENU_PARAM2      6
#define MENU_PARAM1      5
#define MENU_PARAM0      4

#define MENU_SETTINGS   30

#define SUBMENU_SAVE            33
#define SUBMENU_PERMANENTTIME   34
#define SUBMENU_SCROLLSPEED     35
#define SUBMENU_SETTINGSBACK    36
#define SUBMENU_TOGGLESTRING    37
#define SUBMENU_HEXBINARY       38


#define ADC_MENU_VALUE0			10
#define ADC_MENU_VALUE1			11
#define ADC_MENU_VALUE2			12
#define ADC_MENU_VALUE3			13
#define ADC_MENU_VALUE4			14
#define ADC_MENU_VALUE5			15
#define ADC_MENU_VALUE6			16
#define ADC_MENU_VALUE7			17

#define DAC_MENU_VALUE0			20
#define DAC_MENU_VALUE1			21
#define DAC_MENU_VALUE2			22
#define DAC_MENU_VALUE3			23



// time until menu mode is switched from encoder mode back to normal
// (in 0.5 ms units)
#define ENCODER_MENU_TIME   10000

//----------------------------------------------------------------------------
// Display defines
//----------------------------------------------------------------------------

// for pParams->ucDisplayToggleTitle
#define STRINGAUTO      0
#define STRINGDEFAULT   1
#define STRINGTOGGLE    2

// for pParams->ucDisplayHexBinary
#define HEXVALUE        0
#define BINARYVALUE     1

// for g_ucValueEntryMode
#define SHOWVALUE       0
#define CHANGEVALUE     1
#define SHOWINOUT       2



#ifdef COMPILE_WITH_DISPLAY204              /* PM 20*4 */
#define CURSOR_ARRAY_SIZE 8*8
#else                                       /* PM8 */
#define CURSOR_ARRAY_SIZE 4*8
#endif

extern const PROGMEM uint8_t cursor[CURSOR_ARRAY_SIZE];

#ifdef COMPILE_WITH_DISPLAY204               /* PM 20*4 */
#define EXTRA_EA_DIP_SIGN_TRMSC  "\x01"
#define EXTRA_EA_DIP_SIGN_ENTER  "\x10"
#define EXTRA_EA_DIP_SIGN_BACK   "\x16"

#define EXTRA_EA_DIP_SIGN_MU     "\x8F"
#define EXTRA_EA_DIP_SIGN_OFFSET "\x2E"

#define EXTRA_EA_DIP_SIGN_DOWN   "\x1B"
#define EXTRA_EA_DIP_SIGN_UP     "\x1A"

#define CHAR_EMPTYBOX      0
#define CHAR_SOLIDBOX    255

#else                                        /* PM8 */
#define EXTRA_EA_DIP_SIGN_EMPTYBOX  "\x00"
#define EXTRA_EA_DIP_SIGN_HATCHED   "\x01"
#define EXTRA_EA_DIP_SIGN_ENTER     "\x02"
#define EXTRA_EA_DIP_SIGN_BACK      "\x02"
#define EXTRA_EA_DIP_SIGN_ENTRY     "\x03"

#define CHAR_HATCHEDBOX    1
#define CHAR_SOLIDBOX    255

#endif


//----------------------------------------------------------------------------
// prototypes for menu actions and menu items
//----------------------------------------------------------------------------


// FileEncoderFunction
void FileEncoderFunction(int16_t iEncoderPos, void *pParam);
void FileEnterFunction(void);
void FileUpdateDisplayFunction(uint8_t ucForceUpdate , void *pParam);
// ADC
void Panel_AdcValueUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
//DAC
void Panel_DacValueUpdateEncoderFunction(int16_t iEncoderPos, void *pParam);
void Panel_DacValueUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
void Panel_DacValueUpdateEnterFunction(void);
// Value
void ValueEncoderFunction(int16_t iEncoderPos, void *pParam);
void ValueUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
void ValueEnterFunction(void);
// Param
void ParamEncoderFunction(int16_t iEncoderPos, void *pParam);
void ParamUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
void ParamEnterFunction(void);


void SettingsToggleStringUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
void SettingsHexBinaryUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);

void LevelUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
void LevelEncoderFunction(int16_t iEncoderPos, void *pParam);

void IEncoderFunction(int16_t iEncoderPos, void *pParam);
void UCEncoderFunction(int16_t iEncoderPos, void *pParam);

void SettingsSaveUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam);
void SettingsSaveEnterFunction(void);

extern const PROGMEM menu_entry_t MenuFileSelect, MenuSettings,

	AdcMenuValue7, AdcMenuValue6, AdcMenuValue5,	AdcMenuValue4, AdcMenuValue3, AdcMenuValue2, AdcMenuValue1,	AdcMenuValue0,

	DacMenuValue3, DacMenuValue2, DacMenuValue1, DacMenuValue0,

	MenuValue3, MenuValue2, MenuValue1, MenuValue0,
	MenuParam3, MenuParam2, MenuParam1, MenuParam0,

	SubmenuSettingsSave, SubmenuSettingsPermanentTime,
	SubmenuSettingsScrollSpeed, SubmenuSettingsBack,
	SubmenuSettingsToggleString, SubmenuSettingsHexBinary
	;

extern menu_entry_t *g_currentMenuItem;
extern uint8_t g_ucMenuItemChanged;

extern struct fat_dir_entry_struct g_dir_entry_panel;
extern uint16_t g_uiFilePtr;
extern uint8_t g_ucFileMenuUpdate;

//----------------------------------------------------------------------------
// prototypes
//----------------------------------------------------------------------------
void Panel_MainFunction(void);
//void Panel_OutputValue(float f, int16_t i, uint8_t uType);
menu_entry_t* findMenuEntry(uint8_t ucIdentifier);

void Panel_SplashScreen(void);

#ifdef __cplusplus
}
#endif

#endif /* _PANEL_H_*/
