/*
 * Copyright (c) 2017 by Karsten Schmidt
 * Copyright (c) 2012-13 by Paul Schmid
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

 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"
#include "fat.h"
#include "parser.h"
#include "sd_commands.h"
#include "fpga.h"
#include "fpga_hw_config.h"
#include "panel.h"

#include "fpga_image.h"


/**
 * Set the register for SPI-operations 
 *
 * This function set the register for read-write SPI-operations. 
 *
 * \param[in] FPGA register number
 * \returns void
 * \see FPGA_ReadWriteSPIvalue32
 */
void FPGA_SetSPIregister(uint8_t ucReg)
{
    uint8_t spcr = SPCR;            // save SPI settings for SD card
    uint8_t spsr = SPSR;            // save SPI settings for SD card

    SPCR = FPGA_SPCR;               // preset constant SPI settings for FPGA
    SPSR &= ~(1 << SPI2X);          // No doubled clock frequency
	
    PORTA &= ~(1 << FPGA_BIT_SPI_RS);

    SPDR = ucReg;
    while (!(SPSR & (1<<SPIF)));

    PORTA |= (1 << FPGA_BIT_SPI_RS);

    SPCR = spcr;                    // Restore settings for SD mode
    SPSR = spsr;

}



/**
 * Write a value in an SPI-Register
 *
 * This function ....
 *
 * \param[inout] .....
 * \returns void
 * \see FPGA_SetSPIregister
 */
void FPGA_SetSPIvalue32(uint32_t ulValue)
{
    int8_t i;
    uint8_t* ucTemp = (uint8_t*) &ulValue;

    uint8_t spcr = SPCR;            // save SPI settings for SD card
    uint8_t spsr = SPSR;            // save SPI settings for SD card

    SPCR = FPGA_SPCR;               // preset constant SPI settings for FPGA
    SPSR &= ~(1 << SPI2X);          // No doubled clock frequency

    PORTA &= ~(1 << FPGA_BIT_SPI_DS);

    for (i=3; i>=0; i--)
    {
        SPDR = *(ucTemp + i);       // not portable
        while (!(SPSR & (1<<SPIF)));
    }

    PORTA |= (1 << FPGA_BIT_SPI_DS);

    SPCR = spcr;                    // Restore settings for SD mode
    SPSR = spsr;
}

uint32_t FPGA_ReadSPIvalue32(void)
{
	// Caution: Function returns the value read back from FPGA, so the input data is overwritten
	int8_t i;
	
	U32_4B rx;

	uint8_t spcr = SPCR;            // save SPI settings for SD card
	uint8_t spsr = SPSR;            // save SPI settings for SD card

	SPCR = FPGA_SPCR;               // preset constant SPI settings for FPGA
	SPSR &= ~(1 << SPI2X);          // No doubled clock frequency

	PORTA &= ~(1 << FPGA_BIT_SPI_DS);

	for (i=3; i>=0; i--)
	{
		SPDR = 0x00;
		while (!(SPSR & (1<<SPIF)));
		
		rx.u8[i]= SPDR;
	}
	

	PORTA |= (1 << FPGA_BIT_SPI_DS);

	SPCR = spcr;                    // Restore settings for SD mode
	SPSR = spsr;
	
	return (rx.u32);
}


/**
 * Write and read a value 
 *
 * This function ....
 *
 * \param[in] .....
 * \param[in] .....
 * \returns void
 * \see FPGA_SetSPIregister
 */
void FPGA_ReadWriteSPIvalue32(uint8_t *ucpValue, uint8_t ucDirection)
{
    // Caution: Function returns the value read back from FPGA, so the input data is overwritten
    int8_t i;

    uint8_t spcr = SPCR;            // save SPI settings for SD card
    uint8_t spsr = SPSR;            // save SPI settings for SD card

    SPCR = FPGA_SPCR;               // preset constant SPI settings for FPGA
    SPSR &= ~(1 << SPI2X);          // No doubled clock frequency

    PORTA &= ~(1 << FPGA_BIT_SPI_DS);

    if (ucDirection == NORMAL)	    // NORMAL is sequence of 4 Bytes
    {
        for (i=0; i<=3; i++)
        {
            SPDR = *(ucpValue + i);
            while (!(SPSR & (1<<SPIF)));

            *(ucpValue + i) = SPDR ;
        }
    }
    else						    // REVERSE is 4 Bytes representing actually a uint32_t
    {
        for (i=3; i>=0; i--)
        {
            SPDR = *(ucpValue + i);
            while (!(SPSR & (1<<SPIF)));

            *(ucpValue + i) = SPDR ;
        }
    }

    PORTA |= (1 << FPGA_BIT_SPI_DS);

    SPCR = spcr;                    // Restore settings for SD mode
    SPSR = spsr;
}



void CFG_Splash(uint8_t ucFlag)
{
    char s[10];

    if (ucFlag == TRUE)
    {
        sprintf_P(s, PSTR("Running "));
        Lcd_Write(0, 0, 8, s);
        sprintf_P(s, PSTR("  script"));
        Lcd_Write(0, 1, 8, s);
    }
    else
    {
        sprintf_P(s, PSTR("Loading "));
        Lcd_Write(0, 0, 8, s);
        sprintf_P(s, PSTR("     ..."));
        Lcd_Write(0, 1, 8, s);
    }
}

