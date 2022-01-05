/*
* Copyright (c) 2012-13 by Paul Schmid
*
* partially based on SD library
* Copyright (c) by Roland Riegel
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

#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "main.h"
#include "fat.h"
#include "partition.h"
#include "sd_raw.h"
#include "debug.h"
#include "sd_commands.h"

#include "fpga.h"
#include "Rtc.h"
#include "Uart.h"

struct fat_fs_struct*  g_fs;        // handle for SD file system
struct fat_dir_struct* g_dd;        // handle for general file access (dd #1)
struct fat_dir_struct* g_dd_ini;    // handle for ini script (dd #2). An ini script may contain commands that require access to another file.
struct fat_dir_struct* g_dd_panel;  // handle for panel access (dd #3). Panel access shall have no influence to the other file paths. ("fat_config.h" FAT_FILE_COUNT 3)

struct fat_dir_entry_struct g_directory;
struct partition_struct* g_partition;


// ****************************************************************************
// Reset SD and load parameters
// ****************************************************************************
uint8_t SD_init(void)
{
	// CHECKPOINT;
	g_ucModuleBusy++;

	// setup sd card slot
	if(!sd_raw_init())
	{
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
		return ERROR_SDSLOTACCESS;  // error accessing card slot
	}

	// open first partition
	g_partition = partition_open(	sd_raw_read,
	sd_raw_read_interval,
	#if SD_RAW_WRITE_SUPPORT
	sd_raw_write,
	sd_raw_write_interval,
	#else
	0,
	0,
	#endif
	0
	);

	if(!g_partition)
	{
		// If the partition did not open, assume the storage device
		// is a "superfloppy", i.e. has no MBR.

		g_partition = partition_open(   sd_raw_read,
		sd_raw_read_interval,
		#if SD_RAW_WRITE_SUPPORT
		sd_raw_write,
		sd_raw_write_interval,
		#else
		0,
		0,
		#endif
		-1
		);

		if(!g_partition)
		{
			if (g_ucModuleBusy > 0) g_ucModuleBusy--;
			return ERROR_OPENPARTITION; // error opening partition
		}
	}

	// open file system
	g_fs = fat_open(g_partition);

	if(!g_fs)
	{
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
		return ERROR_OPENFILESYSTEM;    // error opening filesystem
	}

	// open root directory
	fat_get_dir_entry_of_path(g_fs, "/", &g_directory);
	g_dd = fat_open_dir(g_fs, &g_directory);

	if(!g_dd)
	{
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
		return ERROR_OPENDIRECTORY;     // error opening root directory
	}

	g_dd_ini = fat_open_dir(g_fs, &g_directory);

	if(!g_dd_ini)
	{
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
		return ERROR_OPENDIRECTORY;     // error opening root directory
	}

	g_dd_panel = fat_open_dir(g_fs, &g_directory);

	if(!g_dd_panel)
	{
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;
		return ERROR_OPENDIRECTORY;     // error opening root directory
	}

	// print some card information as a boot message
	// print_disk_info(fs);

	if (g_ucModuleBusy > 0) g_ucModuleBusy--;
	return 0;
}

// ****************************************************************************
// Close active access to SD
// ****************************************************************************
void SD_close(void)
{
	CHECKPOINT;
	g_ucModuleBusy++;

	// close directory
	fat_close_dir(g_dd_panel);

	// close directory
	fat_close_dir(g_dd_ini);

	// close directory
	fat_close_dir(g_dd);

	// close file system
	fat_close(g_fs);

	// close partition
	partition_close(g_partition);

	if (g_ucModuleBusy > 0) g_ucModuleBusy--;

}

uint8_t SD_IsCardPresent(void)
{
	uint8_t ucPresent;              // interim storage for presence of SD card
	uint8_t ucResult;               // interim storage for SD_init result
	static uint8_t counter = 1;     // delay for next try to (re-)initialize the SD card. First time during boot shall be at once -> 1


	ucPresent = sd_raw_available();

	if ( ((!ucPresent) && (g_ucSDpresent)) || (g_ucSDdisable))
	{
		LOG_INFO(PSTR("deactivating SD\n"));
		CHECKPOINT;
		g_ucModuleBusy++;
		SD_close();
		if (g_ucModuleBusy > 0) g_ucModuleBusy--;

		g_ucSDpresent = FALSE;
		g_ucSDdisable = FALSE;
		counter = 0;
	}
	else if ((ucPresent) && (!g_ucSDpresent))
	{
		counter--;
		if (!counter)
		{
			// CHECKPOINT;
			g_ucModuleBusy++;
			ucResult = SD_init();
			if (g_ucModuleBusy > 0) g_ucModuleBusy--;

			LOG_INFO(PSTR("activating SD: %d\n"), ucResult);

			if (!ucResult)
			{
				g_ucSDpresent = TRUE;
			}
		}
	}
	return g_ucSDpresent;
}


// ****************************************************************************
// SD_ls
// ****************************************************************************
uint8_t SD_ls(uint8_t ucFlag)
{
	//char response[20];
	uint16_t i= 0;
	uint8_t ucIsDir;

	uint8_t uc_tmp;

	if (!g_ucSDpresent)
	{
		return ERROR_NOCARD;
	}
	// print directory listing
	struct fat_dir_entry_struct dir_entry;
	while(fat_read_dir(g_dd, &dir_entry))
	{
		ucIsDir = (dir_entry.attributes & FAT_ATTRIB_DIR);

		if (g_ucCoreCommand == 0)   // => output to console
		{
			_delay_ms(10);          // trade-off: wait some time to allow for a very busy bus
			while (Uart_GetRxCount() > 10) _delay_ms(10); // make sure that the receiving buffer is almost empty to avoid data loss while sending.

			uc_tmp = g_ucModuleBusy;
			g_ucModuleBusy = 0;

			if (ucFlag)
			{
				printf_P(PSTR("#%d:241=%u %c%s%c   %ld\n"), g_ucSlaveCh, i++, ucIsDir ? '<':'[', dir_entry.long_name, ucIsDir ? '>':']', dir_entry.file_size);
			}
			else
			{
				printf_P(PSTR("#%d:241=%u %c%s%c\n"), g_ucSlaveCh, i++, ucIsDir ? '<':'[', dir_entry.long_name, ucIsDir ? '>':']');
			}
			g_ucModuleBusy = uc_tmp;
		}
		else  // => output to FPGA
		{
			LOG_ERROR(PSTR("SOMETHING Wrong please check\n"));
		}
	}

	return 0;
}

// ****************************************************************************
// SD_filenum
// ****************************************************************************
uint8_t SD_filenum(void)
{
	//char response[20];
	uint16_t i= 0;
	// uint8_t ucIsDir;

	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	// print directory listing
	struct fat_dir_entry_struct dir_entry;
	while(fat_read_dir(g_dd, &dir_entry))
	{
		// ucIsDir = (dir_entry.attributes & FAT_ATTRIB_DIR);
		i++;
	}

	if (g_ucCoreCommand == 0)   // => output to console
	{
		printf_P(PSTR("#%d:242=%d\n"), g_ucSlaveCh, i);
	}
	else                        // => output to FPGA
	{
		CHECKPOINT;
		// TODO KS Remove
		/*
		sprintf_P(response, PSTR("#%d:242=%d"),g_ucSlaveCh, i);
		FPGA_SendStringSPI(response,strlen(response));
		FPGA_SendCharSPI('\r');
		_delay_ms(5);
		*/
	}

	return 0;
}


