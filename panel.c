/*
 * Copyright (c) 2011 by Paul Schmid
 *
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

#include <avr/pgmspace.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <util/delay.h>

#include "panel.h"
#include "Encoder.h"
#include "Timer.h"

#include "fat.h"
#include "parser.h"
#include "sd_commands.h"
#include "fpga.h"
#include "unic_hw.h"

#include "helper_macros.h"
#include "debug.h"
#ifdef COMPILE_WITH_DISPLAY204

// can be slow due 20 spaces
#define PANEL_PRINT_CENTER(x)            Lcd_OverWrite_P((COLUMN_MAX - sizeof(x))/2+1, 1, sizeof(x)-1, PSTR((x)));

// LCDOverwrite_P operates in this case over 20 char, this is too much and it will erase the first part of the display line
// so we use the combination of Panel_ClearDataLine, which clears only the second part (all information are right aligned
// and LCDWrite_P and some calculation
#define PANEL_PRINT_CHOICE(x)            Panel_ClearDataLine(); \
                                         LCDWrite_P(COLUMN_MAX + 1 - sizeof(x), 2, sizeof(x)-1, PSTR((x)));

#define PANEL_PRINT_CHOICE_WITH_ENTER(x) Panel_ClearDataLine(); \
                                         LCDWrite_P(COLUMN_MAX + 1 - sizeof(x), 2, sizeof(x)-1, PSTR((x))); Panel_PrintEnterChoice();

#define PANEL_PRINT_FIRMWARE_UPDATE(x)

#else

#define PANEL_PRINT_CENTER(x)            Lcd_OverWrite_P((COLUMN_MAX - sizeof(x))/2+1, 0, sizeof(x)-1, PSTR((x)));
#define PANEL_PRINT_CHOICE(x)            Lcd_OverWrite_P(COLUMN_MAX + 1 - sizeof(x), 1, sizeof(x)-1, PSTR((x)));
#define PANEL_PRINT_CHOICE_WITH_ENTER(x) Lcd_OverWrite_P(COLUMN_MAX     - sizeof(x), 1, sizeof(x)-1, PSTR((x))); Panel_PrintEnterChoice();

#define PANEL_PRINT_FIRMWARE_UPDATE(x)   Lcd_OverWrite_P(0, 0, 8, PSTR("FIRMWARE")); Lcd_OverWrite_P(0, 1, 8, PSTR(" UPDATE "));

#endif

// new functions
static void Panel_PrintBack(void);
static void Panel_PrintEnterChoice(void);
static void Panel_PrintEnter(void);
static void Panel_ResetEnter(void);
static void Panel_ClearDataLine(void);
static void Panel_ClearMenuLine(void);

static void Panel_UpdateStatusLine(void);
static void Panel_ScrollMenuName(menu_entry_t *g_currentMenuItem);


const PROGMEM uint8_t cursor[CURSOR_ARRAY_SIZE] =
{
    #ifdef COMPILE_WITH_DISPLAY204
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0 terminated string
    0x00, 0x08, 0x15, 0x02, 0x08, 0x15, 0x02, 0x00,   // 1 TRMSC symbol EXTRA_EA_DIP_SIGN_TRMSC
    0x10, 0x18, 0x1c, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f,   // 2 D oben   + S rechts oben
    0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1c, 0x18, 0x10,   // 3 D unten  + S rechts unten
    0x01, 0x03, 0x07, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f,   // 4 S links oben
    0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x07, 0x03, 0x01,   // 5 S links unten
    0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00,   // 6 S mitte
    0x00, 0x00, 0x00, 0x10, 0x18, 0x1c, 0x1e, 0x1f,   // 7 S mitte
    #else
    0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x00,   // EXTRA_EA_DIP_SIGN_EMPTYBOX
    0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a,   // EXTRA_EA_DIP_SIGN_HATCHED
    0x00, 0x04, 0x06, 0x07, 0x06, 0x04, 0x00, 0x00,   // EXTRA_EA_DIP_SIGN_ENTER
    0x01, 0x03, 0x05, 0x09, 0x05, 0x03, 0x01, 0x00,   // EXTRA_EA_DIP_SIGN_ENTRY
    #endif
};

//............................................................................
// main menu
//............................................................................
//const PROGMEM menu_entry_t MenuFileSelect =
//{
    //.cName = "FileSel.",
    //.pEncoderFunction = FileEncoderFunction,
    //.pEnterFunction = FileEnterFunction,
    //.pUpdateDisplayFunction = FileUpdateDisplayFunction,
    //.pSubmenu = null,
    //.pPrevious = (void*)&MenuSettings,
    //.pNext = (void*)&AdcMenuValue7,
    //.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    //.pParam = null,
    //.ucIdentifier = MENU_FILESELECT
//};

const PROGMEM menu_entry_t AdcMenuValue7 =
{
	.cName = "ADC[7] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&MenuSettings,
	.pNext = (void*)&AdcMenuValue6,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[7]),
	.ucIdentifier = ADC_MENU_VALUE7
};

const PROGMEM menu_entry_t AdcMenuValue6 =
{
	.cName = "ADC[6] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue7,
	.pNext = (void*)&AdcMenuValue5,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[6]),
	.ucIdentifier = ADC_MENU_VALUE6
};

const PROGMEM menu_entry_t AdcMenuValue5 =
{
	.cName = "ADC[5] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue6,
	.pNext = (void*)&AdcMenuValue4,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[5]),
	.ucIdentifier = ADC_MENU_VALUE5
};

const PROGMEM menu_entry_t AdcMenuValue4 =
{
	.cName = "ADC[4] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue5,
	.pNext = (void*)&AdcMenuValue3,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[4]),
	.ucIdentifier = ADC_MENU_VALUE4
};

const PROGMEM menu_entry_t AdcMenuValue3 =
{
	.cName = "ADC[3] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue4,
	.pNext = (void*)&AdcMenuValue2,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[3]),
	.ucIdentifier = ADC_MENU_VALUE3
};

const PROGMEM menu_entry_t AdcMenuValue2 =
{
	.cName = "ADC[2] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue3,
	.pNext = (void*)&AdcMenuValue1,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[2]),
	.ucIdentifier = ADC_MENU_VALUE2
};

const PROGMEM menu_entry_t AdcMenuValue1 =
{
	.cName = "ADC[1] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue2,
	.pNext = (void*)&AdcMenuValue0,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[1]),
	.ucIdentifier = ADC_MENU_VALUE1
};

const PROGMEM menu_entry_t AdcMenuValue0 =
{
	.cName = "ADC[0] =",
	.pEncoderFunction = null,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_AdcValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue1,
	.pNext = (void*)&DacMenuValue3,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.ucParam8[0]),
	.ucIdentifier = ADC_MENU_VALUE0
};

const PROGMEM menu_entry_t DacMenuValue3 =
{
	.cName = "DAC[3] =",
	.pEncoderFunction = Panel_DacValueUpdateEncoderFunction,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_DacValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&AdcMenuValue0,
	.pNext = (void*)&DacMenuValue2,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.DACValues[3]),
	.ucIdentifier = DAC_MENU_VALUE3
};

const PROGMEM menu_entry_t DacMenuValue2 =
{
	.cName = "DAC[2] =",
	.pEncoderFunction = Panel_DacValueUpdateEncoderFunction,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_DacValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&DacMenuValue3,
	.pNext = (void*)&DacMenuValue1,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.DACValues[2]),
	.ucIdentifier = DAC_MENU_VALUE2
};

const PROGMEM menu_entry_t DacMenuValue1 =
{
	.cName = "DAC[1] =",
	.pEncoderFunction = Panel_DacValueUpdateEncoderFunction,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_DacValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&DacMenuValue2,
	.pNext = (void*)&DacMenuValue0,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.DACValues[1]),
	.ucIdentifier = DAC_MENU_VALUE1
};

const PROGMEM menu_entry_t DacMenuValue0 =
{
	.cName = "DAC[0] =",
	.pEncoderFunction = Panel_DacValueUpdateEncoderFunction,
	.pEnterFunction = null,
	.pUpdateDisplayFunction = Panel_DacValueUpdateDisplayFunction,
	.pSubmenu = null,
	.pPrevious = (void*)&DacMenuValue1,
	.pNext = (void*)&MenuValue3,
	.iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
	.pParam = &(Params.DACValues[0]),
	.ucIdentifier = DAC_MENU_VALUE0
};



const PROGMEM menu_entry_t MenuValue3 =
{
    .cName = "Value  3",
    .pEncoderFunction = null,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = ValueUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&DacMenuValue0,
    .pNext = (void*)&MenuValue2,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulValue32[3]),
    .ucIdentifier = MENU_VALUE3
};

const PROGMEM menu_entry_t MenuValue2 =
{
    .cName = "Value  2",
    .pEncoderFunction = null,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = ValueUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuValue3,
    .pNext = (void*)&MenuValue1,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulValue32[2]),
    .ucIdentifier = MENU_VALUE2
};

const PROGMEM menu_entry_t MenuValue1 =
{
    .cName = "Value  1",
    .pEncoderFunction = null,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = ValueUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuValue2,
    .pNext = (void*)&MenuValue0,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulValue32[1]),
    .ucIdentifier = MENU_VALUE1
};

const PROGMEM menu_entry_t MenuValue0 =
{
    .cName = "Value  0",
    .pEncoderFunction = null,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = ValueUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuValue1,
    .pNext = (void*)&MenuParam3,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulValue32[0]),
    .ucIdentifier = MENU_VALUE0
};

const PROGMEM menu_entry_t MenuParam3 =
{
    .cName = "Param  3",
    .pEncoderFunction = ParamEncoderFunction,
    .pEnterFunction = ParamEnterFunction,
    .pUpdateDisplayFunction = ParamUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuValue0,
    .pNext = (void*)&MenuParam2,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulParam32[3]),
    .ucIdentifier = MENU_PARAM3
};

const PROGMEM menu_entry_t MenuParam2 =
{
    .cName = "Param  2",
    .pEncoderFunction = ParamEncoderFunction,
    .pEnterFunction = ParamEnterFunction,
    .pUpdateDisplayFunction = ParamUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuParam3,
    .pNext = (void*)&MenuParam1,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulParam32[2]),
    .ucIdentifier = MENU_PARAM2
};

const PROGMEM menu_entry_t MenuParam1 =
{
    .cName = "Param  1",
    .pEncoderFunction = ParamEncoderFunction,
    .pEnterFunction = ParamEnterFunction,
    .pUpdateDisplayFunction = ParamUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuParam2,
    .pNext = (void*)&MenuParam0,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulParam32[1]),
    .ucIdentifier = MENU_PARAM1
};

const PROGMEM menu_entry_t MenuParam0 =
{
    .cName = "Param  0",
    .pEncoderFunction = ParamEncoderFunction,
    .pEnterFunction = ParamEnterFunction,
    .pUpdateDisplayFunction = ParamUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&MenuParam1,
    .pNext = (void*)&MenuSettings,
    .iEI1 = 4, .iEI2 = 2, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ulParam32[0]),
    .ucIdentifier = MENU_PARAM0
};

const PROGMEM menu_entry_t MenuSettings =
{
    .cName = "Settings",
    .pEncoderFunction = null,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = null,
    .pSubmenu = (void*)&SubmenuSettingsToggleString,
    .pPrevious = (void*)&MenuParam0,
    .pNext = (void*)&AdcMenuValue7,
    .iEI1 = 1, .iEI2 = 1, .iEI3 = 1, .iEI4 = 1,
    .pParam = null,
    .ucIdentifier = MENU_SETTINGS
};


//............................................................................
// Submenu "Settings"
//............................................................................


const PROGMEM menu_entry_t SubmenuSettingsToggleString =
{
    .cName = "ToggleString",
    .pEncoderFunction = UCEncoderFunction,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = SettingsToggleStringUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&SubmenuSettingsBack,
    .pNext = (void*)&SubmenuSettingsHexBinary,
    .iEI1 = 1, .iEI2 = 1, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ucDummy1),
    .ucIdentifier = SUBMENU_TOGGLESTRING
};

const PROGMEM menu_entry_t SubmenuSettingsHexBinary =
{
    .cName = "HexBinary",
    .pEncoderFunction = UCEncoderFunction,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = SettingsHexBinaryUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&SubmenuSettingsToggleString,
    .pNext = (void*)&SubmenuSettingsSave,
    .iEI1 = 1, .iEI2 = 1, .iEI3 = 1, .iEI4 = 1,
    .pParam = &(Params.ucDummy2),
    .ucIdentifier = SUBMENU_HEXBINARY
};

const PROGMEM menu_entry_t SubmenuSettingsSave =
{
    .cName = "Save",
    .pEncoderFunction = UCEncoderFunction,
    .pEnterFunction = SettingsSaveEnterFunction,
    .pUpdateDisplayFunction = SettingsSaveUpdateDisplayFunction,
    .pSubmenu = null,
    .pPrevious = (void*)&SubmenuSettingsHexBinary,
    .pNext = (void*)&SubmenuSettingsBack,
    .iEI1 = 1, .iEI2 = 1, .iEI3 = 1, .iEI4 = 1,
    //.pParam = &(Params.ucDummy3),
    .ucIdentifier = SUBMENU_SAVE
};

const PROGMEM menu_entry_t SubmenuSettingsBack =
{
    .cName = "    back",
    .pEncoderFunction = null,
    .pEnterFunction = null,
    .pUpdateDisplayFunction = null,
    .pSubmenu = (void*)&MenuSettings,
    .pPrevious = (void*)&SubmenuSettingsSave,
    .pNext = (void*)&SubmenuSettingsToggleString,
    .iEI1 = 1, .iEI2 = 1, .iEI3 = 1, .iEI4 = 1, .pParam = null,
    .ucIdentifier = SUBMENU_SETTINGSBACK
};



//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

// this variable holds a pointer to the current menu item
menu_entry_t *g_currentMenuItem = (void*)&AdcMenuValue7; //MenuFileSelect;

// this flag is set to "1" everytime the menu changes, jobPanel() will then
// show the new menu item on LCD
uint8_t g_ucMenuItemChanged = 1;

// variable to hold the previous button state
uint8_t g_uLastButtons = 0;

// current scrolling positions
uint8_t g_ucScrollOffset = 0;

uint8_t g_ucParamEntryMode = SHOWVALUE;

struct fat_dir_entry_struct g_dir_entry_panel;
uint16_t g_uiFilePtr = 0;
uint8_t g_ucFileMenuUpdate = 0;

// do text scrolling
void Panel_ScrollMenuName(menu_entry_t *g_currentMenuItem)
{
    uint8_t i, j, ucScrollLength;
    uint16_t u;

    char s[COLUMN_MAX+1];


    ATOMIC_RW(u, xg_uScrollTimer);
    if ((0 == u) || (u > 0xF000))
    {
        // if text has been reset, the timer is counting downwards from 0xFFFF
        // -> wait longer if the text has just been updated, this causes
        // a small delay before scrolling begins
        if (u > 0xF000)
        {
            ATOMIC_RW(xg_uScrollTimer, (Params.iScrollSpeed<<1)*4);
        }
        else
        {
            ATOMIC_RW(xg_uScrollTimer, (Params.iScrollSpeed<<1));
        }
        ucScrollLength = COLUMN_MAX;
        if ( pgm_read_byte(&g_currentMenuItem->ucIdentifier) == MENU_FILESELECT )
        {
            // Scroll from RAM

            if (g_uiFilePtr != 0) // only if SD is inserted and first Dir entry is has been read
            {
                uint8_t ucIsDir = (g_dir_entry_panel.attributes & FAT_ATTRIB_DIR);

                // do not scroll if length < 9 (or 8, if encoder scrolling is active)
                if (strlen(g_dir_entry_panel.long_name + (ucIsDir ? 1 : 0)) <= ucScrollLength)
                {
                    // output menu item
                    Panel_ClearMenuLine();
                    Lcd_Write(0, 0, strlen(g_dir_entry_panel.long_name), g_dir_entry_panel.long_name);
                    if(ucIsDir) Lcd_Write_P(strlen(g_dir_entry_panel.long_name), 0, 1, PSTR("/"));
                }
                else
                {
                    // create scrolling text for LCD
                    j = g_ucScrollOffset;
                    for (i = 0; i < ucScrollLength; i++)
                    {
                        if (j >= (strlen(g_dir_entry_panel.long_name) + (ucIsDir ? 1 : 0) + SCROLL_DISTANCE ))
                            j = 0;

                        if (j < strlen(g_dir_entry_panel.long_name))
                            s[i] = g_dir_entry_panel.long_name[j];
                        else if ( (ucIsDir) && (j == strlen(g_dir_entry_panel.long_name)) )
                            s[i] = '/';
                        else
                            s[i] = 0x020;
                        j++;
                    }
                    Lcd_Write(0, 0, ucScrollLength, s);
                    g_ucScrollOffset++;
                    // add 8 spaces at the end
                    // if SCROLL_DISTANCE > COLUMN_MAX then there is a short waiting time... looks nice
                    if (g_ucScrollOffset >= (strlen(g_dir_entry_panel.long_name) + COLUMN_MAX))
                        g_ucScrollOffset = 0;
                }

            }
        }
        else  // Scroll const char PROGMEM (menu title)
        {
            // do not scroll if length < 9 (or 8, if encoder scrolling is active)
            if (strlen_P(g_currentMenuItem->cName) <= ucScrollLength)
            {
                // output menu item
                Panel_ClearMenuLine();
                Lcd_OverWrite_P(0, 0, strlen_P(g_currentMenuItem->cName), g_currentMenuItem->cName);
            }
            else
            {
                // create scrolling text for LCD
                j = g_ucScrollOffset;
                for (i = 0; i < ucScrollLength; i++)
                {
                    if (j >= (strlen_P(g_currentMenuItem->cName) + SCROLL_DISTANCE ))
                        j = 0;

                    if (j < strlen_P(g_currentMenuItem->cName))
                        s[i] = pgm_read_byte(g_currentMenuItem->cName + j);
                    else
                        s[i] = 0x020;
                    j++;
                }
                Lcd_Write(0, 0, ucScrollLength, s);
                g_ucScrollOffset++;
                // add 8 spaces at the end
                // if SCROLL_DISTANCE > COLUMN_MAX then there is a short waiting time... looks nice
                if (g_ucScrollOffset >= (strlen_P(g_currentMenuItem->cName) + COLUMN_MAX))
                    g_ucScrollOffset = 0;
            }

        }

    }
}

//----------------------------------------------------------------------------
// jobPanel
//
// Read buttons and encoder input, change appropriate parameters, write to  Panel.
//
// This function is called repeatedly.
//
// -> --
// <- --
//----------------------------------------------------------------------------
void Panel_MainFunction(void)
{
    uint8_t uButtons;
    int16_t iEncoderPos;
    uint16_t u;
    menu_entry_t *m;
#ifdef ENCODER_NAVIGATION
    char s[9]; // 8 char + null byte
#endif

    // variables to receive function pointers
    void (*EncoderFunction)(int16_t iEncoderValue, void *pParam);
    void (*EnterFunction)(void);
    void (*UpdateDisplayFunction)(uint8_t ucForceUpdate, void *pParam);

    /*  uncomment this to test encoder acceleration
        iEncoderPos = GetAndResetEncPos();
        SetEncoderIncrements(4, 3, 2, 1);
        if(iEncoderPos)
        {
            printf_P(PSTR("%8i\n"), iEncoderPos);
        };
        return;
    */

    // check buttons (only if button bounce timer reached zero)
    ATOMIC_RW(u, xg_uButtonTimer);
    if(0==u)
    {
        uButtons = Lcd_GetButton();
        // up? (and previous not up)
        if((uButtons&BUTTON_UP)&&(!(g_uLastButtons&BUTTON_UP)))
        {
            ATOMIC_RW(xg_uButtonTimer, BUTTON_TIME);

            // if possible, go back to previous menu item
            m = (void*)pgm_read_word(&g_currentMenuItem->pPrevious);
            if(null!=m)
            {
                g_currentMenuItem = m;
                g_ucMenuItemChanged = 1;
            }
        }

        // down? (and previous not down)
        if((uButtons&BUTTON_DOWN)&&(!(g_uLastButtons&BUTTON_DOWN)))
        {
            ATOMIC_RW(xg_uButtonTimer, BUTTON_TIME);

            // if possible, advance to next menu item
            m = (void*)pgm_read_word(&g_currentMenuItem->pNext);
            if(null!=m)
            {
                g_currentMenuItem = m;
                g_ucMenuItemChanged = 1;
            }
        }

        // enter? (and previous not enter)
        if((uButtons&BUTTON_ENTER)&&(!(g_uLastButtons&BUTTON_ENTER)))
        {
            ATOMIC_RW(xg_uButtonTimer, BUTTON_TIME);

            EnterFunction =
                (void*)pgm_read_word(&g_currentMenuItem->pEnterFunction);

            // if a submenu was defined, switch to this submenu
            m = (void*)pgm_read_word(&g_currentMenuItem->pSubmenu);
            if(null!=m)
            {
                g_currentMenuItem = m;
                g_ucMenuItemChanged = 1;
            }
            else
                // if an "Enter" function was defined, call it now
            {
                if(null!=EnterFunction) EnterFunction();
            }
        }

        // save current button state
        g_uLastButtons = uButtons;
    }

    // Update Parameter for DSP-command
    LastParams.ucDisplayedMenu = Params.ucDisplayedMenu = pgm_read_byte(&g_currentMenuItem->ucIdentifier);

    // get the encoder function and update function
    EncoderFunction =
        (void*)pgm_read_word(&g_currentMenuItem->pEncoderFunction);
    UpdateDisplayFunction =
        (void*)pgm_read_word(&g_currentMenuItem->pUpdateDisplayFunction);

    // display new menu item if necessary and set new encoder acceleration values
    if(g_ucMenuItemChanged)
    {
        // reset the chars which shows the usage of enter key of the encoder
        Panel_ResetEnter();

        ActivityTimer_SetValue(75);
        g_ucMenuItemChanged = 0;

        // force output parameter name to LCD
        ATOMIC_RW(xg_uScrollTimer, 0xFFFF);
        g_ucScrollOffset = 0;

        // set new acceleration values
        Encoder_SetAcceleration(pgm_read_word(&g_currentMenuItem->iEI1),
                               pgm_read_word(&g_currentMenuItem->iEI2),
                               pgm_read_word(&g_currentMenuItem->iEI3),
                               pgm_read_word(&g_currentMenuItem->iEI4));

        // also update parameter display
        if(null!=UpdateDisplayFunction)
            UpdateDisplayFunction(1,
                                  (void*)pgm_read_word(&g_currentMenuItem->pParam));

        // if there's no EncoderFunction, display triangle to indicate if there
        // is a submenu or output a blank line if not
        else
        {
            if(null!=(void*)pgm_read_word(&g_currentMenuItem->pSubmenu))
            {
                // here we choice between enter and back
                if(strcmp_P("    back",g_currentMenuItem->cName) == 0)
                {
                    Panel_ClearDataLine();
                    Panel_PrintBack();
                }
                else
                {
                    Panel_ClearDataLine();
                    Panel_PrintEnter();
                }
            }
            else
            {
                // not used yet see http://www.thoralt.de/phpbb/viewtopic.php?f=2&t=439
                Lcd_ClearLine(0);
            }
        }
    }

    // if ( pgm_read_byte(&g_currentMenuItem->ucIdentifier) != MENU_VALUE )
    // with FPGA module, there is no menu with special treatment. All title lines are set to scrolling mode
    {
        Panel_ScrollMenuName(g_currentMenuItem);
    }


    // call the function which updates the Display for the current parameter
    // LCD will only be written if parameter changed
    if(null!=UpdateDisplayFunction)
        UpdateDisplayFunction(0,
                              (void*)pgm_read_word(&g_currentMenuItem->pParam));

    // read encoder position
    // current parameter is changed if necessary
    iEncoderPos = Encode_GetAndResetEncPos();
    if(iEncoderPos)
    {
        // call encoder function if it is not null
        if(null!=EncoderFunction)
            EncoderFunction(iEncoderPos,
                            (void*)pgm_read_word(&g_currentMenuItem->pParam));
        CheckLimits();
        if(null!=UpdateDisplayFunction)
            UpdateDisplayFunction(0,
                                  (void*)pgm_read_word(&g_currentMenuItem->pParam));

    }

    Panel_UpdateStatusLine();
}