uint8_t CFG_LoadFile(char* ucp_filename, uint8_t ucMode __attribute__((unused)))
{
    static uint8_t ucFlag=FALSE;

    if (!g_ucSDpresent)
        return ERROR_NOCARD;

    // find file extension; precondition is that filename has no quotes and last char is 0
    uint8_t uc_dotindex = 0;
    uint8_t result = SUCCESS;
    uint8_t uc_count = 0;

    char c_extension[4];

    while (uc_count < strlen(ucp_filename))
    {
        if (ucp_filename[uc_count] == '.')
        {
            uc_dotindex = uc_count;
        }
        uc_count++;
    }

    if ((uc_dotindex == 0) || (strlen(ucp_filename)-uc_dotindex != 4))
    {
        return ERROR_PARAMETER;
    }

    uc_dotindex++; // point to first byte of extension rather than to the '.'

    for (uc_count=0; uc_count<4; uc_count++)
    {
        c_extension[uc_count] = tolower(ucp_filename[uc_dotindex+uc_count]);
    }

    // select function depending on file extension
    if( (strncmp_P(c_extension, PSTR("bin"), 3) == 0) ||
            (strncmp_P(c_extension, PSTR("bit"), 3) == 0) )
    {
        CFG_Splash(ucFlag);
        result = FPGA_LoadImageFromSD(ucp_filename);
    }
    else if ((strncmp_P(c_extension, PSTR("ini"), 3) == 0))
    {
        ucFlag = TRUE;
        CFG_Splash(ucFlag);
        result = jobParseINIscript(ucp_filename);
        ucFlag = FALSE;
    }
    else
	{
		CHECKPOINT;
	}
	/*
	if ((result == SUCCESS) && (ucMode == BUSMODE))
    {
        // after the file was loaded successfully, point the panel to the same file
        g_uiFilePtr = 0;
        fat_reset_dir(g_dd_panel);

        do
        {
            if (!fat_read_dir(g_dd_panel, &g_dir_entry_panel))
                break;
            g_uiFilePtr++;
        }
        while (strcmp(ucp_filename, g_dir_entry_panel.long_name));
    }
	g_ucFileMenuUpdate = 1;
	*/

    return result;
}

// ****************************************************************************
// Load FPGA with bit/bin data via configuration interface
// ****************************************************************************
uint8_t FPGA_LoadImageFromSD(char* ucp_filename)
{
    struct fat_file_struct* fd;

    if (!g_ucSDpresent)
	{
        return ERROR_NOCARD;
	}
    fd = open_file_in_dir(g_fs, g_dd, ucp_filename);

    if(!fd)
    {
        return ERROR_OPENFILE;
    }

    PORTA &= ~(1<<FPGA_PROG);    // Resetting FPGA
    _delay_us(1);                // minimum pulse for initialization of FPGA Tprog >=0.3ms

	PORTA |= (1<<FPGA_PROG);
    _delay_ms(5);                // maximum waiting time for the FPGA being ready for configuration Tpl <= 3ms

    // load FPGA = stream configuration file to FPGA
    uint16_t count;
    while((count = fat_read_file(fd, gucBuffer, sizeof(gucBuffer))) > 0)
    {
        FGPA_ConfigBuffer(count & 0xFF);
    }

    fat_close_file(fd);

    if ( !(PINA & FPGA_DONE) )  // Check FPGA Done signal.
    {
        LOG_INFO(PSTR("FPGA_UNEXPECTED_ENDOFFILE\n"));
		return ERROR_UNEXPECTED_ENDOFFILE;
    }
	LOG_INFO(PSTR("FPGA_SUCCESS\n"));

    PORTA |= (1 << FPGA_BIT_SPI_DS);
	
	return SUCCESS;
}
// ****************************************************************************
// Load FPGA with image from progmen
// ****************************************************************************
uint8_t FPGA_LoadImageFromProgmem(void)
{
#ifdef LOAD_FPGA_FROM_FLASH   
    uint16_t size;
    const char * pImage;
    size = _binary_FPGA_image_bit_end - _binary_FPGA_image_bit_start;
    pImage = _binary_FPGA_image_bit_start;

    LOG_DEBUG(PSTR("size: 0X%X\n"), size);
    LOG_DEBUG(PSTR("endpointer: %p\n"), _binary_FPGA_image_bit_end);
    LOG_DEBUG(PSTR("startpointer: %p\n"), _binary_FPGA_image_bit_start);
    LOG_DEBUG(PSTR("startpointer: %p\n"), pImage);

    PORTA &= ~(1<<FPGA_PROG);    // Resetting FPGA
    _delay_us(1);                // minimum pulse for initialization of FPGA T_{prog} >=0.3ms
    
	PORTA |= (1<<FPGA_PROG);
    _delay_ms(5);                // maximum waiting time for the FPGA being ready for configuration T_{pl} <= 3ms

    // load FPGA = stream configuration file to FPGA
    uint16_t count;
     
    while (pImage < _binary_FPGA_image_bit_end)
    {    
        if ((_binary_FPGA_image_bit_end - pImage) > sizeof(gucBuffer))
        {
            count = sizeof(gucBuffer);
        }
        else
        {
            count = _binary_FPGA_image_bit_end - pImage;
        }
        memcpy_P(gucBuffer, pImage, count);
        FGPA_ConfigBuffer(count & 0xFF);
        pImage += sizeof(gucBuffer);
    }
 
    if ( !(PINA & FPGA_DONE) )  // Check FPGA Done signal.
    {
        LOG_INFO(PSTR("FPGA_UNEXPECTED_ENDOFFILE\n"));
		return ERROR_UNEXPECTED_ENDOFFILE;
    }
	LOG_INFO(PSTR("FPGA_SUCCESS_FROM \n"));

    PORTA |= (1 << FPGA_BIT_SPI_DS);
    
    return SUCCESS;
#else
    return ERROR_FEATURE_NOT_ENABLED;
#endif

}