// ****************************************************************************
// SD_cd
// ****************************************************************************
uint8_t SD_cd(char* ucp_dirname, struct fat_dir_entry_struct* p_subdir_entry)
{
	struct fat_dir_entry_struct subdir_entry;
	struct fat_dir_entry_struct* lp_subdir_entry;

	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	if (ucp_dirname == NULL)
	{
		lp_subdir_entry = p_subdir_entry;
	}
	else
	{
		lp_subdir_entry = &subdir_entry;

		if(!find_file_in_dir(g_fs, g_dd, ucp_dirname, lp_subdir_entry))
		{
			return ERROR_OPENDIRECTORY; // ****tbd
		}
	}

	if ( ! (lp_subdir_entry->attributes & FAT_ATTRIB_DIR))
	{
		return ERROR_OPENDIRECTORY;
	}

	struct fat_dir_struct* dd_new = fat_open_dir(g_fs, lp_subdir_entry);
	if(dd_new)
	{
		// directory handler for the bus commands
		fat_close_dir(g_dd);
		g_dd = dd_new;

		dd_new = fat_open_dir(g_fs, lp_subdir_entry);
		if(dd_new)              // directory handler for the panel file selector
		{
			// (both must be independent, even if pointing to the same directory)
			fat_close_dir(g_dd_panel);
			g_dd_panel = dd_new;

			return SUCCESS;
		}
	}

	return ERROR_OPENDIRECTORY;
}

