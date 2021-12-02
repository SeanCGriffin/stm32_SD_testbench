/*
 * types.h
 *
 *  Created on: Oct 26, 2021
 *      Author: sgriffin
 */

#ifndef INC_BASE_TYPES_H_
#define INC_BASE_TYPES_H_


#include <stdint.h>
#include <stdbool.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define False false
#define True true
#define FALSE false
#define TRUE true

typedef enum {
	G_OK,
	G_NOTOK
} G_STATUS;

typedef enum {
	STREAMER_RC_OK, 
	STREAMER_RC_EOF,
	STREAMER_RC_DISK_ERR
} STREAMER_RC;

typedef struct SystemTime{
//	uint64_t full;
//	struct {
//		uint32_t fine;
//		uint32_t coarse;
//	};
	uint32_t coarse;
	uint16_t fine;
} SystemTime;


#endif /* INC_BASE_TYPES_H_ */
