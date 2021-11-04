/*
 * file_IO.h
 *
 *  Created on: May 6, 2021
 *      Author: Sean
 */

#ifndef INC_FILE_IO_H_
#define INC_FILE_IO_H_



#include <hs_streamer.h> //Packet structs
#include "fatfs.h"
#include "main.h" //typedefs



#define verbose 1


void print_card_info(char* volume);
FRESULT verbose_ls (char* path);
FRESULT recursive_ls (char* path);
FRESULT mount_fs(FATFS* FatFs, char* path);
FRESULT unmount_fs();
FRESULT open_file(FIL* fil, char* path, BYTE mode);
FRESULT close_file(FIL* fil);
FRESULT mkdir(char* path);
FRESULT delete_node (TCHAR* path, UINT sz_buff, FILINFO* fno);
FRESULT stat_file(char *path);
FRESULT make_dafault_filesystem();

uint32_t get_total_space_KiB(char* volume);
uint32_t get_free_space_KiB(char* volume);


#endif /* INC_FILE_IO_H_ */