#ifdef UNUSED_SD_COMMANDS
// ****************************************************************************
// SD_cat
// ****************************************************************************
uint8_t SD_cat(char* ucp_dirname)
{

	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	// search file in current directory and open it
	struct fat_file_struct* fd = open_file_in_dir(g_fs, g_dd, ucp_dirname);
	if(!fd)
	{
		return ERROR_OPENFILE;
	}

	// print file contents
	uint8_t buffer[8];
	uint32_t offset = 0;
	uint16_t count;
	while((count = fat_read_file(fd, buffer, sizeof(buffer))) > 0)
	{

		if (g_ucCoreCommand == 0)	// output to console
		{
			/*
			uart_putdw_hex(offset);
			uart_putc(':');
			for(uint8_t i = 0; i < count; ++i)
			{
			uart_putc(' ');
			uart_putc_hex(buffer[i]);
			}
			uart_putc('\n');
			offset += 8;
			*/
			printf_P(PSTR("#%d:99=%04x:"), g_ucSlaveCh, offset);
			for(uint8_t i = 0; i < count; ++i)
			{
				printf_P(PSTR(" %02x"),buffer[i]);
			}
			printf_P(PSTR("\n"));
			offset += 8;

		}
	}

	fat_close_file(fd);

	return SUCCESS;
}
#endif

#if FAT_WRITE_SUPPORT
// ****************************************************************************
// SD_rm
// ****************************************************************************
uint8_t SD_rm(char* ucp_filename)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	struct fat_dir_entry_struct file_entry;
	if(find_file_in_dir(g_fs, g_dd, ucp_filename, &file_entry))
	{
		if(fat_delete_file(g_fs, &file_entry))
		{
			return SUCCESS;
		}
	}

	return ERROR_DELETEFILE;
}

#ifdef UNUSED_SD_COMMANDS
// ****************************************************************************
// SD_touch
// ****************************************************************************
uint8_t SD_touch(char* ucp_filename)
{

	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	struct fat_dir_entry_struct file_entry;
	if(!fat_create_file(g_dd, ucp_filename, &file_entry))
	{
		return ERROR_CREATEFILE;
	}

	return SUCCESS;
}

// ****************************************************************************
// SD_mv
// ****************************************************************************
uint8_t SD_mv(char* ucp_filename)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	char* target = ucp_filename;
	while(*target != ' ' && *target != '\0')
	++target;

	if(*target == ' ')
	*target++ = '\0';
	else
	{
		return ERROR_CREATEFILE;
	}

	struct fat_dir_entry_struct file_entry;
	if(find_file_in_dir(g_fs, g_dd, ucp_filename, &file_entry))
	{
		if(fat_move_file(g_fs, &file_entry, g_dd, target))
		{
			return SUCCESS;
		}
	}

	return ERROR_CREATEFILE;
}
#endif

