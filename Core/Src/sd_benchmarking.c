/*
 * sd_benchmarking.c
 *
 *  Created on: May 10, 2021
 *      Author: Sean Griffin
 *
 *
 *     Moves SD benchmarking code into standalone libraries.
 */

#include "sd_benchmarking.h"

uint32_t calc_rate(uint32_t delta, uint32_t block_size, float ticksize){

	return (uint32_t) block_size / (delta * ticksize);

}
void size_benchmark(TIM_HandleTypeDef timer1000k, uint32_t nbytes){

	//some variables for FatFs
	FATFS FatFs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;   //Result after operations
	char filename_buffer[200];
	char text_buffer[200];

	//Mount the filesystem.
	sprintf(filename_buffer, "/");
	fres = mount_fs(&FatFs, filename_buffer);
	//uint16_t block_sizes[] = {1, 128, 4096};
	uint32_t block_sizes[] = {128, 4096, 5*4096};
	uint8_t n = 3;
	UINT bw = 0, br = 0;
	uint32_t nerrors[] = {0, 0};
	uint32_t t1000, t1000_now, t1, t1_now;
	uint32_t r1000, r1;

	uint32_t block_rrate[n][2];
	uint32_t block_wrate[n][2];

	print("----------------------\r\n");
	print("IO Operation Benchmark\r\n");
	print("Target size: %lu bytes\r\n", nbytes);
	print("Target IO sizes: \r\n");
	for(int i = 0; i < n; i++)
		print("\t 0x%6X\r\n", block_sizes[i]);
	print("----------------------\r\n");

	for(int i = 0; i < n; i++){

		uint32_t block_size = block_sizes[i];
		uint32_t nchunks = nbytes / block_size;
		if(verbose){
			print("---------------------\r\n\n");
			print("WRITE BLOCK SIZE 0x%6X\r\n", block_size);
			print("-----\r\n");
		}
		sprintf(filename_buffer, "datalog_nb%lu_bs%lu.txt", nbytes, block_size);
		open_file(&fil, filename_buffer, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);


//		print("Block writing %u bytes: nchunks=%lu, block size=%lu, total = %lu\r\n",
//				nbytes, nchunks, block_size, nchunks*block_size);

		//Allocate and assign memory.
		TestPacket *evtptr = malloc(block_size);
		for (unsigned j = 0; j < block_size; j++) {
			evtptr[j].timestamp = 65+(j % 100);
			//print("%d\r\n", evtptr[j].timestamp);
		}

		//Reset counter and take timestamps.
		__HAL_TIM_SET_COUNTER(&timer1000k, 0);

		t1000 = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1    = HAL_GetTick();

		//Write data to file.
		for(uint32_t j = 0; j < nchunks; j++){
			if(j > 0 && j%1000000 == 0)
				print("Alive! %d/%d\r\n", j, nchunks);
			f_write(&fil, evtptr, block_size, &bw);
			//f_sync(&fil);

			if (block_size != bw)
				print("Error writing data to file:\r\n"
						"\t Block size: 0x%X\t bytes written: 0x%X\r\n",
						block_size, bw);

		}
		f_sync(&fil);
		//Grab counters after I/O.
		t1000_now = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1_now    = HAL_GetTick();

		//Check for errors.
		for(uint32_t j = 0; j < block_size; j++){
			if(evtptr[j].timestamp != 65+(j % 100))
				print("Error in write! 0x%X \t 0x%X \r\n", evtptr[j].timestamp, 65+(j % 100) );
		}

		r1000 = calc_rate(t1000_now-t1000, nbytes, 0.000001);
		r1 = calc_rate(t1_now-t1, nbytes, 0.001);

		if(verbose){
			print("DeltaT_1000: %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1000_now-t1000, nbytes, r1000);
			print("DeltaT_1   : %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1_now-t1, nbytes, r1);
		}
		block_wrate[i][0] = r1000; calc_rate(t1000_now-t1000, nbytes, 0.000001);
		block_wrate[i][1] = r1;

		free(evtptr);
		close_file(&fil);

		if(verbose){
			print("\r\nREAD BLOCK SIZE 0x% 6X\r\n", block_size);
			print("-----\r\n");
		}
		evtptr = malloc(block_size);
		open_file(&fil, filename_buffer, FA_READ);

		__HAL_TIM_SET_COUNTER(&timer1000k, 0);

		t1000 = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1    = HAL_GetTick(); //__HAL_TIM_GET_COUNTER(&timer1k);

		for(uint32_t j = 0; j < nchunks; j++){
			if(j > 0 && j%1000000 == 0)
				print("Alive! %d/%d\r\n", j, nchunks);
			f_read(&fil, evtptr, block_size, &br);

			if (block_sizes[i] != br)
				print("Error reading data from file:\r\n"
						"\t Block size: 0x%X\t bytes read: 0x%X\r\n",
						block_size, br);
		}

		t1000_now = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1_now    = HAL_GetTick(); //__HAL_TIM_GET_COUNTER(&timer1k);

		for(uint32_t j = 0; j < block_size; j++){
			if(evtptr[j].timestamp != 65+(j % 100))
				print("Error in readback! %x \t %x \r\n", evtptr[j].timestamp, 65+(j % 100) );
		}

		r1000 = calc_rate(t1000_now-t1000, nbytes, 0.000001);
		r1 = calc_rate(t1_now-t1, nbytes, 0.001);

		if(verbose){
			print("DeltaT_1000: %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1000_now-t1000, nbytes, r1000);
			print("DeltaT_1   : %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1_now-t1, nbytes, r1);
		}

		block_rrate[i][0] = r1000;
		block_rrate[i][1] = r1;

		free(evtptr);
		close_file(&fil);


	}

	print("\n\n");

	//Dump results to a text file on the SD card to be read offline.
	sprintf(filename_buffer, "chunked_bandwidth_results.txt");
	open_file(&fil, filename_buffer, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
	sprintf(text_buffer, "# Total data written (bytes) %lu\r\n", nbytes);
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	sprintf(text_buffer, "# Write Rates\r\n");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"----------", "----------", "----------");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"Reference", "1MHz", "1kHz");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"Block Size (B)", "", "Rate (B/s)");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"----------", "----------", "----------");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	for(int i = 0; i < n; i++){
		sprintf(text_buffer, "0x%8X\t| %10lu\t %10lu\r\n",
				block_sizes[i],
				block_wrate[i][0], block_wrate[i][1]);
		print(text_buffer);
		//For ease of processing.
		sprintf(text_buffer, "%10u\t, %10lu\t, %10lu\r\n",
				block_sizes[i],
				block_wrate[i][0], block_wrate[i][1]);
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);

	}

	f_puts("\n\n", &fil);
	print("\n\n");

	sprintf(text_buffer, "#Read Rates\r\n");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"----------", "----------", "----------");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"Reference", "1MHz", "1kHz");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"Block Size (B)", "", "Rate (B/s)");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\r\n",
			"----------", "----------", "----------");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	for(int i = 0; i < n; i++){
		sprintf(text_buffer, "0x%8X\t| %10lu\t| %10lu\r\n",
				block_sizes[i],
				block_rrate[i][0], block_rrate[i][1]);
		print(text_buffer);
		sprintf(text_buffer, "%10u\t, %10lu\t, %10lu\r\n",
				block_sizes[i],
				block_rrate[i][0], block_rrate[i][1]);
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);


	}


	close_file(&fil);

	unmount_fs();

	//print("\n\n\n\n");

}

