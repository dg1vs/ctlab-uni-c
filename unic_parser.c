/*
* Copyright (c) 2017 by Karsten Schmidt
* Copyright (c) 2012 by Paul Schmid
* Copyright (c) 2007 by Hartmut Birr, Thoralt Franz, J�rg Wilke
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

#include <inttypes.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <util/delay.h>

#include "fat.h"
#include "debug.h"
#include "parser.h"
#include "panel.h"
#include "sd_commands.h"
#include "fpga.h"
#include "Rtc.h"
#include "Timer.h"
#include "helper_macros.h"
#include "unic_hw.h"
#include "I2CRegister.h"
#include "unic_parser.h"




const PROGMEM char Mnemonics [][5] =
{
	{'n','o','p',  2, 53}, // No OPeration, must be in the first place; mapped to REM/TST SubCH 253
	{'v','a','l',  0,  0}, // VALue command, VAL can be omitted (abbreviation for other commands)
	{'p','i','o',  0, 30}, // PIO
	{'d','d','r',  0, 40}, // DDR
	{'r','a','w',  0, 50}, // RAW
	{'d','s','p',  0, 80}, // DSP
	{'c','l','k',  0, 90}, // CLK
	{'d','e','f',  1, 00}, // DEF
	{'o','p','t',  2, 00}, // OPT
	{'i','c','b',  2, 30}, // I2C_Byte
	{'i','c','w',  2, 31}, // I2C_Word
	{'i','c','s',  2, 32}, // I2C_Swap
	{'i','c','t',  2, 33}, // I2C_Temperature
	{'i','c','l',  2, 35}, // I2C_Long // TODO REMOVE
	{'i','c','f',  2, 38}, // I2C_Find Dev
	{'i','c','a',  2, 39}, // I2C_Address
	{'c','f','g',  2, 40}, // CFG
	{'l','s','t',  2, 41}, // LST
	{'d','i','r',  2, 41}, // DIR
	{'f','n','m',  2, 42}, // FNM
	{'f','n','a',  2, 43}, // FNA
	{'f','d','l',  2, 44}, // FDL
	{'c','d','r',  2, 45}, // CDR (new: change directory)
	{'l','s','s',  2, 46}, // LSS (new: list with size)
	{'f','q','u',  2, 49}, // FQU
	{'w','e','n',  2, 50}, // EEPROM Write ENable
	{'e','r','c',  2, 51}, // ERror Count since last reset
	{'s','b','d',  2, 52}, // SerBaud UBRR register with U2X=1
	{'t','s','t',  2, 53}, // TeST, returns command without doing anything
	{'r','e','m',  2, 53}, // same as TST
	{'i','d','n',  2, 54}, // IDeNtify
	{'s','t','r',  2, 55}, // STatus Request
	{'f','w','r',  2, 60}, // FileWriteRegister, LabScript-Registerinhalt  0..9 in (ggf. mit FNA bestimmte) Textdatei schreiben, wird automatisch erstellt (wenn n�tig), Header angelegt, ge�ffnet und geschlossen, Registernummer und Uhrzeit werden ebenfalls geschrieben (TAB-getrennt).
	{'f','w','v',  2, 70}, // LabScript: FileWriteValue, angegebenen Wert in (ggf. mit FNA bestimmte) Textdatei schreiben, wird automatisch erstellt (wenn n�tig) ge�ffnet und geschlossen, Header angelegt. Index und Uhrzeit werden ebenfalls geschrieben (TAB-getrennt).
	{'w','t','h',  2, 90}, // LabScript: WTH xg_uScriptWTH
	{'w','t','m',  2, 91}, // LabScript: WTM xg_uScriptWTM
	{'w','t','s',  2, 92}, // LabScript: WTS xg_uScriptWTS
	{'d','l','y',  2, 99}, // LabScript: xg_uiScriptDelay
	{'r','e','g',  3, 00}, // LabScript: REG
	{'a','c','c',  3, 00}, // LabScript: ACC
	{'m','o','v',  3, 10}, // LabScript: MOV
	{'d','e','c',  3, 20}, // LabScript: DEC
	{'i','n','c',  3, 30}, // LabScript: INC
	{'c','p','z',  3, 40}, // LabScript: CPZ
	{'x','c','h',  3, 50}, // LabScript: XCH
	{'g','e','t',  4, 00}, // LabScript: GET
	{'p','u','t',  5, 00}, // LabScript: PUT
	{'m','u','l',  6, 00}, // LabScript: MUL
	{'d','i','v',  6, 10}, // LabScript: DIV
	{'a','d','d',  6, 20}, // LabScript: ADD
	{'s','u','b',  6, 30}, // LabScript: SUB
	{'s','q','r',  6, 40}, // LabScript: SQR
	{'s','q','u',  6, 50}, // LabScript: SQU
	{'n','e','g',  6, 60}, // LabScript: NEG
	{'l','b','l', 10, 00}, // LabScript: LBL
	{'g','t','o', 11, 00}, // LabScript: GTO
	{'b','r','a', 11, 00}, // LabScript: BRA (always)
	{'b','r','g', 12, 00}, // LabScript: BRG (greater than)
	{'b','g','e', 13, 00}, // LabScript: BGE (greater or equal)
	{'b','e','q', 14, 00}, // LabScript: BEQ (equal)
	{'b','l','e', 15, 00}, // LabScript: BLE (less or equal)
	{'b','r','l', 16, 00}, // LabScript: BRL (less)
	{'i','n','p', 20, 00}, // LabScript: INP
	{'o','u','t', 20, 00}, // LabScript: OUT
	{ 0, 0, 0, 0, 0}       // Terminator
};


void ReturnDIR(void)
{
	g_ucModuleBusy++;
	SD_ls(0);
	if (g_ucModuleBusy > 0) g_ucModuleBusy--;
}

//void ReturnLSS(PARAMTABLE* ParamTable __attribute__((unused)))
//{
//g_ucModuleBusy++;
//SD_ls(1);
//if (g_ucModuleBusy > 0) g_ucModuleBusy--;
//}
//

void ReturnFNM()
{
	g_ucModuleBusy++;
	SD_filenum();
	if (g_ucModuleBusy > 0) g_ucModuleBusy--;
}

uint8_t ParseFileParam(uint16_t SubCh, char* cp, uint16_t uiFileNumber , uint8_t ucScriptMode)
{
	uint8_t result = NoErr;
	g_ucModuleBusy++;

	LOG_INFO(PSTR("ParseFileParam\n"));  

	// if the function is called with a file number in the directory
	// we need to search for the file name first since the SD library takes only file names
	// parsing the SD directory is really fast so it does not make any sense to waste memory by buffering file number vs. file name or dir_entry
	// that way we can handle really big directories
	if (uiFileNumber != 0xFFFF)
	{
		uint16_t i= 0;

		if (!g_ucSDpresent)
		{
			if (g_ucModuleBusy > 0) g_ucModuleBusy--;
			return ERROR_NOCARD;
		}
		// parse directory
		struct fat_dir_entry_struct dir_entry;
		do
		{
			if (!fat_read_dir(g_dd, &dir_entry))
			{
				if (g_ucModuleBusy > 0) g_ucModuleBusy--;
				return ERROR_OPENFILE;
			}
			i++;
		}
		while (i <= uiFileNumber);

		memcpy(cp, dir_entry.long_name, MIN( strlen(dir_entry.long_name)+1, FILENAMEBUFFERSIZE ) );
		fat_reset_dir(g_dd);
	}
	if ( SubCh == 210 ) // OPT 10
	{
		if ((Status.EEUnlocked == FALSE) && (g_newEEMode == FALSE))
		{
			result = LockedErr;
		}
		else
		{
			if ( strlen(cp) > sizeof(Params.cp_OPT30_AutoLoadFileName))
			{
				result = ParamErr;
			}
			else
			{
				strcpy(Params.cp_OPT30_AutoLoadFileName, cp);
				if (Status.EEUnlocked == TRUE)
				{
					eeprom_write_block(Params.cp_OPT30_AutoLoadFileName, (void*)((uint16_t)Params.cp_OPT30_AutoLoadFileName-(uint16_t)&Params), sizeof(Params.cp_OPT30_AutoLoadFileName));
				}
			}
			LOG_INFO(PSTR("Status.EEUnlocked = FALSE\n"));
			Status.EEUnlocked = FALSE;
		}
	}
	else if ( SubCh == 211 ) // OPT 11
	{
		if ((Status.EEUnlocked == FALSE) && (g_newEEMode == FALSE))
		{
			result = LockedErr;
		}
		else
		{
			if ( strlen(cp) > sizeof(Params.cp_OPT31_SaveDefaultFileName))
			{
				result = ParamErr;
			}
			else
			{
				strcpy(Params.cp_OPT31_SaveDefaultFileName, cp);
				if (Status.EEUnlocked == TRUE)
				{
					eeprom_write_block(Params.cp_OPT31_SaveDefaultFileName, (void*)((uint16_t)Params.cp_OPT31_SaveDefaultFileName-(uint16_t)&Params), sizeof(Params.cp_OPT31_SaveDefaultFileName));
				}
				strcpy(pParams->cp_FNA_SaveFileName, pParams->cp_OPT31_SaveDefaultFileName);
			}
			LOG_INFO(PSTR("Status.EEUnlocked = FALSE\n"));
			Status.EEUnlocked = FALSE;
		}
	}
	else if ( SubCh == 243 ) // FNA
	{
		if ( strlen(cp) > sizeof(Params.cp_FNA_SaveFileName))
		{
			result = ParamErr;
		}
		else
		{
			strcpy(Params.cp_FNA_SaveFileName, cp);
		}
	}
	else if ( SubCh == 240 ) //  CFG
	{
		result = CFG_LoadFile(cp, ucScriptMode);
	}
	else if ( SubCh == 244 ) //  FDL
	{
		result = SD_rm(cp);
	}
	else if ( SubCh == 245 ) //  CDR
	{
		result = SD_cd(cp, NULL);

		if (result == SUCCESS)
		{
			g_uiFilePtr = 0;
			FileUpdateDisplayFunction(0, NULL);
		}
	}
	else if ( SubCh == 249 ) //  FQU
	{
		result = SD_query(cp);
	}
	else  // SubCh out of range
	{
		result = ParamErr;
	}
	if (g_ucModuleBusy > 0) g_ucModuleBusy--;

	return result;
}

// called from parser.c
// to be removed
uint8_t ParseStringParam(uint16_t SubCh, char* cp __attribute__((unused)))
{
	//uint8_t ucChannel;
	CHECKPOINT;
	if (( SubCh >= 900) && ( SubCh <= 969) )  //  TSR 900..969
	{
		//ucChannel = SubCh - 900; // TSR
		CHECKPOINT;
		//return TSR_FPGA_SPI(cp, ucChannel);
		return ParamErr;
	}
	return ParamErr;
}

void ScriptWait(uint16_t SubCh)
{
	if ( SubCh == 290) // WTH
	{
		uint16_t ui = 1;
		ATOMIC_RW(xg_uiScriptWTH, 1);
		CHECKPOINT;
		g_ucModuleBusy++;

		while(ui)
		{
			_delay_ms(100);
			ATOMIC_RW(ui, xg_uiScriptWTH);
		}

		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	}
	else if ( SubCh == 291) // WTM
	{
		uint16_t ui = 1;
		ATOMIC_RW(xg_uiScriptWTM, 1);
		CHECKPOINT;
		g_ucModuleBusy++;

		while(ui)
		{
			_delay_ms(100);
			ATOMIC_RW(ui, xg_uiScriptWTM);
		}

		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	}
	else if ( SubCh == 292) // WTS
	{
		uint16_t ui = 1;
		ATOMIC_RW(xg_uiScriptWTS, 1);
		CHECKPOINT;
		g_ucModuleBusy++;

		while(ui)
		{
			_delay_ms(10);
			ATOMIC_RW(ui, xg_uiScriptWTS);
		}

		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	}
}

void ParseGetParam(uint16_t SubCh, uint8_t ucScriptMode )
{
	char fmt[20];

	union
	{
		float f;
		int32_t l;
		uint32_t U;
		int16_t i;
		uint16_t u;
		uint8_t b;
		const char* s;
	} Data;

	// U32_4B uParam;
	//uint8_t uParam8;
	uint8_t ucINPflag = 0;
	
	uint8_t ucRegister = (uint8_t)(SubCh % 10);
	uint8_t ucArray[3];

	LOG_INFO(PSTR("ParseGetParam\n"));  

	if ( (g_ucSearchLabel == TRUE) && !(( SubCh >= 1000 ) && ( SubCh <= 1099 )) ) 	// Labels
	{
		return;
	}

	if (SubCh >= 2000)
	{
		ucINPflag = 1;
		SubCh -= 2000;
	}

	switch (SubCh)
	{
		case 4:				// FPGA-Readflags auf SPI-Register 0
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 5:				// FPGA-Writeflags
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 8:				// Frequenzz�hler/Counter (Eingang T1/PB1 vom ATmega644)
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 9:				// Frequenzz�hler Mode
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 10 ... 17:		// MCP3208, 12-Bit-ADC, 8 Kan�le, Spannungswert 0..+10 (V).
			Data.f = Adc_Read(ucRegister);
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.f; }
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, Data.f);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 19:			// 8 value at ones
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 20 ... 23:		// MCP4822 Ausgabewerte DAC 12 Bit 4 Kan�le, Spannungswert -10..+10 (V)
			Data.f = pParams->DACValues[ucRegister].val;
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.f; }
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, Data.f);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 30 ... 33:		// Relais-Ports 0 bis 3, Ausgabewerte
			Data.b = pParams->RelayPortArray[ucRegister];
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=0x%02X"), g_ucSlaveCh, SubCh, Data.b);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 34 ... 35:		// Ports auf IO16-Bridge, Aus- und Eingabewerte
			if (SubCh == 34)
			{
				ucArray[0] = PORTIO_PCA9555D_REGISTER_INPUT_PORT_0;
				I2CRegister_WriteImplicit(PORTIO_PCA9555D_ADDRESS<<1, 1, ucArray);
			}
			else
			{
				ucArray[0] = PORTIO_PCA9555D_REGISTER_INPUT_PORT_1;
				I2CRegister_WriteImplicit(PORTIO_PCA9555D_ADDRESS<<1, 1, ucArray);
			}
			I2CRegister_ReadImplicit(PORTIO_PCA9555D_ADDRESS<<1, 1, &Data.b);
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=0x%02X"), g_ucSlaveCh, SubCh, (Data.b));
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 39:			// TODO Check byteorder
			Data.U= (uint32_t)(	(uint32_t) pParams->RelayPortArray[3] << 24 |
			(uint32_t) pParams->RelayPortArray[2] << 16 |
			(uint32_t) pParams->RelayPortArray[1] <<  8 |
			(uint32_t) pParams->RelayPortArray[0]         );
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.U; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d= %08lX"), g_ucSlaveCh, SubCh, Data.U);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 44 ... 45:		// Ports auf IO16-Bridge, Datenrichtung (Bit = 1 -> Ausgang)
			// TODO check for PortIO attached to the uni-c
			if (SubCh == 44)
			{
				ucArray[0] = PORTIO_PCA9555D_REGISTER_CONFIG_0;
				I2CRegister_WriteImplicit(PORTIO_PCA9555D_ADDRESS<<1, 1, ucArray);
			}
			else
			{
				ucArray[0] = PORTIO_PCA9555D_REGISTER_CONFIG_1;
				I2CRegister_WriteImplicit(PORTIO_PCA9555D_ADDRESS<<1, 1, ucArray);
			}
			I2CRegister_ReadImplicit(PORTIO_PCA9555D_ADDRESS<<1, 1, &Data.b);
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=0x%02X"), g_ucSlaveCh, SubCh, (Data.b));
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 50 ... 57:			// 8 ADC-Rohwerte ohne Skalierung/Offset
			Data.u = Adc_RawRead(ucRegister);
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.u; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%u"), g_ucSlaveCh, SubCh, Data.u);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 60 ... 79:			// FPGA Register - 60
			{
			uint32_t test = 0;
			uint8_t reg = ((uint8_t)(SubCh - 60));
			// TBR different handling to FPGA-Board FPGA_SetSPIregister( 0x00 );
			FPGA_SetSPIregister( reg );
			test = FPGA_ReadSPIvalue32();
			if (ucINPflag) 	{ g_dScriptRegister[0] = test; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=0x%08lX"), g_ucSlaveCh, SubCh, test);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
			}
		case 80:
			Data.b = Params.ucDisplayedMenu;
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%u"), g_ucSlaveCh, SubCh, Data.b);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 90 ... 95:				// CLK (read RTC)
			{
			Rtc_ReadChipTime(); // read hardware RTC first
			uint8_t sreg = SREG;
			cli();
			switch (SubCh)
			{
				case 90:
					Data.b = gRTC.ucHours;
					break;
				case 91:
					Data.b = gRTC.ucMinutes;
					break;
				case 92:
					Data.b = gRTC.ucSeconds;
					break;
				case 93:
					Data.b = gRTC.ucDay;
					break;
				case 94:
					Data.b = gRTC.ucMonth;
					break;
				case 95:
					Data.b = gRTC.ucYear;
					break;
			}
			SREG = sreg;
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%u"), g_ucSlaveCh, SubCh, Data.b);
			SerStr((char*)gucBuffer, ucScriptMode);
			}
			break;
		case 96 ... 98:
			{
			Rtc_ReadChipTime();
			uint8_t sreg = SREG;
			cli();
			switch (SubCh)
			{
				case 96:
					sprintf_P((char*)gucBuffer, PSTR("#%d:96=0 [%02d:%02d:%02d]"), g_ucSlaveCh, gRTC.ucHours, gRTC.ucMinutes, gRTC.ucSeconds );
					break;
				case 97:
					sprintf_P((char*)gucBuffer, PSTR("#%d:97=0 [%02d.%02d.%02d]"), g_ucSlaveCh, gRTC.ucDay, gRTC.ucMonth, gRTC.ucYear);
					break;
				case 98:
					sprintf_P((char*)gucBuffer, PSTR("#%d:98=0 [%02d:%02d:%02d %02d.%02d.%02d]"), g_ucSlaveCh, gRTC.ucHours, gRTC.ucMinutes, gRTC.ucSeconds, gRTC.ucDay, gRTC.ucMonth, gRTC.ucYear);
					break;
			}
			SREG = sreg;
			SerStr((char*)gucBuffer, ucScriptMode);
			}
			break;
		case 100 ... 107:			// Offsets ADC-Wert in ADC-Rohwerten
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%d"), g_ucSlaveCh, SubCh, pParams->ADCoffsets[ucRegister]);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 110 ... 117:			// Kanal-Skalierung AD-Wert; Messwert = (ADC-Rohwert + Offset) * Kanal-Skalierung * Basis-Skalierung
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, pParams->ADCscales[ucRegister]);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 120 ... 123:			// Offsets DAC-Werte in DAC-Rohwerten
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%d"), g_ucSlaveCh, SubCh, pParams->DACOffset[ucRegister]);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 130 ... 133:			// Kanal-Skalierungen DAC-Werte; Ausgabewert = (Kanal-Skalierung * Spannung / Basis-Skalierung) + Kanal-Offset
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, pParams->DACscale[ucRegister]);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 140:					// DAC-Basis-Skalierung; m�glichste nicht �ndern!
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, pParams->DACBaseScale);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 141:					// ADC-Basis-Skalierung; m�glichste nicht �ndern!
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, pParams->ADCbaseScale);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 210:					 // OPT10
			sprintf_P((char*)gucBuffer, PSTR("#%d:210=\"%s\""), g_ucSlaveCh, Params.cp_OPT30_AutoLoadFileName);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 211:					 // OPT11
			sprintf_P((char*)gucBuffer, PSTR("#%d:211=\"%s\""), g_ucSlaveCh, Params.cp_OPT31_SaveDefaultFileName);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 230:					//I2C Byte
			if (I2CRegister_ReadImplicit(Params.ucI2CAddress<<1, 1, &Data.b))
			{
				SerPrompt(I2CErr, 0, ucScriptMode);
				break; // TODO Check
			}
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%d"), g_ucSlaveCh, SubCh, (Data.b));
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 231:					// I2C Word
			if (I2CRegister_ReadImplicit(Params.ucI2CAddress<<1, 2, ucArray))
			{
				SerPrompt(I2CErr, 0, ucScriptMode);
				break; // TODO Check
			}
			Data.u = (ucArray[0] << 8) | ucArray[1];
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.u; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%d"), g_ucSlaveCh, SubCh, (Data.u));
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 232:					// I2C Word swapped
			if (I2CRegister_ReadImplicit(Params.ucI2CAddress<<1, 2, ucArray))
			{
				SerPrompt(I2CErr, 0, ucScriptMode);
				break; // TODO Check
			}
			Data.u = (ucArray[1] << 8) | ucArray[0];
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.u; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%d"), g_ucSlaveCh, SubCh, (Data.u));
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 233:				// LM75 Temperature
			// i2c are 7 bit addresses, i2c_read_regs_implicit is using 8 bit addresses
			if (I2CRegister_ReadImplicit(Params.ucI2CAddress<<1, 2, ucArray))
			{
				SerPrompt(I2CErr, 0, ucScriptMode);
				break;
			}
			Data.i = (ucArray[0] << 1) | (ucArray[1]>>7);
			if (Data.i & 0x100)
			{
				Data.i |= 0xfe00; 	// temperature is negative, adjust the upper bits
			}
			Data.f= Data.i / 2.0f;
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.f; }
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, Data.f );
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 234:				// DS1631 Temperature
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 238:
			/* i2c Scanner
			Die 7-Bit Adressierung ist die erste Adressierungsform des I2C Busses und erm�glicht prinzipiell bis zu 128 (27) Ger�te an einem Bus.
			Durch die Reservierung einiger Adressen, unter anderem f�r die 10-Bit-Adressierung, ist die 7-Bit-Addressierung auf 112 Ger�te begrenzt und erm�glicht
			so eine konfliktfreie Adressierung des 7-Bit- und des 10-Bit-Adressraumes (siehe Tabelle unten: Reservierte Adressen). Da ein Byte aber acht Bits hat,
			und die Adresse nur sieben Bits, gibt es noch ein Bit, das die Datenrichtung angibt. Es bestimmt, ob der Master Daten empfangen m�chte oder
			ob er dem Slave Daten schicken m�chte. Dieses letzte Bit wird als R/W Bit bezeichnet (0 steht f�r "schreiben", 1 f�r "lesen").
			Hat ein Slave seine Adresse richtig verstanden und ist bereit Daten zu empfangen oder zu verschicken, dann sendet er eine Best�tigung.
			Hat er seine Adresse nicht richtig mitbekommen, oder ist gerade nicht in der Lage, Daten zu verschicken oder zu empfangen,
			dann bleibt die Best�tigung aus. Der Master kann dann eine Stoppbedingung erzeugen, so dass die Daten�bertragung abgebrochen wird.	*/
			for (uint8_t i=0; i<128; i++)
			{
				if (!(I2CRegister_Read(i<<1, 0, 1, ucArray)))
				{
					sprintf_P((char*)gucBuffer, PSTR("#%d:%d I2C device found at address = %d"), g_ucSlaveCh, SubCh, i);
					SerStr((char*)gucBuffer, ucScriptMode);
				}
			}
			break;
		case 239:
			Data.b = Params.ucI2CAddress;
			if (ucINPflag) 	{ g_dScriptRegister[0] = Data.b; }
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%u"), g_ucSlaveCh, SubCh, Data.b);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 241:					// LST /DIR
			ReturnDIR();
			break;
		case 242:					// FNM
			ReturnFNM();
			break;
		case 243:					 // FNA
			sprintf_P((char*)gucBuffer, PSTR("#%d:243=\"%s\""), g_ucSlaveCh, Params.cp_FNA_SaveFileName);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 244:					 // FDL
			SerPrompt(NotImplemetedErr, 0, ucScriptMode);
			break;
		case 251:
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%u"), g_ucSlaveCh, SubCh, g_ucErrCount);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 252:
			sprintf_P((char*)gucBuffer, PSTR("#%d:%d=%u"), g_ucSlaveCh, SubCh, Params.ucSerBaudReg);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 254:
			PrintIDNstring();
			break;
		case 255:
			SerPrompt(NoErr, Status.u8, ucScriptMode);
			break;
		case 290 ... 292:				 // WTH || WTM ||  WTS
			ScriptWait(SubCh);
			SerPrompt(NoErr, Status.u8, ucScriptMode);
			break;
		case 299:						 // DLY
			sprintf_P((char*)gucBuffer, PSTR("#%d:299=%u"), g_ucSlaveCh, xg_uiScriptDelay);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 300 ... 309:				// Labscript REG0..9, ACC
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucSlaveCh, SubCh, g_dScriptRegister[ucRegister]);
			SerStr((char*)gucBuffer, ucScriptMode);
			break;
		case 320 ... 329:				// Labscript DEC REG0..9
			g_dScriptRegister[ucRegister] -= 1.0f;
			g_dCompareParam = g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 330 ... 339:				// Labscript INC REG0..9
			g_dScriptRegister[ucRegister] += 1.0f;
			g_dCompareParam = g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 340 ... 349:					// Labscript CPZ REG0..9
			g_dCompareParam = g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 400 ... 409:					// Labscript GET: REG0..9
			{
			g_cWaitForResponse = TRUE;

			sprintf_P((char*)gucBuffer, PSTR("%d:%d?"), g_ucScriptMCH, g_ucScriptSCH);
			SerStr((char*)gucBuffer, BUSMODE);       // always to bus
			CHECKPOINT;
			g_ucModuleBusy++;

			// wait for max. Params.uiGETtimeout
			uint16_t ui = 1;
			ATOMIC_RW(xg_uiScriptDLY, Params.uiGETtimeout);

			while ((g_cWaitForResponse) && (ui))
			{
				ATOMIC_RW(ui, xg_uiScriptDLY);
			}

			if (g_ucModuleBusy > 0) g_ucModuleBusy--;

			if (g_cWaitForResponse) // no answer received
			{
				SerPrompt(ParamErr, 0, ucScriptMode);
				break;
			}

			// write value from bus buffer to REG0..9
			g_dScriptRegister[ucRegister] = g_dWaitResponse;

			SerPrompt(NoErr, 0, ucScriptMode);
			}
			break;
		case 500 ... 509:				// Labscript PUT: REG0..9
			sprintf_P(fmt, PSTR("#%%d:%%d=%%.%df"), Params.ucFloatPrecision); // TODO DKS Variabler Wert
			sprintf((char*)gucBuffer, fmt, g_ucScriptMCH, g_ucScriptSCH, g_dScriptRegister[ucRegister]);
			SerStr((char*)gucBuffer, BUSMODE);          // always to bus
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 600 ... 609:				// Labscript MUL: ACC *= REG0..9
			g_dScriptRegister[0] *= g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 610 ... 619:				// Labscript DIV: ACC /= REG0..9
			/*
			if (g_dScriptRegister[ucRegister] == 0)
			{
			SerPrompt(ParamErr, 0);     // DIV by 0!
			return;
			}
			*/
			// if the argument is zero the result is "inf" or "-inf" = pos/neg infinity
			g_dScriptRegister[0] /= g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 620 ... 629:				// Labscript ADD: ACC += REG0..9
			g_dScriptRegister[0] += g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 630 ... 639:				// Labscript SUB: ACC -= REG0..9
			g_dScriptRegister[0] -= g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 640 ... 649:				// Labscript SQR: REG0..9 = SQRT(REG0..9)
			/*
			if (g_dScriptRegister[ucRegister] < 0)
			{
			SerPrompt(ParamErr, 0);     // negative!
			return;
			}
			*/
			// if the argument is negative the result is "nan" = "not a number"
			g_dScriptRegister[ucRegister] = sqrt(g_dScriptRegister[ucRegister]);
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 650 ... 659:				// Labscript SQU: REG0..9 *= REG0..9
			g_dScriptRegister[ucRegister] *= g_dScriptRegister[ucRegister];
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 660 ... 669:				// Labscript NEG REG0..9
			g_dScriptRegister[ucRegister] *= -1.0f;
			SerPrompt(NoErr, 0, ucScriptMode);
			break;
		case 1000 ... 1099:				// Labscript LBL: SET LABEL
			if (ucScriptMode)
			{
				uint8_t ucLabel = (SubCh % 100) & 0x1F;    // Labels 0..31
				if (g_ucGotoLabel == ucLabel)
				{
					g_ucSearchLabel = FALSE;
				}
				g_ulLabelPointer[ucLabel] = g_ulCurrentINIfilePos;
				g_ucLabelValid[ucLabel] = TRUE;

				LOG_INFO(PSTR("setting pos: #%d @ %ld\n"), ucLabel, g_ulCurrentINIfilePos);
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			}
			else 
			{
				break; //TODO Check
			}
		case 1100 ... 1699:					// Labscript Branch
			if (ucScriptMode)
			{
				uint8_t ucLabel = (SubCh % 100) & 0x1F; // Labels 0..31
				uint8_t ucBranch = FALSE;
				if ( ( SubCh >= 1100 ) && ( SubCh <= 1199 ) ) // BRA / GTO
				{
					ucBranch = TRUE;
				}
				else if ( ( SubCh >= 1200 ) && ( SubCh <= 1299 ) ) // BRG
				{
					if (g_dCompareParam > 0.0f)
					ucBranch = TRUE;
				}
				else if ( ( SubCh >= 1300 ) && ( SubCh <= 1399 ) ) // BGE
				{
					if (g_dCompareParam >= 0.0f)
					ucBranch = TRUE;
				}
				else if ( ( SubCh >= 1400 ) && ( SubCh <= 1499 ) ) // BEQ
				{
					if (g_dCompareParam == 0.0f)
					ucBranch = TRUE;
				}
				else if ( ( SubCh >= 1500 ) && ( SubCh <= 1599 ) ) // BLE
				{
					if (g_dCompareParam <= 0.0f)
					ucBranch = TRUE;
				}
				else if ( ( SubCh >= 1600 ) && ( SubCh <= 1699 ) ) // BRL
				{
					if (g_dCompareParam < 0.0f)
					ucBranch = TRUE;
				}
				if (g_ucLabelValid[ucLabel] == FALSE)
				{
					g_ucSearchLabel = ucBranch; // only search if the branch is really required
					g_ucGotoLabel = ucLabel;
					LOG_INFO(PSTR("seeking forward pos: #%d\n"), ucLabel);
				}
				else
				{
					if (ucBranch == TRUE)
					{
						g_lIniFileSeekPos = g_ulLabelPointer[ucLabel];
						LOG_INFO(PSTR("seeking pos: #%d @ %ld\n"), ucLabel, g_lIniFileSeekPos);
					}
				}
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			}
			else
			{
				break;
			}
		default:
			SerPrompt(ParamErr, 0, ucScriptMode);
			break;
	}
	return;
}
void Handle_EEStuff(const uint8_t mode, uint8_t ucScriptMode __attribute__((unused)))
{
	if (!((mode == 1) || (mode == 42) || (mode == 23) || (mode == 24)))
	{
		SerPrompt(ParamErr, 0, ucScriptMode);
	}
	if (mode == 1)  // legacy mode
	{
		LOG_INFO(PSTR("Status.EEUnlocked = 1\n"));
		Status.EEUnlocked = TRUE;
	}
	if (mode == 42) // enable new ee-mode
	{
		LOG_INFO(PSTR("enable new ee-mode\n"));
		g_newEEMode = TRUE;
	}
	if (mode == 23) // write the Params info ee
	{
		LOG_INFO(PSTR("Write EEP WEN=23\n"));
		if (g_newEEMode == TRUE)
		{
			LOG_INFO(PSTR("Write EEP WEN=23\n"));
			eeprom_write_block(&Params, EEPROM_PARAMS, sizeof(Params));
		}
		else
		{
			SerPrompt(SyntaxErr, 0, ucScriptMode);
		}
	}
	if (mode == 24) // disable new ee-mode
	{
		LOG_INFO(PSTR("disable new ee-mode\n"));
		g_newEEMode = FALSE;
	}

}

