/*
 * hs_types.h
 *
 *  Created on: Oct 28, 2021
 *      Author: sgriffin
 */

#ifndef INC_HS_TYPES_H_
#define INC_HS_TYPES_H_

#define REGISTER_ENUM(x) x,
typedef enum {
	#include "pl_names.h"
	PL_INVALID
} PayloadType_t;
#undef REGISTER_ENUM

//FIXME: move PLNameText[] definition somewhere better. Currently defined in streamer.cpp
extern const char* PLNameText[];


#endif /* INC_HS_TYPES_H_ */