void IO_benchmark(TIM_HandleTypeDef timer1000k, uint32_t *sizes, uint32_t n){

	//some variables for FatFs
	FATFS FatFs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;   //Result after operations
	char filename_buffer[200];
	char text_buffer[200];

	//Mount the filesystem.
	sprintf(filename_buffer, "/");
	fres = mount_fs(&FatFs, filename_buffer);
	UINT bw = 0, br = 0;

	uint32_t t1, t1000, t1_now, t1000_now;
	uint32_t r1, r1000;

	uint32_t block_rrate[n][2];
	uint32_t block_wrate[n][2];
	uint32_t seq_rrate[n][2];
	uint32_t seq_wrate[n][2];

	print("----------------------\r\n");
	print("I/O Block Size Benchmark\r\n");
	print("Target sizes: \r\n");
	for (unsigned i = 0; i < n; i++)
		print("\t %lu bytes\r\n", sizes[i] * sizeof(TestPacket));
	print("----------------------\r\n");

	if(verbose){
		print("Block Write/Read\r\n");
		print("---------------------\r\n\n");
	}
	for (unsigned i = 0; i < n; i++) {

		sprintf(filename_buffer, "datalog.txt");
		open_file(&fil, filename_buffer, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);

		unsigned block_size = sizes[i] * sizeof(TestPacket);
		if(verbose){
			print("---------------------\r\n\n");
			print("WRITE\r\n");
			print("-----\r\n");
			print("Block writing %u bytes: n=%u, size=%u\r\n", block_size, n, sizes[i]);
		}


		TestPacket *evtptr = malloc(block_size);

		for (unsigned j = 0; j < sizes[i]; j++) {
			evtptr[j].timestamp = 65+(j % 100);
//			if(verbose)
//				print("\tTS %u: 0x%X\t%c\r\n", j, evtptr[j].timestamp, evtptr[j].timestamp);
		}

		__HAL_TIM_SET_COUNTER(&timer1000k, 0);

		t1000 = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1    = HAL_GetTick();

		f_write(&fil, evtptr, block_size, &bw);

		f_sync(&fil);

		t1000_now = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1_now    = HAL_GetTick();

		if (block_size != bw)
			print("Error writing data to file:\r\n"
					"\t Block size: 0x%X\t bytes written: 0x%X\r\n",
					block_size, bw);

		r1000 = calc_rate(t1000_now-t1000, block_size, 0.000001);
		r1 = calc_rate(t1_now-t1, block_size, 0.001);
		if(verbose){
			print("DeltaT_1000: %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1000_now-t1000, block_size, r1000);
			print("DeltaT_1   : %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1_now-t1, block_size, r1);
		}
		block_wrate[i][0] = r1000;
		block_wrate[i][1] = r1;

		free(evtptr);

		close_file(&fil);

		//evtptr;// = malloc(sizes[i] * sizeof(TestPacket));
//		for (unsigned j = 0; j < sizes[i]; j++) {
//			evtptr[j].timestamp = 0;
//			print("\tTS %u: 0x%X\r\n", j, evtptr[j].timestamp);
//		}

		if(verbose){
			print("READ\r\n");
			print("-----\r\n");
		}
		evtptr = malloc(block_size);
		for (unsigned j = 0; j < sizes[i]; j++)
			evtptr[j].timestamp = 0;

		open_file(&fil, filename_buffer, FA_READ);


//		if(verbose){
//			print("Blank data:\r\n");
//			for (unsigned j = 0; j < sizes[i]; j++) {
//				evtarray[j].timestamp = 0;
//				print("\tTS %u: 0x%X\r\n", j, evtarray[j].timestamp);
//			}
//			print("\n");
//		}

		//print("Reading data...\r\n");

		__HAL_TIM_SET_COUNTER(&timer1000k, 0);

		t1000 = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1    = HAL_GetTick();

		f_read(&fil, evtptr, block_size, &br);

		t1000_now = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1_now    = HAL_GetTick();

//		if(t100_now < t100)
//			t100_now += 65536;

		r1000 = calc_rate(t1000_now-t1000, block_size, 0.000001);
		r1 = calc_rate(t1_now-t1, block_size, 0.001);
		if(verbose){
			print("DeltaT_1000: %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1000_now-t1000, block_size, r1000);
			print("DeltaT_1   : %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1_now-t1, block_size, r1);
		}
		block_rrate[i][0] = r1000;
		block_rrate[i][1] = r1;

//		print("\tBlock size: 0x%X\t bytes read: 0x%X\r\n\n", block_size, br);
//		print("Readback data:\r\n");
		for (unsigned j = 0; j < sizes[i]; j++) {
			if(evtptr[j].timestamp != 65+(j % 100))
				print("Error in readback!\r\n");
//			print("\tTS %02u: 0x%X\t%c\r\n", j, evtptr[j].timestamp, evtptr[j].timestamp);
		}

		free(evtptr);
		close_file(&fil);
	}


	if(verbose){
		print("Sequential Write/Read Speedtest\r\n");
		print("---------------------\r\n\n");
	}
	for (unsigned i = 0; i < n; i++) {

		sprintf(filename_buffer, "datalog.txt");
		open_file(&fil, filename_buffer, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);

		unsigned block_size = sizes[i] * sizeof(TestPacket);
		if(verbose){
			print("---------------------\r\n\n");
			print("WRITE\r\n");
			print("-----\r\n");
			print("Sequential writing %u bytes: n=%u, size=%u\r\n", block_size, n, sizes[i]);
		}

		TestPacket *evtptr = malloc(block_size);
		//print("Memory address: %p\r\n", (void*)&evtptr);

		for (unsigned j = 0; j < sizes[i]; j++) {
			evtptr[j].timestamp = 65+(j % 100);
//			if(verbose)
//				print("\tTS %u: 0x%X\t%c\t%c\r\n", j, evtptr[j].timestamp, evtptr[j].timestamp, (evtptr+j)->timestamp);
		}

		__HAL_TIM_SET_COUNTER(&timer1000k, 0);

		t1000 = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1    = HAL_GetTick();

		for(int j = 0; j < sizes[i]; j++){
			f_write(&fil, (evtptr+j), sizeof(TestPacket), &bw);
		}

		t1000_now = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1_now    = HAL_GetTick();

		if (sizeof(TestPacket) != bw)
			print("Error writing data to file:\r\n"
					"\t Chunk size: 0x%X\t bytes written: 0x%X\r\n",
					sizeof(TestPacket), bw);

		r1000 = calc_rate(t1000_now-t1000, block_size, 0.000001);
		r1 = calc_rate(t1_now-t1, block_size, 0.001);
		if(verbose){
			print("DeltaT_1000: %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1000_now-t1000, block_size, r1000);
			print("DeltaT_1   : %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1_now-t1, block_size, r1);
		}
		seq_wrate[i][0] = r1000;
		seq_wrate[i][1] = r1;

		free(evtptr);

		close_file(&fil);

		//evtptr;// = malloc(sizes[i] * sizeof(TestPacket));
//		for (unsigned j = 0; j < sizes[i]; j++) {
//			evtptr[j].timestamp = 0;
//			print("\tTS %u: 0x%X\r\n", j, evtptr[j].timestamp);
//		}

		if(verbose){
			print("READ\r\n");
			print("-----\r\n");
		}
		evtptr = malloc(block_size);
		for (unsigned j = 0; j < sizes[i]; j++)
			evtptr[j].timestamp = 0;

		open_file(&fil, filename_buffer, FA_READ);


//		if(verbose){
//			print("Blank data:\r\n");
//			for (unsigned j = 0; j < sizes[i]; j++) {
//				evtarray[j].timestamp = 0;
//				print("\tTS %u: 0x%X\r\n", j, evtarray[j].timestamp);
//			}
//			print("\n");
//		}

		//print("Reading data...\r\n");

		__HAL_TIM_SET_COUNTER(&timer1000k, 0);

		t1000 = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1    = HAL_GetTick();

		for(int j = 0; j < sizes[i]; j++)
			f_read(&fil, evtptr + j, sizeof(TestPacket), &br);

		t1000_now = __HAL_TIM_GET_COUNTER(&timer1000k);
		t1_now    = HAL_GetTick();


		r1000 = calc_rate(t1000_now-t1000, block_size, 0.000001);
		r1 = calc_rate(t1_now-t1, block_size, 0.001);
//		print("DeltaT_1000: %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1000_now-t1000, block_size, r1000);
//		print("DeltaT_1   : %10lu\t block_size: %10lu\t rate: %10lu\r\n", t1_now-t1, block_size, r1);
		seq_rrate[i][0] = r1000;
		seq_rrate[i][1] = r1;


//		//Capture rollovers.
//		for(unsigned j = 0; j < 2; j++)
//			if(seq_rrate[i][j] < 0.9 * seq_rrate[i][j+1] || seq_rrate[i][j] > 1.1*seq_rrate[i][j+1])
//				seq_rrate[i][j] = 0;


//		print("\tBlock size: 0x%X\t bytes read: 0x%X\r\n\n", block_size, br);
//		print("Readback data:\r\n");
		for (unsigned j = 0; j < sizes[i]; j++) {
			if(evtptr[j].timestamp != 65+(j % 100))
				print("Error in readback!\r\n");
			//print("\tTS %02u: 0x%X\t%c\r\n", j, evtptr[j].timestamp, evtptr[j].timestamp);
		}

		free(evtptr);
		close_file(&fil);
	}



	print("\n\n");

	//Dump results to a text file on the SD card to be read offline.
	sprintf(filename_buffer, "bandwidth_results.txt");
	open_file(&fil, filename_buffer, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);

	sprintf(text_buffer, "#Write Rates\r\n");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
			"I/O Type", "", "Sequential", "", "Block");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
			"----------", "----------", "----------", "----------", "----------");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
			"Reference", "1MHz", "1kHz", "1MHz", "1kHz");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
			"Data Size (B)", "", "Rate (B/s)", "", "Rate (B/s)");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
			"----------", "----------", "----------", "----------", "----------");
	f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	print(text_buffer);

	for(int i = 0; i < n; i++){
		sprintf(text_buffer, "0x%8X\t, %10lu\t, %10lu\t, %10lu\t, %10lu\r\n",
				(unsigned)sizes[i],
				seq_wrate[i][0], seq_wrate[i][1],
				block_wrate[i][0], block_wrate[i][1]);
		print(text_buffer);
		sprintf(text_buffer, "%10lu\t, %10lu\t, %10lu\t, %10lu\t, %10lu\r\n",
						sizes[i],
						seq_wrate[i][0], seq_wrate[i][1],
						block_wrate[i][0], block_wrate[i][1]);
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
	}
	print("\n\n\n");

	sprintf(text_buffer, "#Read Rates\r\n");
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
		print(text_buffer);

		sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
				"I/O Type", "", "Sequential", "", "Block");
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
		print(text_buffer);

		sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
				"----------", "----------", "----------", "----------", "----------");
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
		print(text_buffer);

		sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
				"Reference", "1MHz", "1kHz", "1MHz", "1kHz");
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
		print(text_buffer);

		sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
				"Data Size (B)", "", "Rate (B/s)", "", "Rate (B/s)");
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
		print(text_buffer);

		sprintf(text_buffer, "#%10s\t| %10s\t %10s\t| %10s\t %10s\r\n",
				"----------", "----------", "----------", "----------", "----------");
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);
		print(text_buffer);


	for(int i = 0; i < n; i++){
		sprintf(text_buffer, "0x%8X\t, %10lu\t, %10lu\t, %10lu\t, %10lu\r\n",
				(unsigned)sizes[i],
				seq_rrate[i][0], seq_rrate[i][1],
				block_rrate[i][0], block_rrate[i][1]);
		print(text_buffer);
		sprintf(text_buffer, "%10lu\t, %10lu\t, %10lu\t, %10lu\t, %10lu\r\n",
				sizes[i],
				seq_rrate[i][0], seq_rrate[i][1],
				block_rrate[i][0], block_rrate[i][1]);
		f_write(&fil, text_buffer, strlen(text_buffer), &bw);

	}


	close_file(&fil);
	unmount_fs();


	print("\n\n");

	return;

}