uint8_t HandleSetParamWithEE(float Value, PARAM_DATA D)
{
	uint8_t result = NoErr;
	uint8_t size = 0;
	uint16_t* eeptr; 

	eeptr= (void*)((uint16_t)D.ram.b - (uint16_t)&Params);
	
	if ((Status.EEUnlocked == FALSE) && (g_newEEMode == FALSE))
	{
		result = LockedErr;
	}
	else
	{
		switch(D.type)
		{
			case PARAM_FLOAT:
				*(D.ram.f) = ((float) Value);
				size = sizeof(float);
			break;

			case PARAM_INT:
				*(D.ram.i) = ((int16_t) Value);
				size = sizeof(int16_t);
			break;
			
			case PARAM_BYTE:
				*(D.ram.b) = ((uint8_t) Value);
				size = sizeof(uint8_t);
			break;

			default:
				size = 0;
			break;
		}
		if (Status.EEUnlocked == TRUE)
		{
			eeprom_write_block(D.ram.v, eeptr, size);
		}
		Status.EEUnlocked = FALSE;
	}
	return result;
}


void ParseSetParam(uint16_t SubCh, float Param, uint8_t ucScriptMode __attribute__((unused)))
{
	PARAM_DATA Data;
	uint8_t ucTmp;
	float  dTmp;
	
	uint8_t ucError = 0;
	uint8_t sreg;
	
	uint8_t result = NoErr;
	uint8_t ucRegister = (uint8_t)(SubCh % 10);
	uint8_t ucOUTflag = 0;

	LOG_INFO(PSTR("ParseSetParam\n"));  

	if (Status.Busy)
	{
		SerPrompt(BusyErr, 0, ucScriptMode);
		return;
	}

	if (SubCh == 250)
	{
		Handle_EEStuff((uint8_t) Param, ucScriptMode);
	}
	else // (SubCh == 250)
	{
		if (SubCh >= 2000)
		{
			ucOUTflag = 1;
			SubCh -= 2000;
		}

		switch (SubCh)
		{
			case 9:		
				SerPrompt(NotImplemetedErr, 0, ucScriptMode);
				break;
			case 20 ... 23:				// VAL Outputvalue DAC 12 Bit 4 Ch, Voltage -10..+10 (V)
				if (abs( (float)Param) > 10.0f)
				{
					SerPrompt(ParamErr, 0, ucScriptMode);
					return;
				}
				else
				{
					Dac_Output(ucRegister, (float) Param);
					// this is an op-Value --> can be changed without WEN=1
					if (Status.EEUnlocked == TRUE)
					{
						Data.ram.f = &Params.DACValues[ucRegister].val;
						Data.type = PARAM_FLOAT;
						result = HandleSetParamWithEE((float)Param, Data);						
					}					
					SerPrompt(result, 0, ucScriptMode);
					return;
				}
				break;
			case 30 ... 33:				// Relais-Ports 0 bis 3, Ausgabewerte & Ports auf IO16-Bridge, Aus- und Eingabewerte
				RelayPort_Set(ucRegister, (uint8_t) Param);
				// this is an op-Value --> can be changed without WEN=1
				if (Status.EEUnlocked == TRUE)
				{
					Data.ram.b = &Params.RelayPortArray[ucRegister];
					Data.type = PARAM_BYTE;
					result = HandleSetParamWithEE((uint8_t)Param, Data);
				}
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 34 ... 35:				//  Ausgabewerte & Ports auf IO16-Bridge, Aus- und Eingabewerte
				// TODO check for PortIO attached to the uni-c
				ucTmp = (uint8_t) Param;
				if (SubCh == 34)
				{
					I2CRegister_Write(PORTIO_PCA9555D_ADDRESS<<1,PORTIO_PCA9555D_REGISTER_OUTPUT_PORT_0,1,&ucTmp);
				}
				else
				{
					I2CRegister_Write(PORTIO_PCA9555D_ADDRESS<<1,PORTIO_PCA9555D_REGISTER_OUTPUT_PORT_1,1,&ucTmp);
				}
				if (Status.EEUnlocked == TRUE)
				{
					SerPrompt(NotImplemetedErr, 0, ucScriptMode);
					Status.EEUnlocked = FALSE;
				}
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			case 44 ... 45:				// Ports auf IO16-Bridge, Datenrichtung (Bit = 1 -> Ausgang)
				// TODO check for PortIO attached to the uni-c
				ucTmp = (uint8_t) Param;
				if (SubCh == 44)
				{
					I2CRegister_Write(PORTIO_PCA9555D_ADDRESS<<1,PORTIO_PCA9555D_REGISTER_CONFIG_0,1,&ucTmp);
				}
				else
				{
					I2CRegister_Write(PORTIO_PCA9555D_ADDRESS<<1,PORTIO_PCA9555D_REGISTER_CONFIG_1,1,&ucTmp);
				}
				if (Status.EEUnlocked == TRUE)
				{
					SerPrompt(NotImplemetedErr, 0, ucScriptMode);
					Status.EEUnlocked = FALSE;
				}
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			case 60 ... 79:			// VAL SPI-Register 0 bis 19 im FPGA, sofern vorhanden
				// TODO TBR different handling to FPGA-Board FPGA_SetSPIregister( 0x80 );
				FPGA_SetSPIregister(  ((uint8_t)(SubCh - 60)) );
				FPGA_SetSPIvalue32( ((uint32_t)Param));
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			case 80: 
				Params.ucDisplayedMenu = (uint8_t) Param;
				break;
			case 90 ... 95:						// CLK (set RTC)
				Rtc_ReadChipTime();
				sreg = SREG;
				cli();
				switch (SubCh)
				{
					case 90:
						if (((uint8_t)Param <= 23)) gRTC.ucHours   = (uint8_t)Param;
						else ucError = 1;
						break;
					case 91:
						if (((uint8_t)Param <= 59)) gRTC.ucMinutes = (uint8_t)Param;
						else ucError = 1;
						break;
					case 92:
						if (((uint8_t)Param <= 59)) gRTC.ucSeconds = (uint8_t)Param;
						else ucError = 1;
						break;
					case 93:
						if (((uint8_t)Param >= 1) && ((uint8_t)Param <= 31)) gRTC.ucDay     = (uint8_t)Param;
						else ucError = 1;
						break;
					case 94:
						if (((uint8_t)Param >= 1) && ((uint8_t)Param <= 12)) gRTC.ucMonth   = (uint8_t)Param;
						else ucError = 1;
						break;
					case 95:
						if (((uint8_t)Param <= 99)) gRTC.ucYear    = (uint8_t)Param;
						else ucError = 1;
						break;
				}
				SREG = sreg;
				if (ucError)
				{
					SerPrompt(ParamErr, 0, ucScriptMode);
				}
				else
				{
					Rtc_SetChipTime();
					SerPrompt(NoErr, 0, ucScriptMode);
				}
				break;
			case 100 ... 107:				// Offsets ADC-Wert in ADC-Rohwerten
				Data.ram.i = &Params.ADCoffsets[ucRegister];
				Data.type = PARAM_INT;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 110 ... 117:				// Kanal-Skalierung AD-Wert; Messwert = (ADC-Rohwert + Offset) * Kanal-Skalierung * Basis-Skalierung
				Data.ram.f = &Params.ADCscales[ucRegister];
				Data.type = PARAM_FLOAT;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 120 ... 123:				// Offsets DAC-Werte in DAC-Rohwerten
				Data.ram.i = &Params.DACOffset[ucRegister];
				Data.type = PARAM_INT;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 130 ... 133:				// Kanal-Skalierungen DAC-Werte; Ausgabewert = (Kanal-Skalierung * Spannung / Basis-Skalierung) + Kanal-Offset
				Data.ram.f = &Params.DACscale[ucRegister];
				Data.type = PARAM_FLOAT;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 140:						// DAC-Basis-Skalierung; m�glichste nicht �ndern!
				Data.ram.f = &Params.DACBaseScale;
				Data.type = PARAM_FLOAT;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 141:						// ADC-Basis-Skalierung; m�glichste nicht �ndern!
				Data.ram.f = &Params.ADCbaseScale;
				Data.type = PARAM_FLOAT;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 239:
				Data.ram.b = &Params.ucI2CAddress;
				Data.type = PARAM_BYTE;
				result = HandleSetParamWithEE(Param, Data);
				SerPrompt(result, 0, ucScriptMode);
				break;
			case 252:
				if (Status.EEUnlocked == TRUE)
				{
					SerPrompt(NotImplemetedErr, 0, ucScriptMode);
					Status.EEUnlocked = FALSE;
				}
				SerPrompt(NotImplemetedErr, 0, ucScriptMode);
				break;
			case 260 ... 269:				// FWR 220 ... 229 for compatibility reasons.
				{
				uint8_t ucIndex = (uint8_t)Param;
	
				if (ucIndex >= sizeof(g_dScriptRegister)/sizeof(float) )
				{
					SerPrompt(ParamErr, 0, ucScriptMode);
					break;
				}
				CHECKPOINT;
				g_ucModuleBusy++;
				result = SD_write_FWR_FWV(Params.cp_FNA_SaveFileName, ucIndex, Param, TRUE); // Param-Value not used.
				if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	
				SerErrorPrompt(result, 0, ucScriptMode);
			
				}
				break;
			case 270 ... 279:				// FWV
				CHECKPOINT;
				g_ucModuleBusy++;
				result = SD_write_FWR_FWV(Params.cp_FNA_SaveFileName, SubCh - 270, Param, FALSE);
				if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	
				SerErrorPrompt(result, 0, ucScriptMode);
				break;
			case 299:						// DLY
				{
				xg_uiScriptDelay = (uint16_t)Param;
				uint16_t ui = 1;
				ATOMIC_RW(xg_uiScriptDLY, (uint16_t)Param);
	
				while(ui)
				{
					ATOMIC_RW(ui, xg_uiScriptDLY);
				}
				SerPrompt(NoErr, 0, ucScriptMode);
				}
				break;
			case 300 ... 309:				// Labscript REG0..9, ACC
				g_dScriptRegister[ucRegister] = Param;
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			case 310 ... 319:				// Labscript MOV REG0..9
				ucTmp = (uint8_t) Param;
				if (ucTmp >= 10)
				{
					SerPrompt(ParamErr, 0, ucScriptMode);
					break;
				}
				g_dScriptRegister[ucRegister] = g_dScriptRegister[ucTmp];
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			case 350 ... 359:			 // Labscript XCH REG0..9
				ucTmp = (uint8_t) Param;
				if (ucTmp >= 10)
				{
					SerPrompt(ParamErr, 0, ucScriptMode);
					break;
				}
				dTmp = g_dScriptRegister[ucRegister];
				g_dScriptRegister[ucRegister] = g_dScriptRegister[ucTmp];
				g_dScriptRegister[ucTmp] = dTmp;
	
				SerPrompt(NoErr, 0, ucScriptMode);
				break;
			default:
				SerPrompt(ParamErr, 0, ucScriptMode);
				break;
		} // switch
	}	// if (SubCh == 250)
	return;
}




