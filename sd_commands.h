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


#ifndef _SD_COMMANDS_H_
#define _SD_COMMANDS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct fat_fs_struct* g_fs;
extern struct fat_dir_struct* g_dd;
extern struct fat_dir_struct* g_dd_ini;
extern struct fat_dir_struct* g_dd_panel;

extern struct fat_dir_entry_struct g_directory;
extern struct partition_struct* g_partition;

extern uint8_t SD_init(void);
extern void SD_close(void);
extern uint8_t SD_IsCardPresent(void);
extern uint8_t SD_ls(uint8_t ucFlag);
extern uint8_t SD_filenum(void);
extern uint8_t SD_cd(char* ucp_dirname, struct fat_dir_entry_struct* p_subdir_entry);

#ifdef UNUSED_SD_COMMANDS
extern uint8_t SD_cat(char* ucp_dirname);
#endif

#if FAT_WRITE_SUPPORT
extern uint8_t SD_rm(char* ucp_filename);

#ifdef UNUSED_SD_COMMANDS
extern uint8_t SD_touch(char* ucp_filename);
extern uint8_t SD_mv(char* ucp_filename);
#endif
extern uint8_t SD_write_FWR_FWV(char* ucp_filename, uint8_t ucRegister, float dValue , uint8_t ucFromRegister);
extern uint8_t SD_write_TSF(char* ucp_filename);
extern uint8_t SD_write_BSV(char* ucp_filename);
extern uint8_t SD_MakeDir(char* ucp_dirname);
#endif /* FAT_WRITE_SUPPORT */


#if SD_RAW_WRITE_BUFFERING
extern uint8_t SD_Sync(void);
#endif /* SD_RAW_WRITE_BUFFERING */

uint8_t SD_query(char* ucp_dirname);

extern uint32_t strtolong(const char* str);

extern uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
extern struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name);

#ifdef UNUSED_SD_COMMANDS
extern uint8_t print_disk_info(const struct fat_fs_struct* fs);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SD_COMMANDS_H_ */