//----------------------------------------------------------------------------
// RoundPrecision
//
// Rounds a value to a multiple of a selectable precision
//
// -> *d: Pointer to number to be rounded
//    prec: Precision which is used to round (0.1, 1, 10, ...)
// <- The result is placed in input parameter *d
//----------------------------------------------------------------------------
void RoundPrecision(float *d, float prec)
{
    int16_t i;

    i = (*d / prec) + 0.5f;
    *d = i * prec;
}



//----------------------------------------------------------------------------
// FileEncoderFunction
//----------------------------------------------------------------------------
void FileEncoderFunction(int16_t iEncoderPos, void *pParam __attribute__((unused)))
{
    uint16_t uiReverseCtr = 1;
    uint8_t ucOneBack = FALSE;

    if (!g_ucSDpresent)
        return; // the panel framework doesn't take return values
        // in case of problems with with the SD card the are just ignored

    ATOMIC_RW(xg_uScrollTimer, 0xFFFF); // refresh display scrolling delay
    g_ucScrollOffset = 0;
	CHECKPOINT;
    g_ucModuleBusy++;

    if ( (iEncoderPos > 0) || (g_uiFilePtr == 0) )
    {
        g_uiFilePtr++;
        if (!fat_read_dir(g_dd_panel, &g_dir_entry_panel))
        {
            ucOneBack = TRUE; // end of directory list encountered.
        }
        g_ucFileMenuUpdate = 1;
    }

    if ( (iEncoderPos < 0) || (ucOneBack == TRUE) )
    {
        fat_reset_dir(g_dd_panel);

        while( uiReverseCtr < g_uiFilePtr-1 )
        {
            fat_read_dir(g_dd_panel, &g_dir_entry_panel);
            uiReverseCtr++;
        }

        g_uiFilePtr = uiReverseCtr;
        fat_read_dir(g_dd_panel, &g_dir_entry_panel);

        g_ucFileMenuUpdate = 1;
    }
	CHECKPOINT;
    if (g_ucModuleBusy > 0) g_ucModuleBusy--;
}

