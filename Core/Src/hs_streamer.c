/*
 * streamer.c
 *
 *  Created on: Oct 5, 2021
 *      Author: sgriffin
 */

#define __HS_STREAMER_C

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ioctl.h"


#ifdef PLATFORM_STANDALONE
	#pragma message( "Compiling " __FILE__ " for DESKTOP")
	#include "ff_proxy.h"
#else
	#pragma message( "Compiling " __FILE__ " for STM32")
	#include "ff.h"
#endif


#include <hs_streamer.h>
#include "types.h"
#include "hs_types.h"
#include "hs_readback.h"

extern void print(const char *fmt, ...);
extern SystemTime* get_system_time();

//https://www.youtube.com/watch?v=iVBCBcEANBc
#define REGISTER_ENUM(x) #x,
const char* PLNameText[] =
{
    #include "pl_names.h"
	"INVALID"

};
#undef REGISTER_ENUM


u32 tdc_reference = 10; //I can't remember what these are for.
u32 subsample_reference = 10; //Also this.

bool daq_enabled = 0; //This probably works as expected.


/*
 *
 * These variables are part of the ring bufffer system.
 *     hanler_active: flags indicating file is being written.
 *     handler_open: flags identifying if file is open.
 *     file_handlers: FATFS file handlers.
 *     live_filenames: filenames of the active files.
 *	   buffer_full: flags for which file buffer(s) to write.
 *
 */
char live_filenames[NUM_PMT][256]; //Filenames.
bool handler_active[NUM_PMT]; //Is file actively being written to?
bool handler_open[NUM_PMT];   //Is the file open?
FRESULT f_op_res[NUM_PMT];    //File operation results
FIL file_handlers[NUM_PMT];   //Actual file handles.
bool buffer_full[NUM_PMT];


extern u32 sys_seconds; //FIXME: This is for debugging for sure.
/*
 * These are for buffering wuBase data.
 * 		nhits_inbuff: number of hits in a buffer (debugging)
 * 		write_buff: buffer to hold wuBase data before being sent to f_write()
 * 		n_consumed: number of bytes in write_buff
 * 		n_written: last number of bytes written by f_write
 * 		write_head: pointer to location in write_buff where new data will be written.
 *
 */

u32 nhits_inbuff[NUM_PMT]; //FIXME: Does this work with wuBase buffers?
u8 write_buff[NUM_PMT][WRITEBUFFER_MAXISIZE];
u32 n_consumed[NUM_PMT];
UINT n_written[NUM_PMT];
u32 n_written_tot[NUM_PMT];
u8 *write_head[NUM_PMT];
u32 total_bytes_written = 0;

u32 nbuff_recv[NUM_PMT];
u32 n_spe_hits = 0;
u32 n_mpe_hits = 0;
u32 n_total_hits = 0;


u8 wub_buff[NUM_PMT][WUBASEBUFFER_MAXSIZE]; //Data buffers from wuBase.
SPEPacket *spep; //Active SPEPacket write struct.
MPEPacket *mpep; //Active MPEPacket write struct.
WUBPacket *wubp; //Active WuBase buffer write struct.


PayloadType_t current_hit_type = PL_INVALID; //Identifier.
PayloadType_t last_hit_type = PL_INVALID;

//Initialize the hit buffers
void init_hit_buffers(){

	spep = allocate_SPEPacket();
	spep->PMT = NUM_PMT-1;
	spep->hit.charge = 0xF;
	spep->trecv = get_system_time()->coarse;
	spep->hit.launch_t = 0xF;
	spep->hit.subsample_t = 0xF;
	spep->hit.tdc = 0xF;
	spep->hit.type = PL_SPE;

	mpep = allocate_MPEPacket(0);
	mpep->PMT = NUM_PMT-1;
	mpep->trecv = get_system_time()->coarse;
	mpep->hit.launch_t = 0xF;
	mpep->hit.tdc = 0xF;
	mpep->hit.type = PL_MPE;

	wubp = allocate_WUBPacket(0);
	wubp->PMT = NUM_PMT-1;
	wubp->trecv = get_system_time()->coarse;
	wubp->hits.type = PL_WUBUFF;

}

