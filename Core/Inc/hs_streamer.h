/*
 * streamer.h
 *
 *  Created on: Oct 5, 2021
 *      Author: sgriffin
 */

#ifndef SRC_HS_STREAMER_H_
#define SRC_HS_STREAMER_H_

#include "types.h" //typedefs
#include "hs_types.h"



#define TARGET_BLOCKSIZE 4096 //Target size for writing to disk. More of a minimum than anything.
#define WRITEBUFFER_MAXISIZE TARGET_BLOCKSIZE+512 //Some fraction of an MPE hit.
#define WUBASEBUFFER_MAXSIZE (5*(sizeof(MPEHit) + 36)) //Assume 5 MPE hits sent at once?? Will need optimizing.


//Hit generation and definition stuff.
u16 calc_SPE_hit_size(SPEHit *spe);
u16 calc_MPE_hit_size(MPEHit *mpe);
u16 calc_WUB_hits_size(HitBuffer *wub);

SPEPacket* allocate_SPEPacket();
MPEPacket* allocate_MPEPacket(uint16_t nsamples);
WUBPacket* allocate_WUBPacket(uint16_t size);

void print_SPEPacket(SPEPacket *spe);
void print_SPEHit(SPEHit *hit);
void print_MPEPacket(MPEPacket *p);
void print_MPEHit(MPEHit *hit);
void print_WUBPacket(WUBPacket *p);


SPEPacket* generate_dummy_SPEPacket(u8 PMT, u16 dummy_charge);
MPEPacket* generate_dummy_MPEPacket(u8 PMT, u16 nsamples, u8* buffer);
WUBPacket* generate_dummy_WUBPacket(u8 PMT, u16 size, u8* buffer);


//Incoming hit handlers.
u32 get_hit();
u32 add_hit_to_buffer();

//Hit buffers and file I/O initialization things.
void init_hit_buffers();
void init_write_heads();
void init_file_handlers();
void print_IO_handlers();
u32 check_and_write_buffer(u8 PMT, bool force);
void closeout_file_buffers();

//Testing
GSTATUS UNIT_write_loop(u32 nloops, u8 PMT, u16 nsamples);
GSTATUS UNIT_read_loop(u32 nloops, u8 PMT, u16 nsamples)

#endif /* SRC_HS_STREAMER_H_ */
