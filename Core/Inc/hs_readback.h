/*
 * hs_readbach.h
 *
 *  Created on: Oct 28, 2021
 *      Author: sgriffin
 */

#ifndef __HS_READBACK_H_
#define __HS_READBACK_H_

#ifdef PLATFORM_STANDALONE
	#pragma message( "Compiling " __FILE__ " for DESKTOP")
	#include "ff_proxy.h"
#else
	#pragma message( "Compiling " __FILE__ " for STM32")
	#include "ff.h"
#endif


#include "hs_types.h"

extern PayloadType_t mrr_hit_type;
extern SPEHit *mrr_speh;
extern MPEHit *mrr_mpeh;

FRESULT open_source(char* path, BYTE mode);//FIXME: Probably meaningless
PayloadType_t read_next_hit(FIL *f);

#endif /* __HS_READBACK_H_ */