// ****************************************************************************
// SD_write_FWR_FWV
// ****************************************************************************
uint8_t SD_write_FWR_FWV(char* ucp_filename, uint8_t ucRegister, float dValue, uint8_t ucFromRegister)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	uint8_t data_len;

	// search file in current directory and open it
	struct fat_file_struct* fd = open_file_in_dir(g_fs, g_dd, ucp_filename);
	if(!fd)
	{
		struct fat_dir_entry_struct file_entry;
		if(!fat_create_file(g_dd, ucp_filename, &file_entry))
		{
			return ERROR_CREATEFILE;
		}
		else
		{
			fd = open_file_in_dir(g_fs, g_dd, ucp_filename);
			if(!fd)
			{
				return ERROR_OPENFILE;
			}

			strcpy_P((char*)gucBuffer, PSTR("VAL\tREG\tTIME\tDATE\n"));
			data_len = strlen((char*)gucBuffer);

			// write header to file
			if(fat_write_file(fd, (uint8_t*) gucBuffer, data_len) != data_len)
			{
				fat_close_file(fd);
				return ERROR_WRITEFILE;
			}
		}
	}

	int32_t offset = 0;
	if(!fat_seek_file(fd, &offset, FAT_SEEK_END))
	{
		fat_close_file(fd);
		return ERROR_SEEKFILE;
	}

	uint8_t sreg = SREG;
	cli();
	sprintf_P((char*)gucBuffer, PSTR("%lf\t%d\t%02d.%02d.%02d\t%02d:%02d:%02d\n"), (ucFromRegister ? g_dScriptRegister[ucRegister] : dValue), ucRegister, gRTC.ucDay, gRTC.ucMonth, gRTC.uiYearWord, gRTC.ucHours, gRTC.ucMinutes, gRTC.ucSeconds);
	SREG = sreg;

	data_len = strlen((char*)gucBuffer);

	// write text to file
	if(fat_write_file(fd, (uint8_t*) gucBuffer, data_len) != data_len)
	{
		fat_close_file(fd);
		return ERROR_WRITEFILE;
	}
	fat_close_file(fd);
	return SUCCESS;
}

