/*
 * streamer.h
 *
 *  Created on: Nov 15, 2021
 *      Author: sgriffin
 */

#ifndef STREAMER_H_
#define STREAMER_H_

#ifdef PLATFORM_STANDALONE
    #pragma message( "Compiling " __FILE__ " for DESKTOP")
    extern "C" {
        #include "ff_proxy.h"
        #include "printer.h"
    }
#else
    #pragma message( "Compiling " __FILE__ " for STM32")
    #include "ff.h"
#endif

#define NUM_PMT 1
#define TARGET_BLOCKSIZE 4096 //Target size for writing to disk. More of a minimum than anything.
#define WRITEBUFFER_MAXISIZE TARGET_BLOCKSIZE+512 //Some fraction of an MPE hit.
#define WUBASEBUFFER_MAXSIZE (5*(sizeof(MPEHit) + 36)) //Assume 5 MPE hits sent at once?? Will need optimizing.


#include "hs_types.h"
#include "base_types.h"

#include "packet.h"
#include <bitset>
#include <string>
#include <sstream>

namespace hitspool {

    G_STATUS hs_hit_io_unit_test();

    class streamer {

        private:
        public:
            char all_files[NUM_PMT];
            //Hit counters
            u16 nhits_inbuff[NUM_PMT];

            //File handlers
            char live_filenames[NUM_PMT][256]; //Filenames.
            bool handler_active[NUM_PMT]; //Is file actively being written to?
            bool handler_open[NUM_PMT];   //Is the file open?            

            FRESULT f_op_res[NUM_PMT];  //File operation results. 
            FIL file_handlers[NUM_PMT]; //Active file handles. 

            //File I/O buffers
            u8 write_buff[NUM_PMT][WRITEBUFFER_MAXISIZE]; //Active writing buffer
            u8* write_head[NUM_PMT];    //Pointer to write head in write_buff.
            u32 n_consumed[NUM_PMT];    //Number of bytes consumed in active buffer (should be write_head - write_buff).
            bool buffer_full[NUM_PMT]; //Is the file's I/O buffer full / past threshold? 

            //Byte trackers.            
            UINT n_written[NUM_PMT];    //Number of bytes written in most recent output operation.
            u32 n_written_tot[NUM_PMT]; //Total number of bytes written to the active file handles. 
            u64 total_bytes_written;    //Sum total of number of bytes written to all files. 

            

            u64 bytes_avail; //Bytes available in filesystem.
            u64 bytes_total; //Full size of available filesystem (bytes)

            

        	streamer();
        	~streamer(); //FIXME: Figure out why this was considered virtual.

        	//Hit buffers and file I/O initialization things.
        	void init_write_heads();
            void print_buffer_heads();
        	void init_file_handlers(u32 inittime);
        	void flush_file_handlers();
        	void close_file_handlers();
        	void fstat_file_handlers();
        	void print_IO_handlers();

            STREAMER_RC read_next_hit(FIL* file, PayloadType_t *type, u8* hitbuffer);

            u32 check_and_write_buffer(u8 PMT, bool force);            

            template <typename T> u32 add_hit(T* hit_packet){
                u8 PMT = hit_packet->PMT; 
                u16 write_size = hit_packet->hit->calc_size();
                memcpy(write_head[PMT], (u8*)hit_packet->hit, write_size);
                write_head[PMT]+=write_size;
                n_consumed[PMT]+=write_size;
                nhits_inbuff[PMT]++;
                
                check_and_write_buffer(PMT, false);
                return 0;
            };

            //https://stackoverflow.com/questions/7349689/how-to-print-using-cout-a-number-in-binary-form
            //Useful for presenting bytes as strings.
            template<typename T>
            static std::string toBinaryString(const T& x){
                std::stringstream ss;
                ss << std::bitset<sizeof(T) * 8>(x);
                return ss.str();
            }
            
        	

    };


} /* namespace hitspool */

#endif /* STREAMER_H_ */
