#ifndef __DEVCFG_H__
#define __DEVCFG_H__

#include <stm32h7xx_hal.h>


 #define SD_CS_Pin GPIO_PIN_6
 #define SD_CS_GPIO_Port GPIOF
 #define LD1_Pin GPIO_PIN_0
 #define LD1_GPIO_Port GPIOB
 #define LD3_Pin GPIO_PIN_14
 #define LD3_GPIO_Port GPIOB
 #define LD2_Pin GPIO_PIN_1
 #define LD2_GPIO_Port GPIOE


//Object handle definitions.  
TIM_HandleTypeDef htim2;
SPI_HandleTypeDef hspi5;

#define SD_CS_Pin GPIO_PIN_6
#define SD_CS_GPIO_Port GPIOF
//extern SPI_HandleTypeDef 	hspi5;
#define HSPI_SDCARD		 	&hspi5
#define	SD_CS_PORT			SD_CS_GPIO_Port //GPIOF
#define SD_CS_PIN			SD_CS_Pin //GPIO_PIN_6

#define SPI5_SCK_PORT GPIOF
#define SPI5_SCK_PIN GPIO_PIN_7
#define SPI5_MISO_MOSI_PORT GPIOF
#define SPI5_MOSI_PIN GPIO_PIN_9
#define SPI5_MISO_PIN GPIO_PIN_8
#define SPI5_MISO_MOSI_PINS (SPI5_MISO_PIN | SPI5_MOSI_PIN)

#endif // __DEVCFG_H__
