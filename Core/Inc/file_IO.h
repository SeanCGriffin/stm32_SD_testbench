/*
 * file_IO.h
 *
 *  Created on: May 6, 2021
 *      Author: Sean
 */

#ifndef INC_FILE_IO_H_
#define INC_FILE_IO_H_

#include <fatfs_sd.h>
#include "string.h"
#include "fatfs.h"
#include "stdio.h"

#define verbose 1

extern void print(const char *fmt, ...);
extern void flash_error();


void print_card_info(char* volume);
FRESULT recursive_ls (char* path);
FRESULT mount_fs(FATFS* FatFs, char* path);
FRESULT unmount_fs();
FRESULT open_file(FIL* fil, char* path, BYTE mode);
FRESULT close_file(FIL* fil);




#endif /* INC_FILE_IO_H_ */
