/*
 * file_IO.c
 *
 *  Created on: May 6, 2021
 *      Author: Sean
 */


#include "file_IO.h"




void print_card_info(char* volume){
	//Get the SD card information.
	FRESULT fres;
	DWORD free_clusters, free_sectors, total_sectors;
	FATFS *getFreeFs;
	print("----\r\n");
	fres = f_getfree(volume, &free_clusters, &getFreeFs);
	if (fres != FR_OK) {
		print("f_getfree error (%i)\r\n", fres);
		while(1){
			flash_error();
		}
	}

	//Formula comes from ChaN's documentation
	total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
	free_sectors = free_clusters * getFreeFs->csize;

	print("Total sectors: %lu\r\n"
		  "Free sectors: %lu\r\n"
		  "Sector size: %lu\r\n"
		  "Cluster size: %lu [clusters]\r\n",
		  getFreeFs->n_fatent-2,
		  free_clusters,
		  //getFreeFs->ssize,
		  getFreeFs->csize
		  );

	//Calculation assumes 512 bytes / sector
	print("SD card stats:\r\n"
			"%10lu KiB total drive space.\r\n"
			"%10lu KiB available.\r\n",
			total_sectors / 2, free_sectors / 2);

	return;
}





FRESULT mount_fs(FATFS *FatFs, char* path){

	//print("Mounting filesystem...\r\n");
	FRESULT fres = f_mount(FatFs, path, 1); //1=mount now
	if (fres != FR_OK) {
		print("f_mount error (%i)\r\n\n", fres);
		while(1){
			flash_error();
		}
	} else {
		//print("--> Micro SD card is mounted successfully!\r\n\n");
	}

	return fres;
}

FRESULT unmount_fs(){
	//Unmount the filesystem.
	//print("Unmounting filesystem...\r\n");
	FRESULT fres = f_mount(NULL, "", 1);
	if (fres == FR_OK) {
		//print("--> The Micro SD card is unmounted!\r\n\n");
	} else if (fres != FR_OK) {
		print("--> The Micro SD was not unmounted!\r\n\n");
		flash_error();
	}

	return fres;
}

//Source: http://elm-chan.org/fsw/ff/doc/readdir.html
FRESULT recursive_ls (
		char* path        /* Start node to be scanned (***also used as work area***) */
)
{
	FRESULT res;
	DIR dir;
	UINT i;
	static FILINFO fno;

	res = f_opendir(&dir, path);                       /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			res = f_readdir(&dir, &fno);                   /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) {                    /* It is a directory */
				i = strlen(path);
				sprintf(&path[i], "/%s", fno.fname);
				res = recursive_ls(path);                    /* Enter the directory */
				if (res != FR_OK) break;
				path[i] = 0;
			} else {                                       /* It is a file. */
				print("%s/%s\t%lu B\r\n", path, fno.fname, fno.fsize);
			}
		}
		f_closedir(&dir);
	}

	return res;
}

FRESULT open_file(FIL* fil, char* filename, BYTE mode){
	//print("Opening '%s' in mode %u...\r\n", filename, mode);
	FRESULT fres = f_open(fil, filename,  mode);
	if(fres == FR_OK) {
		//print("--> Opened '%s'.\r\n\n", filename);
	} else {
		print("f_open error (%i)\r\n", fres);
		flash_error();
	}

	return fres;
}

FRESULT close_file(FIL *fil){
	//print("Closing file ...\r\n");
	FRESULT fres = f_close(fil);
	if (fres == FR_OK) {
		//print("--> The file is closed.\r\n\n");
	} else if (fres != FR_OK) {
		print("--> The file was not closed.\r\n\n");
		flash_error();
	}
	return fres;
}