//----------------------------------------------------------------------------
// FileEnterFunction
//----------------------------------------------------------------------------

void FileEnterFunction(void)
{
    uint8_t ucIsDir = (g_dir_entry_panel.attributes & FAT_ATTRIB_DIR);

    if (!g_ucSDpresent)
        return; // the panel framework doesn't take return values
                // in case of problems with with the SD card the are just ignored
	CHECKPOINT;
    g_ucModuleBusy ++;

    if (ucIsDir)
    {
        SD_cd(NULL, &g_dir_entry_panel);
        g_uiFilePtr = 0;
    }
    else
    {
        CFG_LoadFile(g_dir_entry_panel.long_name, PANELMODE);
        g_ucFileMenuUpdate = 1; // to refresh the second line of the splash message, too
    }
	CHECKPOINT;
    if (g_ucModuleBusy > 0) g_ucModuleBusy--;
}


//----------------------------------------------------------------------------
// FileUpdateDisplayFunction
//----------------------------------------------------------------------------
void FileUpdateDisplayFunction(uint8_t ucForceUpdate , void *pParam __attribute__((unused)))
{
    char s[20];         // string composition may be longer than display

    // find file extension; precondition is that filename has no quotes and last char is 0
    uint8_t uc_dotindex = 0;
    uint8_t uc_count = 0;
    char c_extension[COLUMN_MAX+1];
    uint8_t ucIsDir;

    if( ( pgm_read_byte(&g_currentMenuItem->ucIdentifier) == MENU_FILESELECT ) || ucForceUpdate )
    {

        if (!g_ucSDpresent && ((g_ucFileMenuUpdate != 2) || ucForceUpdate))
        {
            sprintf_P(s, PSTR("* FPGA *"));
            Lcd_Write(0, 0, 8, s);
            sprintf_P(s, PSTR("no  card"));
            Lcd_Write(0, 1, 8, s);

            g_uiFilePtr = 0;
            g_ucFileMenuUpdate = 2;

            return;
        }

        if (g_uiFilePtr == 0)
        {
            FileEncoderFunction(0, NULL);   // special case. Readable content is only available after a first read of the directory...
        }

        if ((g_ucFileMenuUpdate == 1) || (ucForceUpdate && (g_ucFileMenuUpdate != 2)) )
        {
            // first line is displayed as if menu title by Panel_ScrollMenuName()
            // string is "just" the file extension for the second line.

            ucIsDir = (g_dir_entry_panel.attributes & FAT_ATTRIB_DIR);

            if (ucIsDir)
            {
                sprintf_P(s, PSTR("DIR     "));
            }
            else
            {
                while (uc_count < strlen(g_dir_entry_panel.long_name))
                {
                    if (g_dir_entry_panel.long_name[uc_count] == '.')
                    {
                        uc_dotindex = uc_count;
                    }
                    uc_count++;
                }

                if ( (uc_dotindex == 0) || (strlen(g_dir_entry_panel.long_name)-uc_dotindex == 1))
                {
                    return;
                }

                uc_dotindex++;  // point to first byte of extension rather than to the '.'

                for (uc_count=0; uc_count < (MIN( strlen(g_dir_entry_panel.long_name)-uc_dotindex, COLUMN_MAX )); uc_count++)
                {
                    c_extension[uc_count] = tolower(g_dir_entry_panel.long_name[uc_dotindex+uc_count]);
                }

                c_extension[uc_count] = 0;
                sprintf_P(s, PSTR("%s       "), c_extension);
            }

            Lcd_Write(0, 1, 8, s);
            g_ucFileMenuUpdate = 0;
        }
    }
}

