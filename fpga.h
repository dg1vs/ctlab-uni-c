/*
 * Copyright (c) 2012 by Paul Schmid
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licen
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


#ifndef _FPGA_H_
#define _FPGA_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif




#define ENABLE_INT2  1
#define DISABLE_INT2 0
#define FALLINGSLOPE 0
#define RISINGSLOPE  1

#define WRITE 0
#define READ  1

//----------------------------------------------------------------------------
// LEDs
//----------------------------------------------------------------------------
#define ACTIVITY_LED    (1<<PD2)
#define UNUSED_LED      (1<<PD3)

#define SetLED(ucLED, ucState)  if(ucState) PORTD &= ~ucLED; else  PORTD |= ucLED;


//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

/* initialize SPI for FPGA communication */

#define FPGA_SPCR   ((0 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (0 << SPR1) | (1 << SPR0))

/* Clock Polarity: SCK high when idle */
/* Clock Phase: sample on trailing SCK edge */
/* Clock Frequency: f_OSC / 4 */


#define CR 	0x0d
#define LF 	0x0a
#define ACK 0x06

#define SPI_FPGA_OFFSET_DATA_AUTOINCR      0   // default 128
#define SPI_FPGA_OFFSET_CORE_SELECTRESET   1   // default 129; write access is RESET, too.
#define SPI_FPGA_OFFSET_STARTADDRESS_WRITE 2   // default 130
#define SPI_FPGA_OFFSET_STARTADDRESS_READ  3   // default 131

/* error codes */

#define SUCCESS                     100 // NoErr
#define ERROR_OPENFILE              101 // NotFoundErr
#define ERROR_UNEXPECTED_ENDOFFILE  102 // FaultErr

#define ERROR_CREATEFILE            104 // FaultErr
#define ERROR_DELETEFILE            105 // FaultErr
#define ERROR_SEEKFILE              106 // FaultErr
#define ERROR_WRITEFILE             107 // FaultErr
#define ERROR_CREATEDIRECTORY       108 // FaultErr

#define ERROR_NOCARD                109 // NoCardErr

#define ERROR_OPENPARTITION         110 // FaultErr
#define ERROR_OPENFILESYSTEM        111 // FaultErr
#define ERROR_OPENDIRECTORY         112 // FaultErr

#define ERROR_SYNCDISK              115 // FaultErr
#define ERROR_SDSLOTACCESS          117 // FaultErr

#define ERROR_PARAMETER             120 // ParamErr


#define ERROR_FEATURE_NOT_ENABLED   150 // ParamErr


//----------------------------------------------------------------------------
// prototypes
//----------------------------------------------------------------------------
// Aassembler function ses fpga_hw.s
extern void FGPA_ConfigBuffer(uint8_t ucCount);

uint8_t FPGA_LoadImageFromSD(char* ucp_filename);
uint8_t FPGA_LoadImageFromProgmem(void);

void FPGA_SetSPIregister(uint8_t ucValue);
void FPGA_SetSPIvalue32(uint32_t ulValue);
void FPGA_ReadWriteSPIvalue32(uint8_t *ucpValue, uint8_t ucDirection);
uint32_t FPGA_ReadSPIvalue32(void);

uint8_t CFG_LoadFile(char* ucp_filename, uint8_t ucMode);
uint8_t jobParseINIscript(char* ucp_filename);

#ifdef __cplusplus
}
#endif

#endif /* _FPGA_H_ */