uint8_t g_ucINIreadBuffer[128];
char g_ucINIcommandBuffer[64];

uint8_t jobParseINIscript(char* ucp_filename)
{
    ERROR Error = NoErr;
    uint8_t i;
    uint32_t ulINIfileRead = 0;
    uint8_t ucLastButton = Lcd_GetButton() & BUTTON_ENTER;
    uint8_t ucButton;

    struct fat_file_struct* fd_ini;
	//CHECKPOINT;
    for (i=0 ; i < sizeof(g_ucLabelValid); i++)
        g_ucLabelValid[i] = FALSE;

    g_ulCurrentINIfilePos = 0;

    if (!g_ucSDpresent)
	{
        return ERROR_NOCARD;
	}

    fd_ini = open_file_in_dir(g_fs, g_dd_ini, ucp_filename);

    if(!fd_ini)
    {
        return ERROR_OPENFILE;
    }

    uint8_t gucINIptr = 0;
    uint8_t ucReadPtr = 0;
    char c;

    uint16_t count;
    while((count = fat_read_file(fd_ini, g_ucINIreadBuffer, sizeof(g_ucINIreadBuffer))) > 0)
    {
        uint8_t ucReadMax = (count-1) & 0xFF;

        while ( ucReadPtr <= ucReadMax )
        {
            c = g_ucINIreadBuffer[ucReadPtr++];

            if (c == '\n')
            {
                if (ucReadPtr == sizeof(g_ucINIreadBuffer))
                    break;
                else
                    continue;
            }

            if (c == '\r')
            {
                g_ucINIcommandBuffer[gucINIptr] = 0;

				LOG_INFO(PSTR("INIcommand:%s\n"), g_ucINIcommandBuffer);
                
                g_ulCurrentINIfilePos = ulINIfileRead + ucReadPtr; // point to next command (required if label)
                g_ucScriptMode = Params.ucDefaultChannel;
                ParseCommand(g_ucINIcommandBuffer, &Error, SCRIPTMODE); // There is no point in returning the "result" BREAK/CONTINUE
                g_ucScriptMode = FALSE;

                gucINIptr = 0;

                ucButton = Lcd_GetButton() & BUTTON_ENTER;
                if ((ucLastButton == 0) && (ucButton != 0 ))
                {
                    g_ucSearchLabel = FALSE;
                    fat_close_file(fd_ini);

                    return SUCCESS; // well, it was a USRBREAK
                }
                ucLastButton = ucButton;

                if (g_lIniFileSeekPos != 0)
                {
                    break;      // if a jump to a label is required, stop here ...
                }
            }
            else
            {
                g_ucINIcommandBuffer[gucINIptr++] = c;

                if (gucINIptr == sizeof(g_ucINIcommandBuffer))
                {
                    return ERROR_PARAMETER; // command buffer overflow
                }
            }

            if (ucReadPtr == sizeof(g_ucINIreadBuffer))
			{
                break;
			}
        }
        ulINIfileRead += count;
        ucReadPtr = 0;

        if (g_lIniFileSeekPos != 0)  // ... and parse the file for that label position before continuing
        {
            if(!fat_seek_file(fd_ini, &g_lIniFileSeekPos, FAT_SEEK_SET))
            {
                fat_close_file(fd_ini);
                return ERROR_SEEKFILE;
            }
            ulINIfileRead = g_lIniFileSeekPos;
            g_lIniFileSeekPos = 0;
        }
    }

    if (gucINIptr != 0) // last command without CR ?
    {
        g_ucINIcommandBuffer[gucINIptr] = 0;

		LOG_INFO(PSTR("INIcommand:%s\n"), g_ucINIcommandBuffer);

        g_ucScriptMode = Params.ucDefaultChannel;
        ParseCommand(g_ucINIcommandBuffer, &Error, SCRIPTMODE); // There is no point in returning the "result" BREAK/CONTINUE
        g_ucScriptMode = FALSE;
    }
    g_ucSearchLabel = FALSE; // in case there was a forward label that was not found
    fat_close_file(fd_ini);
    return SUCCESS;
}

