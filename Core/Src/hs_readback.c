/*
 * hs_readback.c
 *
 *  Created on: Oct 28, 2021
 *      Author: sgriffin
 */
//#include <hs_streamer.h> //FIXME: Maybe move the struct typedefs to types.

#define __HS_READBACK_C

#include <stdlib.h> //malloc
#include <stdio.h>
#include <string.h> //memcpy


#include "hs_readback.h"
#include "hs_streamer.h"

#ifdef PLATFORM_STANDALONE
	#pragma message( "Compiling " __FILE__ " for DESKTOP")
	#include "ff_proxy.h"
#else
	#pragma message( "Compiling " __FILE__ " for STM32")
	#include "ff.h"
#endif



FIL active_file;
// FRESULT f_op_res;

PayloadType_t mrr_hit_type = PL_INVALID;
SPEHit *mrr_speh; //Most recent read SPE hit.
MPEHit *mrr_mpeh; //Most recent read MPE hit.

// FRESULT open_source(char* path, BYTE mode){
// 	f_op_res = f_open(&active_file, path, mode);

// 	return f_op_res;
// }



/*
 * Read a hit from an open file.
 * The order of operations is to read in the base data,
 * check the first byte (which will be the Payload_t)
 * and respond according to what type of hit was placed
 * and how big the payload is.
 */
PayloadType_t read_next_hit(FIL *f){
	u8 buff[1024];
	u8* buffhead = buff;
	UINT btr = 0, br = 0, br_tot = 0;

	//This will capture the whole MPE hit (minus waveform).
	//If we need top capture a SPE hit we'll make an additional read.
	//This if/else deals with the fact that the structs may change.
	if(SPEBaseSize > MPEBaseSize)
		btr = MPEBaseSize;
	else
		btr = SPEBaseSize;

	f_read(f, buff, btr, &br);
	br_tot += br;

	u8 header_bits = buff[0];
	PayloadType_t pl_type = (PayloadType_t)((header_bits)&0xF)	;
//	print("header byte: %d\r\n", header_bits);
//	print("PayloadType_t: %d\r\n", pl_type);
//	print("PayloadType: %s\r\n", PLNameText[pl_type]);
	mrr_hit_type = pl_type;


	switch(pl_type){

		case PL_SPE:
			btr = SPEBaseSize - MPEBaseSize;
			//print("btr: %d\r\n", btr);
			f_read(f,  buffhead, btr, &br);

			if(btr != br){
				//We reached EOF, or something weird.
				//Abort reading.
				mrr_hit_type = PL_INVALID;
				return PL_INVALID;
			}

			br_tot += br;
			buffhead+=br;

			mrr_speh = (SPEHit*)malloc(SPEBaseSize);
			//print("br-tot%d\r\n", br_tot);
			memcpy(mrr_speh, buff, br_tot);
//			print_SPEHit((SPEHit*)buff);
//			print_SPEHit(mrr_speh);
			break;

		case PL_MPE:
			//MPEHit *m = (MPEHit*)malloc(MPEBaseSize);
			//memcpy(m, buff, br);
			//print_MPEHit((MPEHit*)buff);
			buffhead+=br;
			btr = ((MPEHit*)buff)->nsamples;
			f_read(f,  buffhead, btr, &br);

			if(btr != br){
				//We reached EOF, or something weird.
				//Abort reading.
				mrr_hit_type = PL_INVALID;
				return PL_INVALID;
			}


			//print("Waveform size: 0x%08X, br=: 0x%08X\r\n", btr, br);

			mrr_mpeh = (MPEHit*)malloc(MPEBaseSize + btr);
			memcpy(mrr_mpeh, buff, MPEBaseSize + btr);

			break;

		case PL_INVALID:
		default:
			return PL_INVALID;

	}

	return pl_type;

}
