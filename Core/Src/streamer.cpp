/*
 * streamer.cpp
 *
 *  Created on: Nov 15, 2021
 *      Author: sgriffin
 */
#include <cstdio>
#include <cstdlib> //malloc, free
#include <cstring>

#include "packet.h" //SPE/MPE/WUBPacket contents.
#include "streamer.h"

#include <bitset> //Byte to bits

#ifdef PLATFORM_STANDALONE
#pragma message("Compiling " __FILE__ " for DESKTOP")
#define f_read(a, b, c, d) f_read(&a, b, c, d)
extern "C" {
#include "printer.h"
}
#else
#pragma message("Compiling " __FILE__ " for STM32")
extern void print(const char *fmt, ...);
#endif

// https://www.youtube.com/watch?v=iVBCBcEANBc
#define REGISTER_ENUM(x) #x,
const char *PLNameText[] = {
#include "pl_names.h"
	"INVALID"

};
#undef REGISTER_ENUM

namespace hitspool {

// streamer class members

streamer::streamer() {

	for (int i = 0; i < NUM_PMT; i++) {
		nhits_inbuff[i] = 0;
		sprintf(live_filenames[i], "NULL");
		handler_active[i] = false;
		handler_open[i] = false;

		n_consumed[i] = 0;
		buffer_full[i] = false;

		n_written[i] = 0;
		n_written_tot[i] = 0;
		total_bytes_written = 0;
	}
}

streamer::~streamer() {
	// TODO Auto-generated destructor stub
	// last_hit_type = PL_INVALID;
	// free(spep);
	// free(mpep);
	// free(wubp);
}

void streamer::init_write_heads() {
	for (int i = 0; i < NUM_PMT; i++) {
		sprintf((char *)write_buff[i], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
		n_consumed[i] = 0;
		write_head[i] = (u8 *)write_buff[i];
	}
}

// Print the first 12 bytes of the write buffer and the write head.
void streamer::print_buffer_heads() {
	print("Buffer contents\r\n");
	print("-----------------------------------\r\n");
	char buff[6];
	for (int i = 0; i < NUM_PMT; i++) {
		print("PMT%02d:\r\n", i);
		print("Address of buffer[%02d] is %p\n", i, (void *)write_buff[i]);
		print("Address of head[%02d]   is %p\n", i, (void *)write_head[i]);
		memcpy(buff, write_head[i], 6);
		print("\tBuffer: %s\r\n"
			  "\tHead:   %s\r\n",
			  write_buff[i], buff);
	}
}

void streamer::init_file_handlers(u32 inittime) {

	for (int i = 0; i < NUM_PMT; i++) {
		nhits_inbuff[i] = 0;
		handler_active[i] = FALSE;
		buffer_full[i] = FALSE;
		sprintf(live_filenames[i], "hitspool/PMT%02d/0x%08lX.spool", i, inittime);

		f_op_res[i] =
			f_open(&file_handlers[i], live_filenames[i], 
				FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
		print("PMT%02d file opened with fres=(%d)\r\n", i, f_op_res[i]);
	}
	// print("\n");
}

void streamer::flush_file_handlers() {
	for (int i = 0; i < NUM_PMT; i++) {
		f_op_res[i] = f_sync(&file_handlers[i]);
		if (f_op_res[i] != FR_OK)
			print("ERROR syncing file %s; fres = %d\r\n", live_filenames[i], f_op_res[i]);
	}
}

void streamer::close_file_handlers() {

	for (int i = 0; i < NUM_PMT; i++) {
		f_op_res[i] = f_close(&file_handlers[i]);
		if (f_op_res[i] != FR_OK)
			print("ERROR closing file %s; fres = %d\r\n", live_filenames[i], f_op_res[i]);
	}
	print("Done closing all file handlers.\r\n");
}

void streamer::print_IO_handlers() {

	print("PMT file handlers:\r\n");
	print("------------------\r\n");
	for (int i = 0; i < NUM_PMT; i++) {
		print("PMT%02d:\r\n", i);
		// print("\tbuffer_full:     %s\r\n", buffer_full[i] ? "TRUE" : "FALSE");
		// print("\thandler_active:  %s\r\n", handler_active[i] ? "TRUE" : "FALSE");
		// print("\tactive filename: %s\r\n", live_filenames[i]);
		print("\t----------------------------\r\n");
	}

	// print("\nMost recent hit data:\r\n");
	// print("---------------------\r\n");
	// print_SPEPacket(spep);
	// print_MPEPacket(mpep);
	// print_WUBPacket(wubp);
}

// FIXME: Return an FSTATUS and pass the n_written by reference.
u32 streamer::check_and_write_buffer(u8 PMT, bool force) {
	if ((n_consumed[PMT] >= TARGET_BLOCKSIZE) || force) {
		// Kick off the file write process.
		if (force) {
			print("Forcing write of PMT%02d buffer.\r\n"
				  "\t n_consumed: 0x%04X\r\n",
				  PMT, n_consumed[PMT]);
		} else {
			print("PMT%02d buffer meets threshold.\r\n"
				  "\t n_consumed: 0x%04X\r\n",
				  PMT, n_consumed[PMT]);
		}

		// do some writing things
		buffer_full[PMT] = TRUE;
		handler_active[PMT] = TRUE;
		f_op_res[PMT] =
			f_write(&file_handlers[PMT], write_buff[PMT], n_consumed[PMT], &n_written[PMT]);

		if (f_op_res[PMT] != FR_OK) {
			print("Error writing file %s!\r\n", live_filenames[PMT]);
		}

		n_written_tot[PMT] += n_written[PMT];
		total_bytes_written += n_written[PMT];
		print("written: now: 0x%04X\ttot/PMT: 0x%04X\ttot: 0x%04X\r\n", n_consumed[PMT],
			  n_written[PMT], n_written_tot[PMT]);

		// reset write heads
		n_consumed[PMT] = 0;
		write_head[PMT] = write_buff[PMT];

		// release the file handler
		buffer_full[PMT] = FALSE;
		handler_active[PMT] = FALSE;
	}

	return n_written[PMT];
}

STREAMER_RC streamer::read_next_hit(FIL *file, PayloadType_t *type, u8 *hitbuffer) {
	/*
	 * Read the next hit from the filename buffer.
	 */

    print("read_next_hit()\r\n");
	FRESULT fres;
	STREAMER_RC SMR_RC = STREAMER_RC_OK;
	u8 lead[sizeof(SPEHit)]; // this is the size of a SPEHit and MPEHit base unit.
	u8 data[1024];			 // FIXME: Make this match the maximum number of samples in an MPEHit.
	UINT br = 0;
	UINT btr = 6; // or sizeof(MPEHit)

	fres = f_read(file, lead, btr, &br);
	if (fres != FR_OK) {
		if (br == 0) // EOF          
			return STREAMER_RC_EOF;
		else{ // We read out fewer bytes than expected.
            print("Error reading from file; br=%d btr=%d\r\n", br, btr);
			return STREAMER_RC_DISK_ERR;
        }
	}
	// for(int i = 0; i < sizeof(SPEHit); i++){
	//    print("%s ", toBinaryString(lead[i]).c_str());
	// }
	// print("\n");
	memcpy(data, lead, 6); // the rest of data will be filled with event data.
	// data[7] = '\0';
	// print("%s\r\n", data);
	*type = static_cast<PayloadType_t>(lead[0] & 0x3);

	size_t s = 0;
	u16 nsamples = 0;
	MPEHit *mpe;
	SPEHit *spe;

	switch (*type) {
	case PL_SPE:
		s = sizeof(SPEHit);
		hitbuffer = (u8 *)malloc(s);
		memcpy(hitbuffer, lead, s);
		SMR_RC = STREAMER_RC_OK;
        spe = (SPEHit*)hitbuffer;
        print("%s\r\n", spe->tostring().c_str());
		break;

	case PL_MPE:
		//print("Read type: %s\r\n", PLNameText[*type]);
		nsamples = ((MPEHit *)lead)->nsamples;
		//print("nsamples from cast: %d\r\n", nsamples);

		s = sizeof(MPEHit) + sizeof(u16) * 2 * ((MPEHit *)lead)->nsamples;
		s--; // subtract 1 because the first data byte is built into MPEHit already.
		//print("Total size: 0x%X\r\n", s);
		btr = s - sizeof(MPEHit);

		fres = f_read(file, data + 6, btr, &br);
		//print("btr = %d\t br= %d\r\n", btr, br);
		if (fres != FR_OK) {
			// We read out fewer bytes than expected.
			return STREAMER_RC_DISK_ERR;
		}
		hitbuffer = (u8 *)malloc(s);
		memcpy(hitbuffer, data, s);
		mpe = (MPEHit *)hitbuffer;
		print("%s\r\n", mpe->tostring().c_str());
		mpe->print_samples(200);

		SMR_RC = STREAMER_RC_OK;
		break;

	default:
		SMR_RC = STEAMER_RC_TYPE_ERR;
	}
	//print("%d\r\n", SMR_RC);
	return SMR_RC;
}

G_STATUS hs_hit_io_unit_test() {

	print("-----------------------------------\r\n");
	print("---- hs_hit_io_unit_test() ----\r\n");
	print("-----------------------------------\r\n\n");

	streamer *s = new streamer();

	print("Initializing write buffers, heads... ");
	s->init_write_heads();
	print("Done.\r\n");
	print("Initializing file handlers...\r\n");
	s->init_file_handlers(0xFF000000);
	print("File handlers done.\r\n");
	print("-----------------------------------\r\n");
	s->print_buffer_heads();
	print("-----------------------------------\r\n");

    print("Hit: %d\t SPEHit size: %d\t MPEHit size: %d\r\n", sizeof(Hit), sizeof(SPEHit), sizeof(MPEHit) - 1);
	u16 nsamples = 10;
	u16 waveform_buffer[2 * nsamples];
	for (int i = 0; i < 2 * nsamples; i++)
		waveform_buffer[i] = 2 * nsamples - i;

	// For testing....
	char pattern[7];
	sprintf(pattern, "SCGPHD");
	SPEHit *spe_pattern = (SPEHit *)pattern; // new SPEHit(0xABCD, 0x5, 0xA, 0xF);
	SPEHit *speh = new SPEHit(0xABCD, 0x5, 0xA, 0xF);

	MPEHit *mpeh = new (nsamples) MPEHit(0xCDEF, 0x4, nsamples, (u8 *)waveform_buffer);
    print("Test MPEHit predicted size: 0x%X + 0x%X = 0x%X\r\n", 
    sizeof(MPEHit)-1, 2*nsamples*sizeof(u16), 
    sizeof(MPEHit)-1 + 2*nsamples*sizeof(u16));
    print("Test MPEHit calculated size: 0x%X\r\n", mpeh->calc_size());

	// mpeh->print_samples(5);
	//  u8* hitbytes = (u8*)malloc(mpeh->calc_size() + speh->calc_size());
	//  memcpy(hitbytes, (u8*)mpeh, mpeh->calc_size());
	//  memcpy(hitbytes + mpeh->calc_size(), (u8*)mpeh, mpeh->calc_size());
	//  WUBuffer* wubuff = new WUBuffer(mpeh->calc_size() + speh->calc_size(), 2,
	//  hitbytes);
	//  //printf("%s\r\n", speh->tostring().c_str());

	hitpacket<SPEHit> *spep = new hitpacket<SPEHit>(0, 0x1234ABCD, speh);

	hitpacket<MPEHit> *mpep = new hitpacket<MPEHit>(0, 0x1234ABCD, mpeh);

	hitpacket<SPEHit> *spep_pattern = new hitpacket<SPEHit>(0, 0x1234ABCD, spe_pattern);
	spep_pattern->hit->pl_type = PL_SPE;

	// s->add_hit(spep_pattern);
	// s->add_hit(spep_pattern);
	int nhits_to_write = 5;
	int nhits_written = 0;
	for (int i = 0; i < nhits_to_write; i++) {
		spep->hit->tdc++;
		s->add_hit(spep);
		nhits_written++;
	}

	
    for (int i = 0; i < 5; i++){
        ((u16*)(mpep->hit->waveform))[i%nsamples]+=100;
        mpep->hit->tdc++; 
        s->add_hit(mpep); nhits_written++;
        //print("Added 0x8%X bytes to buffer.\r\n", mpep->hit->calc_size());
    }
    


	// Flush the write buffers and close the files.
	for (int i = 0; i < NUM_PMT; i++) {
		s->check_and_write_buffer(i, true);
	}
	s->close_file_handlers();

	// s->print_buffer_heads();
	print("-----------------------------------\r\n");
	print("Opening PMT0 file for reading.\r\n");


	FRESULT fres = f_open(&(s->file_handlers[0]), s->live_filenames[0], FA_READ);
	if (fres != FR_OK)
		print("Error opening file for reading.\r\n");

	print("Reading hits...\r\n");
	print("-----------------------------------\r\n");

	STREAMER_RC read_status = STREAMER_RC_OK;
	PayloadType_t next_hit_type;
	u8 *next_hit_contants = NULL;
	u8 rbuff[1024];
	UINT br = 0;
	UINT btr = 6; // or sizeof(MPEHit)

	int nhits_read = 0;
	while (1) {
		// print("Attempting to read hit %d\r\n", nhits_read);
#ifdef PLATFORM_STANDALONE
		read_status = s->read_next_hit((s->file_handlers[0]), &next_hit_type, next_hit_contants);
#else
		read_status = s->read_next_hit(&(s->file_handlers[0]), &next_hit_type, next_hit_contants);
#endif
		if (read_status != STREAMER_RC_OK) {
			if (read_status == STREAMER_RC_EOF) {
				print("Reached EOF; exiting read.\r\n");
			} else {
				print("Streamer exited with code %d\r\n", read_status);
			}

			break;
		} else {
			nhits_read++;
		}
		if (next_hit_contants != NULL)
			free(next_hit_contants);
	}
	if (nhits_read == nhits_written)
		return G_OK;
	else
		return G_NOTOK;
}

} /* namespace hitspool */