// ****************************************************************************
// SD_write_TSF
// ****************************************************************************
/*
uint8_t SD_write_TSF(char* ucp_filename)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	uint8_t data_len;

	// search file in current directory and open it
	struct fat_file_struct* fd = open_file_in_dir(g_fs, g_dd, ucp_filename);
	if(!fd)
	{

		struct fat_dir_entry_struct file_entry;
		if(!fat_create_file(g_dd, ucp_filename, &file_entry))
		{
			return ERROR_CREATEFILE;
		}
		else
		{
			fd = open_file_in_dir(g_fs, g_dd, ucp_filename);
			if(!fd)
			{
				return ERROR_OPENFILE;
			}
		}
	}

	int32_t offset = 0;
	if(!fat_seek_file(fd, &offset, FAT_SEEK_END))
	{
		fat_close_file(fd);
		return ERROR_SEEKFILE;
	}

	FPGA_SetSPIregister(Params.ucSPIcoreTxAVRtoFPGA);
	FPGA_SetSPIvalue8(ACK);        // Select Core 0 ACK

	static uint8_t coreBufferPtr = 0;
	uint8_t uiTmpChar = 0;

	do
	{
		if (g_ucCoreCommand)        // from FPGA
		{
			if (gucIRQwBufferPtr != gucIRQrBufferPtr)
			{
				uiTmpChar = gucIRQBuffer[gucIRQrBufferPtr++];
			}
			else
			{
				continue;
			}
		}
		else    // from bus
		{
			if (Uart_GetRxCount())
			{
				Uart_GetRxData((uint8_t*)&uiTmpChar, 1);
			}
			else
			{
				continue;
			}
		}

		if ( uiTmpChar >= 10)
		{
			gucBuffer[coreBufferPtr++] = uiTmpChar;
		}

		if (uiTmpChar == CR)
		{
			gucBuffer[coreBufferPtr] = 0;

			LOG_INFO(PSTR("TSFdata:%s\n"), gucBuffer);

			if(fat_write_file(fd, (uint8_t*) gucBuffer, coreBufferPtr) != coreBufferPtr)
			{
				fat_close_file(fd);
				return ERROR_WRITEFILE;
			}

			FPGA_SetSPIregister(Params.ucSPIcoreTxAVRtoFPGA);
			FPGA_SetSPIvalue8(ACK);        // Select Core 0 ACK

			coreBufferPtr = 0;
		}

	}
	while (uiTmpChar != 3);             // Contrl-C

	gucBuffer[coreBufferPtr] = 0;       // for the case that the last line was not completed by a CR
	data_len = strlen((char*) gucBuffer);

	if (data_len)
	{
		// write text to file
		if(fat_write_file(fd, (uint8_t*) gucBuffer, data_len) != data_len)
		{
			fat_close_file(fd);
			return ERROR_WRITEFILE;
		}
	}

	FPGA_SetSPIregister(Params.ucSPIcoreTxAVRtoFPGA);
	FPGA_SetSPIvalue8(ACK);        // Select Core 0 ACK

	fat_close_file(fd);
	return SUCCESS;
}
*/
// ****************************************************************************
// SD_write_BSV
// ****************************************************************************
/*
uint8_t SD_write_BSV(char* ucp_filename)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	if (Params.ul_AIE_SPI_FPGA_EndAddress <= Params.ul_AIM_SPI_FPGA_StartAddress)
	return ERROR_PARAMETER;

	// search file in current directory and open it
	struct fat_file_struct* fd = open_file_in_dir(g_fs, g_dd, ucp_filename);
	if(!fd)
	{
		// if file does not exist, create a new one
		struct fat_dir_entry_struct file_entry;
		if(!fat_create_file(g_dd, ucp_filename, &file_entry))
		{
			return ERROR_CREATEFILE;
		}
		else
		{
			fd = open_file_in_dir(g_fs, g_dd, ucp_filename);
			if(!fd)
			{
				return ERROR_OPENFILE;
			}
		}
	}
	else // resize the existing file overwrite the content from beginning and make it only as long as required
	{
		if(!fat_resize_file(fd, 0))
		{
			fat_close_file(fd);
			return ERROR_SEEKFILE;
		}
	}

	PrepareAutoIncTransfer(READ);

	// read from FPGA via SPI auto incrementation

	uint16_t uiBlockCount = (((Params.ul_AIE_SPI_FPGA_EndAddress - Params.ul_AIM_SPI_FPGA_StartAddress - 1) / sizeof(gucBuffer) ) / Params.uc_AIW_SPI_FPGA_Wordlength) + 1;
	uint16_t uiBytePtr = 0;

	while (uiBlockCount > 0)
	{
		while(uiBytePtr < sizeof(gucBuffer))
		{
			FPGA_SetSPIvalue8_16_32(&gucBuffer[uiBytePtr], Params.uc_AIW_SPI_FPGA_Wordlength);
			uiBytePtr += Params.uc_AIW_SPI_FPGA_Wordlength;
		}

		if(fat_write_file(fd, (uint8_t*) gucBuffer, uiBytePtr) != (int16_t)uiBytePtr)
		{
			fat_close_file(fd);
			return ERROR_WRITEFILE;
		}
		uiBytePtr = 0;
		uiBlockCount--;
	}

	fat_close_file(fd);

	FPGA_SetSPIregister(pParams->uc_AIR_SPI_FPGA_Baseaddress + SPI_FPGA_OFFSET_CORE_SELECTRESET);
	FPGA_SetSPIvalue8(pParams->uc_AIS_SPI_FPGA_CORE_Select);

	return SUCCESS;
}
*/
// ****************************************************************************
// SD_MakeDir
// ****************************************************************************
uint8_t SD_MakeDir(char* ucp_dirname)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	struct fat_dir_entry_struct dir_entry;

	if(!fat_create_dir(g_dd, (char*) ucp_dirname, &dir_entry))
	{
		return ERROR_CREATEDIRECTORY;
	}

	return SUCCESS;
}

