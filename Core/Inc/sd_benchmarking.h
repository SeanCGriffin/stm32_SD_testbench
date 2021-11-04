/*
 * sd_benchmarking.h
 *
 *  Created on: May 10, 2021
 *      Author: Sean
 *
 *     Moves SD benchmarking code into standalone libraries.
 *
 */

#ifndef INC_SD_BENCHMARKING_H_
#define INC_SD_BENCHMARKING_H_


#include "file_IO.h"

typedef struct {

	//uint8_t PMT : 5;
	//uint32_t timestamp : 21;
	//uint8_t feat_ex : 1;
	uint8_t timestamp :8;
//uint8_t adc_data[5];

} __attribute__((packed)) TestPacket;




uint32_t calc_rate(uint32_t delta, uint32_t block_size, float ticksize);
void mkdir_benchmark(TIM_HandleTypeDef* timer);
void size_benchmark(TIM_HandleTypeDef timer1000k, uint32_t nbytes, uint8_t n, uint32_t *block_sizes);
void IO_benchmark(TIM_HandleTypeDef timer1000k, uint32_t *sizes, uint32_t n);


#endif /* INC_SD_BENCHMARKING_H_ */