//----------------------------------------------------------------------------
// ValueEncoderFunction
//
// Gets called when encoder is turned while frequency menu item (or sweep
// start/end frequency submenu item) was active
//
// -> iEncoderPos = encoder value difference since last call (positive or
//                  negative)
//    pParam: --
// <- --
//----------------------------------------------------------------------------
void ValueEncoderFunction(int16_t iEncoderPos __attribute__((unused)), void *pParam __attribute__((unused)))
{
//    uint8_t *p_ucParam = (uint8_t*)pParam;
//    uint8_t ucPortValue;
}

//----------------------------------------------------------------------------
// ValueEnterFunction
//
// -> --
// <- --
//----------------------------------------------------------------------------

void ValueEnterFunction(void)
{

}


//----------------------------------------------------------------------------
// ValueUpdateDisplayFunction
//
// Gets called periodically to update display
//
// -> pParam: --
// <- --
//----------------------------------------------------------------------------
void ValueUpdateDisplayFunction(uint8_t ucForceUpdate , void *pParam)
{
    char s[20];         // string composition may be longer than display
    U32_4B uParam;
    uParam.u32 = *(uint32_t*)pParam;
    uint16_t u;
	uint16_t test1;
    static uint32_t ulLastParam = 0;
	
    ATOMIC_RW(u, xg_uPanelUpdateTimer);

    if ( (ulLastParam != uParam.u32) || !u || ucForceUpdate)
    {
		test1 = pgm_read_byte(&g_currentMenuItem->ucIdentifier) - MENU_VALUE0;
		// LOG_INFO(PSTR("test1: %d\n"), test1);  
		FPGA_SetSPIregister( 0x00 );
        FPGA_SetSPIregister( test1);
        
		FPGA_ReadWriteSPIvalue32(uParam.u8, REVERSE);

        sprintf_P(s, PSTR("%08lX"), uParam.u32);
        Lcd_Write(0, 1, 8, s);

        ATOMIC_RW(xg_uPanelUpdateTimer, 800); // 400ms

        ulLastParam = uParam.u32;
    }
}