#endif

#if SD_RAW_WRITE_BUFFERING
// ****************************************************************************
// SD_Sync
// ****************************************************************************
uint8_t SD_Sync(void)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	if(!sd_raw_sync())
	{
		return ERROR_SYNCDISK; // error syncing disk
	}

	return SUCCESS;
}
#endif

// ****************************************************************************
// SD_query
// ****************************************************************************
uint8_t SD_query(char* ucp_dirname)
{
	if (!g_ucSDpresent)
	return ERROR_NOCARD;

	// search file in current directory
	struct fat_dir_entry_struct file_entry;
	if(!find_file_in_dir(g_fs, g_dd, ucp_dirname, &file_entry))
	{
		return ERROR_OPENFILE;
	}

	return SUCCESS;
}

uint32_t strtolong(const char* str)
{
	uint32_t l = 0;
	while(*str >= '0' && *str <= '9')
	l = l * 10 + (*str++ - '0');

	return l;
}


uint8_t find_file_in_dir(struct fat_fs_struct* fs __attribute__((unused)), struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
	while(fat_read_dir(dd, dir_entry))
	{
		if(strcmp(dir_entry->long_name, name) == 0)
		{
			fat_reset_dir(dd);
			return 1;
		}
	}

	return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
	struct fat_dir_entry_struct file_entry;
	if(!find_file_in_dir(fs, dd, name, &file_entry))
	return 0;

	return fat_open_file(fs, &file_entry);
}

#ifdef UNUSED_SD_COMMANDS
uint8_t print_disk_info(const struct fat_fs_struct* fs)
{
	if (!sd_raw_available())
	return ERROR_NOCARD;

	if(!fs)
	return 0;

	struct sd_raw_info disk_info;
	if(!sd_raw_get_info(&disk_info))
	return 0;

	printf_P(PSTR("manufacturer:  0x%x\n"), disk_info.manufacturer);
	/*
	uart_puts_p(PSTR("oem:    ")); uart_puts((char*) disk_info.oem); uart_putc('\n');
	uart_puts_p(PSTR("prod:   ")); uart_puts((char*) disk_info.product); uart_putc('\n');
	uart_puts_p(PSTR("rev:    ")); uart_putc_hex(disk_info.revision); uart_putc('\n');
	uart_puts_p(PSTR("serial: 0x")); uart_putdw_hex(disk_info.serial); uart_putc('\n');
	uart_puts_p(PSTR("date:   ")); uart_putw_dec(disk_info.manufacturing_month); uart_putc('/');
	uart_putw_dec(disk_info.manufacturing_year); uart_putc('\n');
	uart_puts_p(PSTR("size:   ")); uart_putdw_dec(disk_info.capacity / 1024 / 1024); uart_puts_p(PSTR("MB\n"));
	uart_puts_p(PSTR("copy:   ")); uart_putw_dec(disk_info.flag_copy); uart_putc('\n');
	uart_puts_p(PSTR("wr.pr.: ")); uart_putw_dec(disk_info.flag_write_protect_temp); uart_putc('/');
	uart_putw_dec(disk_info.flag_write_protect); uart_putc('\n');
	uart_puts_p(PSTR("format: ")); uart_putw_dec(disk_info.format); uart_putc('\n');
	uart_puts_p(PSTR("free:   ")); uart_putdw_dec(fat_get_fs_free(fs)); uart_putc('/');
	uart_putdw_dec(fat_get_fs_size(fs)); uart_putc('\n');
	*/
	return 1;
}
#endif