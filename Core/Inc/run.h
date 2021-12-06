/*
 * run.h
 *
 *  Created on: Nov 15, 2021
 *      Author: sgriffin
 */

#ifndef INC_RUN_H_
#define INC_RUN_H_

#include <cstdarg> //va_list

#include "base_types.h"


#ifdef PLATFORM_STANDALONE

#else
    #pragma message( "Compiling " __FILE__ " for STM32")
    #include "main.h" //HAL, print
    //MCU bits
    extern SD_HandleTypeDef hsd1;
    extern TIM_HandleTypeDef htim5;
    extern UART_HandleTypeDef huart3;
#endif



//Main run stuff
void run_SDMMC_testbench();


#endif /* INC_RUN_H_ */
