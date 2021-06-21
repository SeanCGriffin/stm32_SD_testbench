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

typedef struct {
	uint16_t payload_length : 12;
	uint8_t payload_type : 4;

} __attribute__((packed)) PacketHeader;

typedef struct {
	PacketHeader header;
	uint8_t timestamp[6]; //48 bits
} __attribute__((packed)) TimestampPacket;

typedef struct {
	PacketHeader header;
	uint8_t PMT : 5;
	uint32_t toffset : 21; //offset timestamp
	uint8_t feat_ex: 1;
	uint16_t ampl: 12;
	//uint8_t pad: 1; //Leftover bit

} __attribute__((packed)) SPEPacket;

typedef struct {
	PacketHeader header;
	uint8_t PMT : 5;
	uint32_t toffset : 21; //offset timestamp
	uint8_t waveforms[35]; //23 samples * 12 bits. There will be 4 bits wasted.
} __attribute__((packed)) ComplexPacket;


uint32_t calc_rate(uint32_t delta, uint32_t block_size, float ticksize);
void size_benchmark(TIM_HandleTypeDef timer1000k, uint32_t nbytes);
void IO_benchmark(TIM_HandleTypeDef timer1000k, uint32_t *sizes, uint32_t n);


#endif /* INC_SD_BENCHMARKING_H_ */
