/*
 * based on work by
 * Copyright (c) 2007 by Hartmut Birr
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
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <util/delay.h>


#include "Uart.h"
#include "parser.h"
#include "fpga.h"
#include "Timer.h"
#include "helper_macros.h"

#include "debug.h"

const PROGMEM char ErrStr[][9]=
{
    "[OK]",
    "[SRQUSR]",
    "[BUSY]",
    "[OVERLD]",
    "[CMDERR]",
    "[PARERR]",
    "[LOCKED]",
    "[CHKSUM]",
    "[FUSE]",
    "[FAULT]",
    "[OVRFLW]",
    "[I2CERR]",
    "[NOFILE]",
    "[NOCARD]",
	"[NOIMPL]"
};

char g_cSerInpStr[64];
uint8_t SerInpCount;
uint8_t g_ucScriptMode = FALSE;

uint8_t CurrentCh = 255;
struct
{
    uint8_t Verbose : 1;
} ParserFlags;


//---------------------------------------------------------------------------------------------
void SerPrompt(ERROR Err, uint8_t Status, uint8_t ucScriptMode)
{
    char ucResponse[40];

    if (ParserFlags.Verbose || Err != NoErr)
    {
        sprintf_P(ucResponse, PSTR("#%d:255=%d %S%c"), g_ucSlaveCh, Err + Status, ErrStr[Err], (ucScriptMode==SCRIPTMODE) ? '\r': '\n');  // capital "%S" to print string from PROGMEM

        if (ucScriptMode == SCRIPTMODE)
        {
            // CHECKPOINT;
			// TODO das ist komisch, was bewirkt das
			// FPGA_SendStringSPI(ucResponse,strlen(ucResponse));
            _delay_ms(5);
        }
        else
        {
            printf_P(PSTR("%s"), ucResponse);
        }
    }
}


//---------------------------------------------------------------------------------------------
void SerErrorPrompt(uint8_t result, uint8_t Status, uint8_t ucScriptMode)
{
    // merge error codes from SD functions to c't lab error codes
    if (result == SUCCESS)          result = NoErr;
    if (result == ERROR_OPENFILE)   result = NotFoundErr;
    if (result == ERROR_NOCARD)     result = NoCardErr;
    if (result == ERROR_PARAMETER)  result = ParamErr;

    switch (result)
    {
        case NoErr:
        SerPrompt(NoErr, Status, ucScriptMode);
        break;

        case LockedErr:
        case ParamErr:
        case NotFoundErr:
        case NoCardErr:
        SerPrompt(result, 0, ucScriptMode);
        break;

        default:
        SerPrompt(FaultErr, 0, ucScriptMode);
        break;
    }
}


//---------------------------------------------------------------------------------------------
void SerStr(char* str, uint8_t ucScriptMode)
{
    if (ucScriptMode == SCRIPTMODE)
    {
        CHECKPOINT;
		// FPGA_SendStringSPI(str,strlen(str));
        // FPGA_SendCharSPI('\r');
        _delay_ms(5);
    }
    else
    {
        printf_P(PSTR("%s\n"), str);
    }
}


//---------------------------------------------------------------------------------------------
void SerialClearBuffer(const uint16_t ucWait)
{
    uint8_t count;
    uint8_t c;
    uint16_t u;

    ATOMIC_RW(xg_uXmodemWaitTimer, ucWait);

    do
    {
        count = Uart_GetRxCount();

        while (count--)
        {
            Uart_GetRxData(&c, 1);
            ATOMIC_RW(xg_uXmodemWaitTimer, ucWait);
        }

        ATOMIC_RW(u, xg_uXmodemWaitTimer);

    }
    while (u);

}


uint8_t SerialReceive(uint8_t* cPtr, const uint16_t ucWait)
{
    uint16_t u;

    ATOMIC_RW(xg_uXmodemWaitTimer, ucWait);

    do
    {
        if (Uart_GetRxCount() != 0 )
        {
            Uart_GetRxData(cPtr, 1);
            return 1;
        }

        ATOMIC_RW(u, xg_uXmodemWaitTimer);
    }
    while (u);

    return 0;
}


//---------------------------------------------------------------------------------------------
void jobParseData(void)
{
    uint8_t count;
    static char c = 0;
    ERROR Error = NoErr;
    static ERROR OvflError = NoErr;

    uint8_t result;
    char prev;

    count = Uart_GetRxCount();

    // maximum in chunks of 20 Bytes, in order not to stay too long here
    if (count > 20)
    {
        count = 20;
    }

    // if an OvrflError has occurred, read in following buffer content until \n or \r to clear overrun buffer garbage
    if (OvflError == OvrflErr)
    {
        while (count--)
        {
            Uart_GetRxData((uint8_t*)&c, 1);
            if  ((c == '\r') || (c == '\n'))
            {
                OvflError = NoErr;
                break;
            }
        }
        return;
    }

    while (count--)
    {
        prev = c;
        Uart_GetRxData((uint8_t*)&c, 1);

        if (c >= ' ' && (uint8_t)c <= 127)
        {
            g_cSerInpStr[SerInpCount++] = c;

            if (SerInpCount >= sizeof(g_cSerInpStr))
            {
                // buffer overrun, last character in buffer is not \n, \r, \b
                SerInpCount = 0;
                CHECKPOINT;
                OvflError = OvrflErr;
                SerPrompt(OvflError, 0, BUSMODE);
                return;
            }
        }
        else if (c == '\b')
        {
            if (SerInpCount)
            {
                SerInpCount--;
            }
        }

        else if (c == '\n' && prev == '\r')
        {
            continue;
        }

        else if (c == '\r' || c == '\n')
        {
            // after \r or \n, we consider a new command complete

            // Zero-Terminate String in Buffer
            g_cSerInpStr[SerInpCount] = 0;

            result = ParseCommand(g_cSerInpStr, &Error, BUSMODE);
            if (result == BREAK)
            {
                break;
            }
            else
            {
                continue;
            }
        }
    }
    if (Error != NoErr)
    {
        SerPrompt(Error, 0, BUSMODE);
    }
}

/*
void jobParseFPGACommand(void)
{
    ERROR Error = NoErr;
    static char coreBuffer[64];                                 // we can't use gucBuffer here, which must survive this function
    static uint8_t coreBufferPtr = 0;
    uint8_t coreTmp;

    while  (gucIRQwBufferPtr != gucIRQrBufferPtr)
    {
        coreTmp = gucIRQBuffer[gucIRQrBufferPtr++];
        if ( ( coreTmp >= 10) && (coreTmp <= 127))
        {
            coreBuffer[coreBufferPtr++] = coreTmp;
        }

        if ((coreTmp == 0x08) && (coreBufferPtr != 0))          // handle Backspace
        {
            coreBufferPtr--;
        }
        else if (coreTmp == CR)
        {
            coreBuffer[coreBufferPtr] = 0;

			LOG_INFO(PSTR("FPGAcommand: %s\n"), coreBuffer);

            if (coreBuffer[0] == CR)                            // handle "CR" only commands from Core
            {
                FPGA_SetSPIregister(Params.ucSPIcoreTxAVRtoFPGA);
                FPGA_SetSPIvalue8(ACK);
                coreBufferPtr = 0;
                continue;
            }

            g_ucCoreCommand = TRUE;

            FPGA_SetSPIregister(Params.ucSPIcoreTxAVRtoFPGA);   // Register Core-Select required before ParseCommand
            g_ucScriptMode = TRUE;                              // This is always the FPGA
            ParseCommand(coreBuffer, &Error, SCRIPTMODE);       // There is no point in returning the "result" BREAK/CONTINUE to the ISR
            g_ucScriptMode = FALSE;
            FPGA_SetSPIregister(Params.ucSPIcoreTxAVRtoFPGA);   // restore, just in case
            FPGA_SetSPIvalue8(ACK);                             // Select Core 0 ACK

            coreBufferPtr = 0;
            g_ucCoreCommand = FALSE;
        }
    }
}
*/

