/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <hs_streamer.h> //Packet definitions
#include "main.h"
#include "fatfs.h"
#include "types.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h> //for va_list
#include <time.h>
#include <inttypes.h>
#include <stdbool.h>

#include "file_IO.h"

#include "sd_benchmarking.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SD_HandleTypeDef hsd1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

SystemTime s = { 0 };
uint32_t sys_seconds = 0xFF000000;
uint16_t sys_subseconds = 0;

extern bool handler_active[NUM_PMT];
extern FIL handlers[NUM_PMT];
extern char live_filenames[NUM_PMT][256];
extern bool daq_enabled;

extern PayloadType_t current_hit_type;
extern SPEPacket *spep;
extern MPEPacket *mpep;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void print(const char *fmt, ...) {
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	HAL_UART_Transmit(&huart3, (uint8_t*) buffer, strlen(buffer), -1);

}

void flash_error() {

	HAL_GPIO_TogglePin(RED_LED_PORT, RED_LED_PIN);
	HAL_Delay(10);
	HAL_GPIO_TogglePin(RED_LED_PORT, RED_LED_PIN);
	HAL_Delay(100);

}

SystemTime* get_system_time() {

	/*
	 * Dummy system time generator.
	 *
	 */

	(&s)->coarse = sys_seconds;
	(&s)->fine = (__HAL_TIM_GET_COUNTER(&htim5) / 1000);
	return &s;

}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_SDMMC1_SD_Init();
	MX_TIM2_Init();
	MX_FATFS_Init();
	MX_TIM5_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */

	//Start timers.
	HAL_TIM_Base_Start(&htim2); //1 MHz; 32 bit
	HAL_TIM_Base_Start_IT(&htim5); //1 MHz; 32 bit

	u16 nsamples = 300;
	u8 sample_buf[nsamples];
	for (int i = 0; i < nsamples; i++)
		sample_buf[i] = i;

	print("\r\n---- SDMMC Interface Testbench ----\r\n");
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

	FIL *fil = malloc(sizeof(FIL));		       //File handle
	FILINFO *finfo = malloc(sizeof(FILINFO));  //File information hanle
	FRESULT fres = FR_OK;   				   //Result after operati ons
	FATFS *fs = malloc(sizeof(FATFS)); 	   //Filesystem handle

	UNUSED(finfo);
	UNUSED(fil);
	UNUSED(fres);

	char filename_buffer[200];

	//Mount the filesystem.
	print("Mounting filesystem.\r\n");
	sprintf(filename_buffer, "/");
	fres = mount_fs(fs, filename_buffer);
	if (fres != FR_OK)
		Error_Handler();

	//	fres = f_stat("/test.img", finfo);
	//	print("%s\t%lu", finfo->fname, finfo->fsize);
	//	return;

	//Display device info.
	print("\r\nDevice stats:\r\n");
	print("--------------------\r\n");
	print_card_info(filename_buffer);

	//recursive_ls(filename_buffer);

	sprintf(filename_buffer, "/hitspool");
	if (mkdir(filename_buffer) == FR_EXIST) {
		print("Directory '%s' already exists, but that's fine.\r\n",
				filename_buffer);
	}

	print("Generating PMT subdirectories.\r\n");

	for (int i = 0; i < NUM_PMT; i++) {
		sprintf(filename_buffer, "/hitspool/PMT%02d", i);
		print("mkdir('%s')\r\n", filename_buffer);

		fres = f_mkdir(filename_buffer);
		if (fres == FR_EXIST) {
			print("Directory '%s' already exists, but that's fine.\r\n",
					filename_buffer);
		} else if (fres != FR_OK) {
			print("mkdir failed with code %d\r\n", fres);
			Error_Handler();
		}

		//recursive_ls(filename_buffer);

	}

	sprintf(filename_buffer, "/hitspool");
	recursive_ls(filename_buffer);

	print("\r\n\n");
	print("\r\n---- Streamer Testbench ----\r\n");
	print("-----------------------------------\r\n\n");

	print("\r\n\n");
	print("Initializing hit buffers, write heads, file handlers.\r\n\n");
	init_write_heads();
	print("Write heads done.\r\n");
	init_file_handlers();
	print("File handlers done.\r\n");
	init_hit_buffers();
	print("Hit buffers done.\r\n");

	print_IO_handlers();

	print("Entering data generation loop.\r\n");
	test_hit_loop(0, 10);