static uint8_t g_ucEncoderPos = 0;
//static uint8_t g_ucParamEntryMode = 0;
uint32_t ulIncrement;

void ParamEncoderFunction(int16_t iEncoderPos __attribute__((unused)), void *pParam __attribute__((unused)))
{

    uint32_t* ulpParam = (uint32_t*)pParam;
    U32_4B uParam32;

    if (g_ucParamEntryMode != SHOWVALUE)
    {
        if ( (iEncoderPos < 0) && (g_ucEncoderPos < 7))
        {
            g_ucEncoderPos++;
        }
        else if ( (iEncoderPos > 0) && (g_ucEncoderPos > 0))
        {
            g_ucEncoderPos--;
        }
    }
    else
    {
        ulIncrement =  0x1ul << (g_ucEncoderPos * 4);

        if (iEncoderPos > 0)
        {
            *ulpParam += ulIncrement;
        }
        else if (iEncoderPos < 0)
        {
            *ulpParam -= ulIncrement;
        }

        uParam32.u32 = *ulpParam;

        FPGA_SetSPIregister( MENU_PARAM0 - pgm_read_byte(&g_currentMenuItem->ucIdentifier));
        FPGA_ReadWriteSPIvalue32(uParam32.u8, REVERSE);     // make sure that only a copy is written since the value is destroyed!!!
    }
}

