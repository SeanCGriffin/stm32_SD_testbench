/*
 * run.cpp
 *
 *  Created on: Nov 15, 2021
 *      Author: sgriffin
 */


#ifdef PLATFORM_STANDALONE
    #pragma message( "Compiling " __FILE__ " for DESKTOP")
    extern "C" {
        #include "ff_proxy.h"
        #include "printer.h"
    }
#else
    #pragma message( "Compiling " __FILE__ " for STM32")
    extern "C" {
		#include "ff.h"
    	#include "fatfs.h"
    	#include "fatfs_help.h"
    }
#endif

#include "run.h"
#include "base_types.h"

#include "streamer.h"
#include "packet.h"

#include <cstdio>//sprintf


using namespace hitspool;

void run_SDMMC_testbench(){

	print("-----------------------------------\r\n");
	print("---- SDMMC Interface Testbench ----\r\n");
	print("-----------------------------------\r\n\n");

	for (int i = 0; i < 10; i++) {

		HAL_GPIO_TogglePin(BLUE_LED_PORT, BLUE_LED_PIN);
		HAL_Delay(100);
		HAL_GPIO_TogglePin(GREEN_LED_PORT, GREEN_LED_PIN);
		HAL_Delay(100);
		HAL_GPIO_TogglePin(RED_LED_PORT, RED_LED_PIN);
		HAL_Delay(100);

	}
	HAL_Delay(500);

	FIL *fil = (FIL*) malloc(sizeof(FIL));		       //File handle
	FILINFO *finfo = (FILINFO*) malloc(sizeof(FILINFO));  //File information hanle
	FRESULT fres = FR_OK;   				   //Result after operati ons
	FATFS *fs = (FATFS*) malloc(sizeof(FATFS)); 	       //Filesystem handle

	UNUSED(finfo);
	UNUSED(fil);
	UNUSED(fres);

	char fs_buffer[200];

	//Mount the filesystem.
	print("Mounting filesystem.\r\n");
	sprintf(fs_buffer, "/");
	fres = f_mount(fs, fs_buffer, 1);
	if (fres != FR_OK){
		print("Failed to mount filesystem!!!\r\n");
		Error_Handler();
	}
	print("Done.\r\n");

	//f_ls(fs_buffer);
    G_STATUS gres = G_NOTOK;

    gres = hs_hit_io_unit_test();
    print("hs_unit_write_loop()\t %s (%d)\r\n", gres == G_OK ? "PASSED" : "FAILED", gres);

	print("Unmounting filesystem.\r\n");
	sprintf(fs_buffer, "/");
	fres = f_mount(0, fs_buffer, 0);
	if(fres != FR_OK)
		Error_Handler();

	print("Done.\r\n");


}