//	sprintf(filename_buffer, "/hitspool/test_extend.test");
//	fres = open_file(fil, filename_buffer, FA_WRITE|FA_CREATE_ALWAYS);
//	if(fres!=FR_OK){
//		Error_Handler();
//	}
//
//	fres = f_stat(filename_buffer, finfo);
//	stat_file(filename_buffer);
//
//	fres = f_expand(fil, 100000, 1);
//	if(fres!=FR_OK){
//		print("--> Error expanding new file %s; exiting...\r\n", filename_buffer);
//		Error_Handler();
//	}

	//
//	//	//1048576 B in one MiB
//	uint32_t MiB = 1048576;
//	//uint32_t nBytes = 64;
//	uint32_t nBytes = 5*MiB / pow(2, 8);
//	//uint32_t nBytes = MiB;
//	uint32_t block_sizes[] = {1024};
//	uint8_t n = 1;
//	size_benchmark(htim2, nBytes, n, block_sizes);

//	//Set up some variables for FatFS
//	FATFS *fs; 	//Filesystem handle
//	fs = malloc(sizeof(FATFS));
//
//	FIL *fil; 		//File handle
//	FILINFO finfo;  //File information hanle
//	FRESULT fres;   //Result after operati ons
//	UNUSED(fil);
//	UNUSED(fres);
//	//make_default_filesystem();
//
//	char filename_buffer[200];
//
//	//Mount the filesystem.
//	print("Mounting filesystem.\r\n");
//	sprintf(filename_buffer, "/");
//	fres = mount_fs(fs, filename_buffer);
//
//	//Display device info.
//	print("\r\nDevice stats:\r\n");
//	print("--------------------\r\n");
//	print_card_info("/");
//
//
//
//	//Display device contents.
//	sprintf(filename_buffer, "/");
//	print("\r\n\n\n");
//	//print("\r\nDevice contents:\r\n");
//	//recursive_ls(filename_buffer);
//	//stat_file(_T("/notafile.txt"));
//	//
//
//
//	sprintf(filename_buffer, "/hitspool");
//	uint32_t tstart = 0, tend = 0;
//
//	tstart = __HAL_TIM_GET_COUNTER(&htim5);
//	recursive_ls(filename_buffer);
////	delete_node(filename_buffer,
////		sizeof(filename_buffer) / sizeof(filename_buffer[0]), &finfo);
//	tend = __HAL_TIM_GET_COUNTER(&htim5);
//
//	print("Recursive LS deltaT: %10d ms\r\n", (tend-tstart)/1000);
//	//mkdir(filename_buffer);
//	//stat_file(_T("/hitspool"));
//
//	FIL *fp;
//    /* Create a new file */
//    fres = f_open(fp = malloc(sizeof (FIL)), "/hitspool/file.dat", FA_WRITE|FA_CREATE_ALWAYS);
//    if (fres) { /* Check if the file has been opened */
//        print("Failed to open the file.\r\n");
//        return;
//    }
//
//    /* Alloacte a 100 MiB of contiguous area to the file */
//    fres = f_expand(fp, 104857600, 1);
//    if (fres) { /* Check if the file has been expanded */
//        print("Failed to allocate contiguous area.\r\n");
//        return;
//    }
//
//    recursive_ls("/hitspool/");
//    stat_file("/hitspool/file.dat");
//
//
//	return;
//	for (unsigned i = 0; i < NUM_PMT; i++) {
//		sprintf(filename_buffer, "/hitspool/PMT%02d", i);
//		mkdir(filename_buffer);
//	}
//
////	sprintf(filename_buffer, "/hitspool");
////	recursive_ls(filename_buffer);
//
//
//	//Initialize the file handlers.
//	for (int i = 0; i < NUM_PMT; i++) {
//		handler_active[i] = false;
//		sprintf(live_filenames[i], "");
//	}
//
//	char *filename;
//	SPEPacket *spep = malloc(sizeof(SPEPacket));
//
//	for (int i = 0; i < 10; i++) {
//		daq_enabled = true;
//		HAL_GPIO_TogglePin(RED_LED_PORT, RED_LED_PIN);
//		for(int i = 0; i < NUM_PMT; i++){
//			filename = live_filenames[i];
//			if(handler_active[i]){
//				print("Existing filename: PMT%02d -- %s --\r\n", i, filename);
//			}
//		}
//
//		if (daq_enabled) {
//			daq_enabled = false;
//			for (int i = 0; i < NUM_PMT; i++) {
//
//				filename = live_filenames[i];
//				FIL livefile = handlers[i];
//
//				print("Existing filename: PMT%02d -- %s --\r\n", i, filename);
//				if (handler_active[i]) {
//					print("Closing active file for PMT%02d", i);
//					close_file(&livefile);
//				}
//				sprintf(filename, "hitspool/PMT%02d/0x%08lx.spool", i, sys_seconds);
//				print("New filename: -- %s --\r\n", filename);
//
//				fres = open_file(&livefile, filename, FA_CREATE_ALWAYS);
//				handler_active[i] = true;
//			}
//
//			HAL_GPIO_TogglePin(BLUE_LED_PORT, BLUE_LED_PIN);
//			print("\r\n\n");
//			daq_enabled = true;
//		}
//
//		HAL_Delay(0);
//	}