void Panel_AdcValueUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam)
{
   char s[10];         // string composition may be longer than display
   uint8_t uParam =  *(uint8_t*)pParam;
   uint16_t u;
   float f;
   static uint8_t ulLastParam = 0;

   ATOMIC_RW(u, xg_uPanelUpdateTimer);

   if ( (ulLastParam != uParam) || !u || ucForceUpdate)
   {
	   f = Adc_Read(uParam);	
	   sprintf_P(s, PSTR("%7.3fV"), f);
	   Lcd_Write(0, 1, 8, s);

	   ATOMIC_RW(xg_uPanelUpdateTimer, 800); // 400ms

	   ulLastParam = uParam;
   }
}

void Panel_DacValueUpdateEncoderFunction(int16_t iEncoderPos, void *pParam)
{
	
	DAC *dac =  (DAC*)pParam;
	float f =  dac->val;
	float finc = 0.1f;
	
	f += (float)iEncoderPos * finc;
	RoundPrecision(&f, finc);
	// TODO KS add Limit
	dac->val = f;
}

void Panel_DacValueUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam)
{
   char s[10];         // string composition may be longer than display
   DAC *dac =  (DAC*)pParam;
   uint16_t u;
   
   static float ulLastParam = 0;

   ATOMIC_RW(u, xg_uPanelUpdateTimer);

   if ( (ulLastParam != dac->val) || !u || ucForceUpdate)
   {
	   sprintf_P(s, PSTR("%7.2fV"), dac->val);
	   Lcd_Write(0, 1, 8, s);

	   ATOMIC_RW(xg_uPanelUpdateTimer, 800); // 400ms

	   ulLastParam = dac->val;
   }
   // TODO KS richtige Stelle?
   Dac_Output(dac->idx, dac->val);
}