uint8_t ParseCommand(char* s, ERROR* pError, uint8_t ucScriptMode)
{
    uint16_t SubCh;
    *pError = NoErr;

    char* pos;
    uint16_t value;
    float Param;
    uint8_t result;

    SerInpCount = 0;

    while (*s == ' ') s++;
    // now check for various options
    if (*s == 0)
    {
        // empty string, just skip, don't forward
        SerPrompt(NoErr, 0, ucScriptMode);
        return BREAK;
        // break;
    }


#ifdef STRICTSYNTAX
    if (!ucScriptMode)	// In regular bus operation ...
    {
        if (*s == '#')  // ... don't forward incomplete or invalid messages
        {
            s++;
            while (*s == ' ') s++;

            if ( (*s >= '0') && (*s <= '9') )
            {
                char cMCH = *s - '0';
                s++;
                while (*s == ' ') s++;

                if (*s == ':')
                {
                    // Syntax "#x:result /r/n" is OK

                    if ((g_cWaitForResponse) && (cMCH == g_ucScriptMCH))
                    {
                        s++;
                        while (*s == ' ') s++;

                        if (*s >= '0' && *s <= '9')
                        {
                            // Direkter SubCh-Aufruf
                            value = 0;
                            while (*s >= '0' && *s <= '9' && value <= 9999)
                            {
                                value *= 10;
                                value += *s - '0';
                                s++;
                            }

                            if ((value > 9999) || (value != g_ucScriptSCH))
                            {
                                *pError = NoErr;       // don't report an error for a shortened result
                                return CONTINUE;       // go on and handle next characters
                            }

                            while (*s != '=')
                            {
                                s++;
                            }
                            s++;

                            g_dWaitResponse = atof(s);
                            g_cWaitForResponse = FALSE;
                        }
                    }
                    else
                    {
                        SerStr(g_cSerInpStr, ucScriptMode);  // forward the result (as it is)
                    }

                    *pError = NoErr;       // don't report an error for a shortened result
                    return CONTINUE;       // go on and handle next characters
                }
            }

            *pError = SyntaxErr;
            return BREAK;
        }
    }
#else
    if (*s == '#')
    {
        if (g_cWaitForResponse)
        {
            s++;
            while (*s == ' ') s++;

            if ( (*s >= '0') && (*s <= '9') )
            {
                char cMCH = *s - '0';
                s++;
                while (*s == ' ') s++;

                if ((*s == ':')   && (cMCH == g_ucScriptMCH))
                {
                    s++;
                    while (*s == ' ') s++;

                    if (*s >= '0' && *s <= '9')
                    {
                        // Direkter SubCh-Aufruf
                        value = 0;
                        while (*s >= '0' && *s <= '9' && value <= 9999)
                        {
                            value *= 10;
                            value += *s - '0';
                            s++;
                        }

                        if ((value > 9999) || (value != g_ucScriptSCH))
                        {
                            *pError = NoErr;       // don't report an error for a shortened result
                            return CONTINUE;       // go on and handle next characters
                        }

                        while (*s != '=')
                        {
                            s++;
                        }
                        s++;

                        g_dWaitResponse = atof(s);
                        g_cWaitForResponse = FALSE;
                    }
                }
            }
        }
        else
        {
            SerStr(g_cSerInpStr, ucScriptMode);  // forward the result (as it is)
        }
        *pError = NoErr;       // don't report an error for a shortened result
        return CONTINUE;       // go on and handle next characters
    }

#endif // STRICTSYNTAX

    if (*pError != NoErr)
    {
        CHECKPOINT;
        return BREAK;
    }
    pos = strchr(s, ':');
    if (pos)
    {
        // address is checked here, if present
        if (*s == '*')
        {
            // Omni-Befehl, Weiterleiten (and process locally later, too)
            SerStr(g_cSerInpStr, ucScriptMode);
            s++;
            while (*s == ' ') s++;
            if (s != pos)
            {
                CHECKPOINT;
                *pError = SyntaxErr;
                return BREAK;
            }
        }
        else
        {
            value = 0;
            if (*s >= '0' && *s <= '9')     // actually, address can only be between 0...7, with 9 = FPGA
            {
                value += *s - '0';
                s++;
            }
            else
            {
                CHECKPOINT;
                *pError = SyntaxErr;
                return BREAK;
            }
            // check, if only spaces are between single digit address and colon ':'
            while (*s == ' ') s++;
            if (s != pos)
            {
                CHECKPOINT;
                *pError = SyntaxErr;
                return BREAK;
            }
            CurrentCh = value;
            if ((CurrentCh != g_ucSlaveCh) && (CurrentCh != 9)) // check for FPGA internal command
            {
                // Nicht für uns, weiterleiten
                SerStr(g_cSerInpStr, ucScriptMode);
                return CONTINUE; // go on and handle next characters
            }
        }
        s = pos + 1;
        while (*s == ' ') s++;
        if (*s == 0)
        {
            CHECKPOINT;
            *pError = SyntaxErr;
            return BREAK;
        }
    } // if (pos) ':'

#ifdef STRICTSYNTAX
    else if (!ucScriptMode)	// In regular bus operation ...
    {
        // no (pos) ':'
        // no address designator found --> incorrect syntax
        *pError = SyntaxErr;
        return BREAK;
    }
#endif

    if ( (ucScriptMode == BUSMODE) && ( (g_ucModuleBusy > 0) || (g_ucScriptMode) || (g_ucCoreCommand) ))
        // deny handling of >bus< commands for this module when
    {
                                                                                // module is busy by panel or running lengthy commands
        printf("Help %d, %d, %d, %d \n", ucScriptMode, g_ucModuleBusy, g_ucScriptMode, g_ucCoreCommand);                                                            // or in script mode or FPGA command anyway
        CHECKPOINT;
		*pError = BusyErr;
        return BREAK;
    }

    // Ist für uns, ab hier eigentliche Verarbeitung
    ParserFlags.Verbose = strchr(s, '!') != NULL || strchr(s, '?') != NULL;

    pos = strchr(s, '$');
    if (pos && isxdigit(pos[1]))
    {
        uint8_t Sum = 0;
        uint8_t calcSum = 0;
        uint8_t i;
        char* ss = g_cSerInpStr;

        // calculate the checksum from command data
        while (ss < pos)
        {
            calcSum ^= *ss++;
        }

        i = isxdigit(pos[2]) ? 2 : 1;

        // retrieve checksum from command after '$'
        while (i)
        {
            i--;
            pos++;
            Sum *= 16;
            if (isdigit(*pos))
            {
                Sum += *pos - '0';
            }
            else
            {
                Sum += toupper(*pos) - 'A' + 10;
            }
        }

        // check if calculated and command checksum are the same
        if (Sum != calcSum)
        {
            *pError = ChecksumErr;

            if (ucScriptMode == BUSMODE)
                printf_P(PSTR("#%d:Command was \"%s\" CS:%02x\n"), g_ucSlaveCh, g_cSerInpStr, calcSum);

            g_ucErrCount++;
            return BREAK;
        }
    }

    if (*s >= '0' && *s <= '9')
    {
        // Direkter SubCh-Aufruf
        value = 0;
        while (*s >= '0' && *s <= '9' && value <= 9999)
        {
            value *= 10;
            value += *s - '0';
            s++;
        }
        if (value > 9999)
        {
            *pError = SyntaxErr;
            return BREAK;
        }
        SubCh = value;
    }
    else
    {
        // Klartext übersetzen to SubCh-Code
        char cmd[4];
        uint8_t i;
        uint8_t str ;
        uint16_t offset = 0;


        cmd[3] = 0;
        for (i = 0; i < 3; i++, s++)
        {
            if (*s >= 'A' && *s <= 'Z')
            {
                cmd[i] = *s + 'a' - 'A';
            }
            else if (*s >= 'a' && *s <= 'z')
            {
                cmd[i] = *s;
            }
            else
            {
                return BREAK;
            }
        }
        if (i < 3)
        {
            CHECKPOINT;
            *pError = SyntaxErr;
            return BREAK;
        }
        value = 0;
        while (*s == ' ') s++;
        if (*s >= '0' && *s <= '9')
        {
            while (*s >= '0' && *s <= '9' && value <= 255)  // subchannel after 3-char-code less than 255?
            {
                value *= 10;
                value += *s - '0';
                s++;
            }
            if (value > 255)	// subchannel after 3-char-code less than 255?
            {
                CHECKPOINT;
                *pError = SyntaxErr;
                return BREAK;
            }

        }

        i=0;
        while(1)
        {
            str = pgm_read_byte(&Mnemonics[i][MNEM_STR]);

            if (str == 0)
                break;


            if ( 0 == memcmp_P( cmd, &Mnemonics[i][MNEM_STR], 3) )
            {
                offset = pgm_read_byte(&Mnemonics[i][MNEM_VAL1])*100 + pgm_read_byte(&Mnemonics[i][MNEM_VAL2]);
                break;
            }
            i++;
        }

        if ( str == 0 )
        {
            CHECKPOINT;
            *pError = SyntaxErr;
            return BREAK;
        }
        if (i == 0)
        {
            // nop
            CHECKPOINT;
            SerPrompt(NoErr, 0, ucScriptMode);
            return BREAK;
        }

        if (value + offset > 9999)
        {
            CHECKPOINT;
            *pError = SyntaxErr;
            return BREAK;
        }

        SubCh = value + offset;
    }
    while (*s == ' ') s++;
    if (*s == '=')
    {
        s++;
        while (*s == ' ') s++;

        // ************************** commands with >>filename<< **************************

        if (( SubCh == 210 ) ||                         //  OPT10
            ( SubCh == 211 ) ||                         //  OPT11
            ( SubCh == 240 ) ||                         //  CFG
            ( SubCh == 243 ) ||                         //  FNA
            ( SubCh == 244 ) ||                         //  FDL
            ( SubCh == 245 ) ||                         //  CDR (new)
            ( SubCh == 249 ) )                          //  FQU
         {
            int16_t i=0;
            char FilenameBuffer[FILENAMEBUFFERSIZE];         // filenames in the library used for SD access are max. 31 characters
            FilenameBuffer[0]=0;
            uint16_t uiFileNumber = 0xFFFF;

            // Options for file commands:
            // CFG=9                // file number
            // CFG=BINDATEI.DAT	    // string without quotation marks (first char MUST NOT be a number)
            // CFG="BINDATEI.DAT"   // any string with quotation marks (first char may be a number, too)

            if ( (*s >= '0' && *s <= '9') )
            {
                uiFileNumber = 0;

                while ( *s != 0x00 ) // handling of file number
                {
                    if ( (*s == '!') || (*s == '\r') )
                    {
                        break;
                    }
                    else if ( (*s  < '0')  || (*s > '9') )
                    {
                        SerPrompt(ParamErr, 0, ucScriptMode);
                        return BREAK;
                    }

                    uiFileNumber = uiFileNumber*10 + (*s - '0');
                    s++;
                }
            }
            else
            {
                if (*s == '"')	// get rid of leading quote, if any
                {
                    s++;
                }
                // search for end of string, if a quote is found within the string, consider it to be the end of the string
                while ( *(s+i) != 0x00 )
                {
                    if ( (*(s+i) == '"') || (*(s+i) == '!') || (*(s+i) == '\r') )
                    {
                        *(s+i) = 0x00;
                        break;
                    }
                    i++;
                }

                LOG_INFO(PSTR("detected filename: >%s< \n"),  s);

                // filename must be copied to a safe place.
                // When next bus commands are processed in the interrupt, the original memory space of the filename is overwritten
                memcpy(FilenameBuffer, s, MIN( strlen(s)+1, FILENAMEBUFFERSIZE ) );
            }

            result = ParseFileParam(SubCh, FilenameBuffer, uiFileNumber, ucScriptMode);

            LOG_INFO(PSTR("error code: %d\n"), result);

            SerErrorPrompt(result, Status.u8, ucScriptMode);
/*
            if (result == SUCCESS)          result = NoErr;
            if (result == ERROR_OPENFILE)   result = NotFoundErr;
            if (result == ERROR_NOCARD)     result = NoCardErr;
            if (result == ERROR_PARAMETER)  result = ParamErr;

            switch (result)
            {
                case NoErr:
                    SerPrompt(NoErr, Status.u8, ucScriptMode);
                break;

                case LockedErr:
                case ParamErr:
                case NotFoundErr:
                case NoCardErr:
                    SerPrompt(result, 0, ucScriptMode);
                    break;

                default:
                    SerPrompt(FaultErr, 0, ucScriptMode);
                    break;
            }
*/
        }

        else //  ************************** commands with >>string<< **************************

            if ( ((SubCh >= 900) && (SubCh <= 969)) || (SubCh == 980) )  //  TSR 900..969 || TSS 890
            {
                int16_t i=0;

                if (*s == '"')  // get rid of leading quote, if any
                {
                    s++;
                }
                // search for end of string, if a quote is found within the string, consider it the end of the string
                while ( *(s+i) != 0x00 )
                {
                    if ( (*(s+i) == '"') || (*(s+i) == '!') || (*(s+i) == '\r'))
                    {
                        *(s+i) = 0x00;
                        break;
                    }
                    i++;
                }

//		            printf_P(PSTR("detected string: %s \n"), s);

                if (SubCh == 980) // TSS
                    printf_P(PSTR("%s\n"), s);
                else if (ParseStringParam(SubCh, s ))
                    SerPrompt(ParamErr, 0, ucScriptMode);
                else
                    SerPrompt(NoErr, Status.u8, ucScriptMode);

            }
            else
            {
                if (!(*s >= '0' && *s <= '9') && *s != '-')
                {
                    CHECKPOINT;
                    *pError = ParamErr;
                    return BREAK;
                }
                Param = atof(s);
                ParseSetParam(SubCh, Param, ucScriptMode);
            }
    }
    else
    {
        ParseGetParam(SubCh, ucScriptMode);
    }
    ActivityTimer_SetValue(50);

    return CONTINUE;
}