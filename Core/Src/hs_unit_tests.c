#include <stdio.h>

#include "types.h" //typedefs
#include "hs_unit_tests.h"
#include "hs_types.h"

#include "hs_streamer.h"
#include "hs_readback.h"

extern void print(const char *fmt, ...);

G_STATUS UNIT_read_loop(u32 nloops, u8 PMT){


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



G_STATUS UNIT_write_loop(u32 nloops, u8 PMT, u16 nsamples){

	print("\n\nEntering test data generation loop.\r\n");
	print("----------------------------\r\n");

	int nhits_gen = 0;
	for(u32 i = 0; i < nloops; i++){

		for(u32 j = 0; j < 10; j++){
			spep = generate_dummy_SPEPacket(PMT, 0x400);
			current_hit_type=PL_SPE;
			add_hit_to_buffer();
		}

		for(u32 j = 0; j < 2; j++){
			mpep = generate_dummy_MPEPacket(PMT, nsamples);
			current_hit_type=PL_MPE;
			add_hit_to_buffer();
		}

		check_and_write_buffer(PMT, TRUE);


		print("----------------------------\r\n");
	}

	//closeout_file_buffers();
	print("Total hits generated: 0x%8lX\r\n"
		  "Total bytes written:  0x%8lX\r\n", nhits_gen, total_bytes_written);
	return G_OK;
}
