/*
 * file_IO.c
 *
 *  Created on: May 6, 2021
 *      Author: Sean
 */

#include <stdio.h>
#include <string.h>
#include "file_IO.h"


extern void print(const char *fmt, ...);
extern void flash_error();

uint64_t volume_total_space_KiB = 0;
uint64_t volume_free_space_KiB = 0;

uint32_t get_total_space_KiB(char* volume){
	//Get the SD card information.
	FRESULT fres;
	DWORD free_clusters, free_sectors, total_sectors;
	FATFS *fs;

	fres = f_getfree(volume, &free_clusters, &fs);
	if (fres != FR_OK) {
		print("f_getfree error (%i)\r\n", fres);
		while(1){
			flash_error();
		}
	}

	//Formula comes from ChaN's documentation
	total_sectors = (fs->n_fatent - 2) * fs->csize;
	free_sectors = free_clusters * fs->csize;

	return total_sectors / 2;
}

uint32_t get_free_space_KiB(char* volume){
	//Get the SD card information.
	FRESULT fres;
	DWORD free_clusters, free_sectors, total_sectors;
	FATFS *fs;

	fres = f_getfree(volume, &free_clusters, &fs);
	if (fres != FR_OK) {
		print("f_getfree error (%i)\r\n", fres);
		while(1){
			flash_error();
		}
	}

	//Formula comes from ChaN's documentation
	total_sectors = (fs->n_fatent - 2) * fs->csize;
	free_sectors = free_clusters * fs->csize;

	return free_sectors / 2;
}

void print_card_info(char* volume){
	//Get the SD card information.
	FRESULT fres;
	DWORD free_clusters, free_sectors, total_sectors;
	FATFS *fs;

	fres = f_getfree(volume, &free_clusters, &fs);
	if (fres != FR_OK) {
		print("f_getfree error (%i)\r\n", fres);
		while(1){
			flash_error();
		}
	}

	//Formula comes from ChaN's documentation
	total_sectors = (fs->n_fatent - 2) * fs->csize;
	free_sectors = free_clusters * fs->csize;

	print("Total sectors: %lu\r\n"\
		  "Free sectors:  %lu\r\n",
		  total_sectors,
		  free_sectors
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


FRESULT verbose_ls (char* path){
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    print("ls %s\r\n", path);
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
            	print("Timestamp: %u/%02u/%02u, %02u:%02u\n",
            		   (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
            		   fno.ftime >> 11, fno.ftime >> 5 & 63);
            	print("%s/%s\t%lu B\r\n", path, fno.fname, fno.fsize);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

//FRESULT get_directory_contents(char* path, int *ncontents, char){
//
//	FRESULT res;
//	DIR dir;
//	UINT i;
//	static FILINFO finfo;
//
//
//	return res;
//
//}

//Source: http://elm-chan.org/fsw/ff/doc/readdir.html
FRESULT recursive_ls (
		char* path        /* Start node to be scanned */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    print("ls %s\r\n", path);
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
            	f_stat(path,  &fno);
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
		Error_Handler();
	}

	return fres;
}

FRESULT close_file(FIL *fil){
	//print("Closing file ...\r\n");
	FRESULT fres = f_close(fil);
	if (fres == FR_OK) {
		//print("--> The file is closed.\r\n\n");
	} else if (fres != FR_OK) {
		print("--> The file was not closed (%d).\r\n\n", fres);
		Error_Handler();
	}
	return fres;
}

//FIXME: This needs to be cleaned up; none of the arguments are really required except $path.
//http://elm-chan.org/fsw/ff/res/app2.c
FRESULT delete_node (
    TCHAR* path,    /* Path name buffer with the sub-directory to delete */
    UINT sz_buff,   /* Size of path name buffer (items) */
    FILINFO* fno    /* Name read buffer */
)
{
    UINT i, j;
    FRESULT fr;
    DIR dir;
    print("rm %s\r\n", path);
    fr = f_opendir(&dir, path); /* Open the sub-directory to make it empty */
    if (fr != FR_OK) return fr;

    for (i = 0; path[i]; i++) ; /* Get current path length */
    path[i++] = _T('/');

    for (;;) {
        fr = f_readdir(&dir, fno);  /* Get a directory item */
        if (fr != FR_OK || !fno->fname[0]) break;   /* End of directory? */
        j = 0;
        do {    /* Make a path name */
            if (i + j >= sz_buff) { /* Buffer over flow? */
                fr = 100; break;    /* Fails with 100 when buffer overflow */
            }
            path[i + j] = fno->fname[j];
        } while (fno->fname[j++]);
        if (fno->fattrib & AM_DIR) {    /* Item is a sub-directory */
            fr = delete_node(path, sz_buff, fno);
        } else {                        /* Item is a file */
            fr = f_unlink(path);
        }
        if (fr != FR_OK) break;
    }

    path[--i] = 0;  /* Restore the path name */
    f_closedir(&dir);

    if (fr == FR_OK) fr = f_unlink(path);  /* Delete the empty sub-directory */
    return fr;
}


FRESULT mkdir(char *path){

	FRESULT fres = f_mkdir(path);
	if(fres != FR_OK){
		print("f_mkdir('%s') failed with fres=(%i)\r\n", path, fres);
		//Error_Handler();
	}

	return fres;
}

FRESULT stat_file(char *path){
    FRESULT fr;
    FILINFO fno;
    print("stat(%s)\r\n", path);
    fr = f_stat(path, &fno);
    switch (fr) {

		case FR_OK:
			print("Size: %lu\n", fno.fsize);
			print("Timestamp: %u/%02u/%02u, %02u:%02u\n",
				   (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
				   fno.ftime >> 11, fno.ftime >> 5 & 63);
			print("Attributes: %c%c%c%c%c\n",
				   (fno.fattrib & AM_DIR) ? 'D' : '-',
				   (fno.fattrib & AM_RDO) ? 'R' : '-',
				   (fno.fattrib & AM_HID) ? 'H' : '-',
				   (fno.fattrib & AM_SYS) ? 'S' : '-',
				   (fno.fattrib & AM_ARC) ? 'A' : '-');
			break;

		case FR_NO_FILE:
			print("%s does not exist.\r\n", path);
			break;

		default:
			print("An error occurred. (%d)\n", fr);
    }

    return fr;
}


FRESULT make_default_filesystem(){

	FRESULT fres;   //Result after operations
	BYTE work[512]; /* Work area (larger is better for processing time) */

	print("Nuking filesystem...\r\n");
	/* Format the default drive with default parameters */

	fres = f_mkfs("", FM_ANY, 0, work, sizeof work);
	if(fres == FR_OK) {
		//print("--> Opened '%s'.\r\n\n", filename);
	} else {
		print("f_mkfs error (%i)\r\n", fres);
		Error_Handler();
	}

	return fres;

}