void Panel_DacValueUpdateEnterFunction(void)
{
	;
}


void ParamUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam)
{
    char s[20];         // string composition may be longer than the display
    U32_4B uParam;
    uParam.u32 = *(uint32_t*)pParam;
    static uint8_t ucToggle = 0;
    uint16_t u;

    static uint32_t ulLastParam = 0;


    ATOMIC_RW(u, xg_uMenuBackTimer);

    if ( !u )
    {
        g_ucParamEntryMode = SHOWVALUE;
    }

    ATOMIC_RW(u, xg_uPanelUpdateTimer);

    if ( (ulLastParam != uParam.u32) || !u || ucForceUpdate)
    {
        sprintf_P(s, PSTR("%08lX"), uParam.u32);

        if ( ucToggle & 0x01 )
        {
            s[7-g_ucEncoderPos] = g_ucParamEntryMode ? CHAR_HATCHEDBOX : CHAR_SOLIDBOX;
        }
        ucToggle++;
        Lcd_Write(0, 1, 8, s);

        ATOMIC_RW(xg_uPanelUpdateTimer, 800); // 400ms
    }

    ulLastParam = uParam.u32;

}

void ParamEnterFunction(void)
{
    if (g_ucParamEntryMode == 0)
    {
        g_ucParamEntryMode = CHANGEVALUE;
        ATOMIC_RW(xg_uMenuBackTimer, 10000);    // 5sec
    }
    else
    {
        g_ucParamEntryMode = SHOWVALUE;
        ATOMIC_RW(xg_uMenuBackTimer, 0);        // disable
    }
}

//----------------------------------------------------------------------------
// IEncoderFunction
//
// Gets called when encoder is turned while
//
// -> iEncoderPos = encoder value difference since last call (positive or
//                  negative)
//    pParam: pointer to Params.Burst0 or Params.Burst1 or Params.SweepTime
// <- --
//----------------------------------------------------------------------------
void IEncoderFunction(int16_t iEncoderPos, void *pParam)
{
    int16_t *i = (int16_t*)pParam;

    // change setting
    *i += iEncoderPos;
}


//----------------------------------------------------------------------------
// UCEncoderFunction
//
// Gets called when encoder is turned while in Menu
// for non-negative values
//----------------------------------------------------------------------------
void UCEncoderFunction(int16_t iEncoderPos, void *pParam)
{
    uint8_t *uc = (uint8_t*)pParam;

    // don't make the value "negative"
    if ((*uc == 0) && (iEncoderPos < 0))
    {
        return;
    }
    // change setting
    *uc += iEncoderPos;
}


//----------------------------------------------------------------------------
// SettingsSaveUpdateDisplayFunction
//
// Gets called periodically to update TRMSC range display
//
// -> pParam: --
// <- --
//----------------------------------------------------------------------------
void SettingsSaveUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam __attribute__((unused)))
{
    if(ucForceUpdate)
    {
        // display new setting
        switch(Params.ucDummy3)
        {
        case 0:
            PANEL_PRINT_CHOICE_WITH_ENTER("now");
            break;
        case 1:
        default:
            PANEL_PRINT_CHOICE("auto 30s");
            break;
        }
    }
}


//----------------------------------------------------------------------------
// SettingsToggleStringUpdateDisplayFunction
//
// Gets called periodically to update TRMSC range display
//
// -> pParam: --
// <- --
//----------------------------------------------------------------------------
void SettingsToggleStringUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam)
{

    uint8_t uc = *(int8_t*)pParam;

    static uint8_t ucs = 0;

    if(ucForceUpdate || (uc != ucs))
    {
        // display new setting
        switch(uc)
        {
        default:
        case STRINGAUTO:
            PANEL_PRINT_CHOICE("Auto");
            break;
        case STRINGDEFAULT:
            PANEL_PRINT_CHOICE("Default");
            break;
        case STRINGTOGGLE:
            PANEL_PRINT_CHOICE("Toggle");
            break;
        }
        ucs = uc;
    }
}