//Initialize the buffer write heads.
void init_write_heads(){
	for(int i = 0; i < NUM_PMT; i++){
		n_consumed[i] = 0;
		write_head[i] = write_buff[i];
	}
}

//FIXME: Filenames are going to need some work.
void init_file_handlers(){
	u32 inittime = get_system_time()->coarse;
	for(int i = 0; i < NUM_PMT; i++){
		nhits_inbuff[i] = 0;
		handler_active[i] = FALSE;
		buffer_full[i] = FALSE;
		sprintf(live_filenames[i], "hitspool/PMT%02d/0x%08X.spool", i, inittime);

		f_op_res[i] = f_open(&file_handlers[i], live_filenames[i], FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
		print("PMT%02d file opened with fres=(%d)\r\n", i, f_op_res[i]);
	}
	print("\n");
}

void print_IO_handlers(){
	print("PMT file handlers:\r\n");
	print("------------------\r\n");
	for(int i = 0; i < NUM_PMT; i++){
		print("PMT%02d:\r\n", i);
		print("\tbuffer_full:     %s\r\n", buffer_full[i] ? "TRUE" : "FALSE");
		print("\thandler_active:  %s\r\n", handler_active[i] ? "TRUE" : "FALSE");
		print("\tactive filename: %s\r\n", live_filenames[i]);
	}

	print("\nMost recent hit data:\r\n");
	print("---------------------\r\n");
	print_SPEPacket(spep);
	print_MPEPacket(mpep);
	print_WUBPacket(wubp);

}


//FIXME: Buffer size.
u16 calc_MPE_hit_size(MPEHit *mpe){
	return sizeof(MPEHit) + sizeof(u8) * (mpe->nsamples*1.0);
}

u16 calc_SPE_hit_size(SPEHit *spe){
	return sizeof(SPEHit);
}

u16 calc_WUB_hits_size(HitBuffer *wub){
	return sizeof(HitBuffer) + sizeof(u8) * (wub->size);
}


SPEPacket* allocate_SPEPacket(){
	return (SPEPacket*)malloc(sizeof(SPEPacket));
}

SPEPacket* generate_dummy_SPEPacket(u8 PMT, u16 dummy_charge){
	SPEPacket* spe = allocate_SPEPacket();
	spe->hit.type = PL_SPE;
	spe->hit.charge = dummy_charge;
	spe->hit.tdc = tdc_reference++;
	spe->hit.subsample_t = subsample_reference++;
	spe->hit.launch_t = get_system_time()->fine;
	return spe;
}

//FIXME: factor of 1.5 for ADC 12 bits. Need to fix this.
MPEPacket* allocate_MPEPacket(uint16_t nsamples){
	MPEPacket* m;
	m = malloc(sizeof(MPEPacket) + sizeof(uint8_t)*nsamples);
	m->hit.nsamples=nsamples;
	return m;
}

MPEPacket* generate_dummy_MPEPacket(u8 PMT, u16 nsamples, u8* buffer){
	MPEPacket* m = allocate_MPEPacket(nsamples);
	memcpy(m->hit.waveforms, buffer, nsamples);
	m->hit.type = PL_MPE;
	m->PMT = PMT;
	m->hit.nsamples = nsamples;
	return  m;
}


WUBPacket* allocate_WUBPacket(uint16_t size){
	WUBPacket* w;
	w = malloc(sizeof(WUBPacket) + sizeof(uint8_t)*size);
	w->hits.size=size;
	return w;
}

WUBPacket* generate_dummy_WUBPacket(u8 PMT, u16 size, u8* buffer){
	WUBPacket* w = allocate_WUBPacket(size);
	memcpy(w->hits.data, buffer, size);
	w->hits.type = PL_WUBUFF;
	w->PMT = PMT;
	w->hits.size = size;
	return  w;
}


void print_SPEPacket(SPEPacket *p){
	char buffer[1024];
	sprintf(buffer, "Packet info:\r\n"
			"\t Type:        %s\r\n"
			"\t PMT:         0x%8X\r\n"
			"\t trecv:       0x%8X\r\n"
			"\t launch_t:    0x%8d\r\n"
			"\t tdc:         0x%8X\r\n"
			"\t charge:      0x%8X\r\n"
			"\t subsample_t: 0x%8X\r\n",
			PLNameText[p->hit.type],
			p->PMT,
			p->trecv,
			p->hit.launch_t,
			p->hit.tdc,
			p->hit.charge,
			p->hit.subsample_t
			);
	print(buffer);
	return;
}

void print_SPEHit(SPEHit *hit){
	char buffer[1024];
	sprintf(buffer, "SPEHit info:\r\n"
			"\t launch_t:    0x%8d\r\n"
			"\t tdc:         0x%8X\r\n"
			"\t charge:      0x%8X\r\n"
			"\t subsample_t: 0x%8X\r\n",
			hit->launch_t,
			hit->tdc,
			hit->charge,
			hit->subsample_t
			);
	print(buffer);
	return;
}

void print_MPEHit(MPEHit *hit){
	char buffer[1024];
	sprintf(buffer, "MPEHit info:\r\n"
			"\t launch_t:    0x%8d\r\n"
			"\t tdc:         0x%8d\r\n"
			"\t nsamples:    0x%8X\r\n"
			"\t padding:     0x%8X\r\n",
			hit->launch_t,
			hit->tdc,
			hit->nsamples,
			hit->padding
			);
	print(buffer);
	return;
}

void print_MPEPacket(MPEPacket *p){
	char buffer[1024];
	sprintf(buffer, "Packet info:\r\n"
			"\t Type:        %s\r\n"
			"\t PMT:         0x%8X\r\n"
			"\t trecv:       0x%8X\r\n"
			"\t launch_t:    0x%8d\r\n"
			"\t tdc:         0x%8d\r\n"
			"\t nsamples:    0x%8X\r\n",
			PLNameText[p->hit.type],
			p->PMT,
			p->trecv,
			p->hit.launch_t,
			p->hit.tdc,
			p->hit.nsamples
			);
	print(buffer);
	return;
}

void print_WUBPacket(WUBPacket *p){
	char buffer[1024];
	sprintf(buffer, "Packet info:\r\n"
			"\t Type:        %s\r\n"
			"\t PMT:         0x%8X\r\n"
			"\t trecv:       0x%8X\r\n"
			"\t size:        0x%8d\r\n",
			PLNameText[p->hits.type],
			p->PMT,
			p->trecv,
			p->hits.size
			);
	print(buffer);
	return;
}


void closeout_file_buffers(){

	print("\r\nWriting partial buffers.\r\n");
	print("----------------------------\r\n");
	for(int i = 0; i < NUM_PMT; i++){
		check_and_write_buffer(i, TRUE);
	}
	print("---------------------------\r\n");

	for(int i = 0; i < NUM_PMT; i++){
		nhits_inbuff[i] = 0;
		write_head[i] = write_buff[i];
		handler_active[i] = FALSE;
		buffer_full[i] = FALSE;
		sprintf(live_filenames[i], "nope");
		f_op_res[i] = f_close(&file_handlers[i]);
		print("PMT%02d file closed with fres=(%d)\r\n", i, f_op_res[i]);
	}
	print("\n");
}

//FIXME: Return an FSTATUS and pass the n_written by reference.
u32 check_and_write_buffer(u8 PMT, bool force){
	if((n_consumed[PMT] >= TARGET_BLOCKSIZE) || force){
		//Kick off the file write process.

		print("PMT%02d buffer meets threshold.\r\n"
			  "\t n_consumed: 0x%04X\r\n", PMT, n_consumed[PMT]);

		//do some writing things
		buffer_full[PMT] = TRUE;
		handler_active[PMT] = TRUE;
		f_op_res[PMT] = f_write(&file_handlers[PMT], write_buff[PMT], n_consumed[PMT], &n_written[PMT]);

		if(f_op_res[PMT] != FR_OK){
			print("Error writing file %s!\r\n", live_filenames[PMT]);
		}

		print("written: now: 0x%04X\ttot: 0x%04X\r\n", n_consumed[PMT], n_written[PMT]);
		n_written_tot[PMT] += n_written[PMT];
		total_bytes_written += n_written[PMT];

		//reset write heads
		n_consumed[PMT] = 0;
		write_head[PMT] = write_buff[PMT];

		//release the file handler
		buffer_full[PMT] = FALSE;
		handler_active[PMT] = FALSE;
	}

	return n_written[PMT];
}

/*
 * Based on the last hit(s) received, put that data in a buffer.
 */
u32 add_hit_to_buffer(){

	u8 PMT = 0;
	u16 write_size = 0;
	print("Current hit type: %s\r\n", PLNameText[current_hit_type]);

	switch(current_hit_type){
		case PL_SPE:
			write_size = calc_SPE_hit_size(&(spep->hit));
			PMT = spep->PMT;
			memcpy(write_buff[PMT], (u8*)&(spep->hit), write_size);
			write_head[PMT]+=write_size;
			n_consumed[PMT]+=write_size;
			nhits_inbuff[PMT]++;
			break;
		case PL_MPE:
			write_size = calc_MPE_hit_size(&(mpep->hit));
			PMT = mpep->PMT;
			memcpy(write_buff[PMT], (u8*)&(mpep->hit), write_size);
			write_head[PMT]+=write_size;
			n_consumed[PMT]+=write_size;
			nhits_inbuff[PMT]++;
			break;
		case PL_WUBUFF:
			write_size = calc_WUB_hits_size(&(wubp->hits));
			PMT = wubp->PMT;
			memcpy(write_buff[PMT], (u8*)&(wubp->hits), write_size);
			write_head[PMT]+=write_size;
			n_consumed[PMT]+=write_size;
			//nhits_inbuff[PMT]++;
			break;
		case PL_INVALID:
			print("Invalid hit type!\r\n");
			return 1;
		default:
			break;

	}

	current_hit_type = PL_INVALID;
	print("just wrote 0x%04X bytes to PMT%02d buffer.\r\n", write_size);

	check_and_write_buffer(PMT, FALSE);

	return FR_OK;
}









GSTATUS UNIT_read_loop(u32 nloops, u8 PMT, u16 nsamples){
	u8 sample_buf[nsamples];
	for(int i = 0; i < nsamples;i++)
		sample_buf[i] = i;

	print("\n\nEntering test data readback loop.\r\n");
	print("----------------------------\r\n");

	//Pattern for readback should be 10 SPEs
	//Followed by 2 MPEs.
	//And repeat.

	while(1){

		for(int j = 0; j < 10; j++){
			read_next_hit(&file_handlers[j]);
			switch(mrr_hit_type){
				case PL_SPE:
					if(mrr_speh->tdc != mrr_speh->subsample_t)
						return G_NOTOK;
					break;
				case PL_MPE:

				default:
					return G_NOTOK;

			}
		}

	}

	return G_OK;

}



GSTATUS UNIT_write_loop(u32 nloops, u8 PMT, u16 nsamples){
	u8 sample_buf[nsamples];
	for(int i = 0; i < nsamples;i++)
		sample_buf[i] = i;

	print("\n\nEntering test data generation loop.\r\n");
	print("----------------------------\r\n");

	int nhits_gen = 0;
	for(int i = 0; i < nloops; i++){

		for(int j = 0; j < 10; j++){
			spep = generate_dummy_SPEPacket(2, 0x400);
			current_hit_type=PL_SPE;
			add_hit_to_buffer();
		}

		for(int j = 0; j < 2; j++){
			mpep = generate_dummy_MPEPacket(PMT, nsamples, sample_buf);
			current_hit_type=PL_MPE;
			add_hit_to_buffer();
		}

		check_and_write_buffer(PMT, TRUE);


		print("----------------------------\r\n");
	}

	//closeout_file_buffers();
	print("Total hits generated: 0x%8lX\r\n"
		  "Total bytes written:  0x%8lX\r\n", nhits_gen, total_bytes_written);
	return GOK;
}