//	uint8_t PMT = 0;
//	for(unsigned i = 0; i < 10000; i++){
//		SystemTime *s = get_system_time();
//		generate_dummy_SPE_hit(spep, i % 18, i + 0xA0);
//		print_SPE_packet(spep);
//		livefile->
//		print("%s\r\n", filename_buffer);
//
//		HAL_Delay(100);
//	}
//
//

	unmount_fs();
	print("\r\n-------\r\n");
	print("Done.\r\n");

//	uint32_t sizes[] = { 1, 32, 1024, 4096, 4096*2, 4096*4, 4096*8, 50000, 60000 };
//	uint32_t n = 7;
//	IO_benchmark(htim2, sizes, n);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 25;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 8;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
			| RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief SDMMC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SDMMC1_SD_Init(void) {

	/* USER CODE BEGIN SDMMC1_Init 0 */
	__HAL_RCC_SDMMC1_CLK_ENABLE();
	/* USER CODE END SDMMC1_Init 0 */

	/* USER CODE BEGIN SDMMC1_Init 1 */

	/* USER CODE END SDMMC1_Init 1 */
	hsd1.Instance = SDMMC1;
	hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
	hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
	hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
	hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
	hsd1.Init.ClockDiv = 5;
	/* USER CODE BEGIN SDMMC1_Init 2 */

	/* USER CODE END SDMMC1_Init 2 */

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 100 - 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 4294967295;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void) {

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 100 - 1;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65535;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */

}

/**
 * @brief TIM5 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM5_Init(void) {

	/* USER CODE BEGIN TIM5_Init 0 */

	/* USER CODE END TIM5_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM5_Init 1 */

	/* USER CODE END TIM5_Init 1 */
	htim5.Instance = TIM5;
	htim5.Init.Prescaler = 100 - 1;
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim5.Init.Period = 1000000 - 1;
	htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim5) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM5_Init 2 */

	/* USER CODE END TIM5_Init 2 */

}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
	GPIO_InitStruct.Pin = RMII_MDC_Pin | RMII_RXD0_Pin | RMII_RXD1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
	GPIO_InitStruct.Pin = RMII_REF_CLK_Pin | RMII_MDIO_Pin | RMII_CRS_DV_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin | LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : RMII_TXD1_Pin */
	GPIO_InitStruct.Pin = RMII_TXD1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
	GPIO_InitStruct.Pin = USB_SOF_Pin | USB_ID_Pin | USB_DM_Pin | USB_DP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : SD_Detect_Pin */
	GPIO_InitStruct.Pin = SD_Detect_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SD_Detect_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
	GPIO_InitStruct.Pin = RMII_TX_EN_Pin | RMII_TXD0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
		flash_error();
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
