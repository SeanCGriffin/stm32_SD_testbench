/*
 * hs_types.h
 *
 *  Created on: Oct 28, 2021
 *      Author: sgriffin
 */

#ifndef INC_HS_TYPES_H_
#define INC_HS_TYPES_H_

#include "types.h"

#define NUM_PMT 5

#define REGISTER_ENUM(x) x,
typedef enum {
	#include "pl_names.h"
	PL_INVALID
} PayloadType_t;
#undef REGISTER_ENUM

extern const char* PLNameText[];


typedef struct {
	PayloadType_t type : 4; //could reduce to 2, and add bits to length in future.
	uint16_t payload_length : 12;
} __attribute__((packed)) PacketHeader; //Packs into 2 bytes.

typedef struct {
	PacketHeader header;
	uint8_t timestamp[6]; //48 bits
} __attribute__((packed)) TimestampPacket;

typedef struct {
	PayloadType_t type  : 8;  //SPE, MPE, timestamp, other
	u16 charge     : 12; //extracted charge
	u8 tdc         : 6;  //TDC value
	u8 subsample_t : 7;  //extracted subsample time
	u16 launch_t   : 16; //ADC launch time (IST)
	u8 padding     : 7;  //We have seven extra bits.
	//Total size: 56 bits / 7 bytes
} __attribute__((packed)) SPEHit;

typedef struct {
	PayloadType_t type : 8;
	u16 nsamples : 12;  //Good to 512 samples per channel, assuming 12-bits.
	u16 launch_t : 16;  //ADC launch time (IST)
	u8 tdc : 6;         //TDC value
	u8 padding : 6;     //We have six extra bits.
	uint8_t waveforms[]; //Channel 1 then Channel 2
	//Total size: 48 bytes / 6 bytes

}__attribute__((packed)) MPEHit;

//typedef struct {
//	PayloadType_t type;
//	u16 nsamples;  //Good to 512 samples per channel, assuming 12-bits.
//	u16 launch_t;  //ADC launch time (IST)
//	u8 tdc;         //TDC value
//	u8 padding;     //We have six extra bits.
//	uint8_t waveforms[]; //Channel 1 then Channel 2
//	//Total size: 48 bytes / 6 bytes
//
//}MPEHit;


#define MPEBaseSize sizeof(MPEHit)
#define SPEBaseSize sizeof(SPEHit)

typedef struct {
	PayloadType_t type : 8;
	u16 size : 12;
	u8 padding: 4;
	uint8_t data[];

}__attribute__((packed)) HitBuffer;

typedef struct {
	uint8_t PMT;
	uint32_t trecv;
	SPEHit hit;
} SPEPacket;

typedef struct {
	uint8_t PMT;
	uint32_t trecv;
	MPEHit hit;
} MPEPacket;

typedef struct {
	uint8_t PMT;
	uint32_t trecv;
	HitBuffer hits;
} WUBPacket;


#endif /* INC_HS_TYPES_H_ */
