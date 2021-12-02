/*
 * fatfs_help.h
 *
 *  Created on: Nov 11, 2021
 *      Author: sgriffin
 */

#ifndef INC_FATFS_HELP_H_
#define INC_FATFS_HELP_H_

#include "fatfs.h"

#define f_rm f_unlink;
FRESULT f_ls(char* path);
FRESULT f_recursive_rm (TCHAR* path, UINT sz_buff, FILINFO* fno);


#endif /* INC_FATFS_HELP_H_ */
