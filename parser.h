/*
 * Copyright (c) 2007 by Hartmut Birr
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
 *
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char* Mnemonic;
    uint8_t dOffset;
} MNEMONIC;

typedef enum
{
    NoErr = 0,
    UserReq,
    BusyErr,
    OvlErr,
    SyntaxErr,
    ParamErr,
    LockedErr,
    ChecksumErr,
    FuseErr,
    FaultErr,
    OvrflErr,
    I2CErr,
    NotFoundErr,
    NoCardErr,
	NotImplemetedErr
} ERROR;

#define MNEM_STR 0
#define MNEM_VAL1 3
#define MNEM_VAL2 4

extern const PROGMEM char Mnemonics [][5];

void SerPrompt(ERROR, uint8_t, uint8_t ucScriptMode);
void SerStr(char*, uint8_t ucScriptMode);
void SerErrorPrompt(uint8_t result, uint8_t Status, uint8_t ucScriptMode);

void jobParseData(void);

void ParseGetParam(uint16_t SubCh, uint8_t ucScriptMode);
void ParseSetParam(uint16_t SubCh, float Param, uint8_t ucScriptMode);

void ActivityTimer_SetValue(uint8_t);

uint8_t ParseStringParam(uint16_t SubCh, char* cp);
uint8_t ParseFileParam(uint16_t SubCh, char* cp, uint16_t uiFileNumber, uint8_t ucScriptMode);
void ScriptWait(uint16_t SubCh);

uint8_t ParseCommand(char* s, ERROR* pError, uint8_t ucScriptMode);

extern uint8_t g_ucScriptMode;

#define BUSMODE 0
#define SCRIPTMODE 1
#define PANELMODE 2

#define BREAK 42
#define CONTINUE 43

void jobParseFPGACommand(void);
void SerialClearBuffer(const uint16_t ucWait);
uint8_t SerialReceive(uint8_t* cPtr, const uint16_t ucWait);

#ifdef __cplusplus
}
#endif

#endif /* _PARSER_H_ */
