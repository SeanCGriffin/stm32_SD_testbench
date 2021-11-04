/*
 * hs_readbach.h
 *
 *  Created on: Oct 28, 2021
 *      Author: sgriffin
 */

#ifndef INC_HS_READBACK_H_
#define INC_HS_READBACK_H_

#ifdef PLATFORM_STANDALONE
	#pragma message( "Compiling " __FILE__ " for DESKTOP")
	#include "ff_proxy.h"
#else
	#pragma message( "Compiling " __FILE__ " for STM32")
	#include "ff.h"
#endif


#include "hs_types.h"

FRESULT open_source(char* path, BYTE mode);//FIXME: Probably meaningless
PayloadType_t read_next_hit(FIL *f);

#endif /* INC_HS_READBACK_H_ */