//----------------------------------------------------------------------------
// SettingsHexBinaryUpdateDisplayFunction
//
// Gets called periodically to update TRMSC range display
//
// -> pParam: --
// <- --
//----------------------------------------------------------------------------
void SettingsHexBinaryUpdateDisplayFunction(uint8_t ucForceUpdate, void *pParam )
{
    uint8_t uc = *(int8_t*)pParam;

    static uint8_t ucs = 0;

    if(ucForceUpdate || (uc != ucs))
    {
        // display new setting
        switch(uc)
        {
        default:
        case HEXVALUE:
            PANEL_PRINT_CHOICE("Hex");
            break;
        case BINARYVALUE:
            PANEL_PRINT_CHOICE("Binary");
            break;
        }
        ucs = uc;
    }
}


//----------------------------------------------------------------------------
// SettingsSaveEnterFunction
//
// Gets called when encoder is pushed while "Save" menu item was active
//
// -> --
// <- --
//----------------------------------------------------------------------------
void SettingsSaveEnterFunction(void)
{
    if(0==Params.ucDummy3)
    {
        LOG_INFO(PSTR("Write EEP SettingsSaveEnterFunction\n"));
		PANEL_PRINT_CENTER("Saving");
        wait_ms(1000);
        
		eeprom_write_block(&Params, EEPROM_PARAMS, sizeof(Params));
        
		PANEL_PRINT_CENTER("OK");
        g_ucMenuItemChanged = 1;
        wait_ms(3000);
        g_currentMenuItem = (void*)&MenuSettings;
    }
}


//----------------------------------------------------------------------------
// findMenuEntry
//
// -> ucIdentifier
//
// <- g_currentMenuItem
//----------------------------------------------------------------------------
menu_entry_t* findMenuEntry(uint8_t ucIdentifier)
{
    menu_entry_t *foundEntry;
    int iSafetyCntr = 0; // avoid endless loops if lists are not properly maintained or I made an error iterating through it ;-)

    if (pgm_read_byte(&g_currentMenuItem->ucIdentifier) == ucIdentifier)
    {
        return g_currentMenuItem;
    }
    else
    {
        foundEntry = (menu_entry_t *) pgm_read_word(&g_currentMenuItem->pNext);
        do
        {
            if (pgm_read_byte(&foundEntry->ucIdentifier) == ucIdentifier)
            {
                return foundEntry;
            }
            else
            {
                foundEntry = (menu_entry_t *)pgm_read_word(&foundEntry->pNext);
            }
            if (iSafetyCntr++ > 100) break;
        }
        while (((menu_entry_t *)(pgm_read_word(&foundEntry->pNext)) != g_currentMenuItem) && ((menu_entry_t *)(pgm_read_word(&foundEntry->pNext)) != null));
    }
    return g_currentMenuItem;
}


//----------------------------------------------------------------------------
// Panel_SplashScreen
//
// -> --
// <- --
//----------------------------------------------------------------------------
extern const PROGMEM char g_cVersStrShort_P[]; // can't be moved to headerfile ;-(

void Panel_SplashScreen(void)
{
    char s[COLUMN_MAX+1];

    sprintf_P(s, g_cVersStrShort_P);
    Lcd_Write(0, 0, 8, s);
    sprintf_P(s, PSTR("Adr  %-3d"), g_ucSlaveCh);
    Lcd_Write(0, 1, 8, s);
}


//----------------------------------------------------------------------------
// Panel_UpdateStatusLine
//
// -> --
// <- --
//----------------------------------------------------------------------------

// works properly only if jobPanel runs before jobExecute
static void Panel_UpdateStatusLine(void)
{
}

//----------------------------------------------------------------------------
// Panel_PrintEnterChoice
//
// -> --
// <- --
//----------------------------------------------------------------------------
void Panel_PrintEnterChoice(void)
{
    // 2x8 hier wird das Enter ohne Überschreiben ausgeführt
    Lcd_Write_P(7, 1, 1, PSTR(EXTRA_EA_DIP_SIGN_ENTER));
}

//----------------------------------------------------------------------------
// Panel_PrintEnter
//
// -> --
// <- --
//----------------------------------------------------------------------------
void Panel_PrintEnter(void)
{
    // 2x8 hier wird das Enter mit Überschreiben ausgeführt
    // TODO check
    Lcd_OverWrite_P(7, 1, 1, PSTR(EXTRA_EA_DIP_SIGN_ENTER));
}

//----------------------------------------------------------------------------
// Panel_PrintBack
//
// -> --
// <- --
//----------------------------------------------------------------------------
static void Panel_PrintBack(void)
{
    // 2x8 hier wird das Back (Enter) mit Überschreiben ausgeführt
    Lcd_OverWrite_P(7, 1, 1, PSTR(EXTRA_EA_DIP_SIGN_BACK));
}

//----------------------------------------------------------------------------
// Panel_ResetEnter
//
// -> --
// <- --
//----------------------------------------------------------------------------
static void Panel_ResetEnter(void)
{
	;
}

//----------------------------------------------------------------------------
// Panel_ClearDataLine
//
// -> --
// <- --
//----------------------------------------------------------------------------
static void Panel_ClearDataLine(void)
{
    ;

}

//----------------------------------------------------------------------------
// Panel_ClearMenuLine
//
// -> --
// <- --
//----------------------------------------------------------------------------
static void Panel_ClearMenuLine(void)
{
    Lcd_Write_P(0, 0, 10, PSTR("          "));
}
