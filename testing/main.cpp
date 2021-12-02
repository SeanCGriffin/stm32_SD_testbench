#include "base_types.h"
#include "local_main.h"

#include <cstdio>
//#include <stdio.h> //FILE
#include <vector>

#include "streamer.h"
#include "packet.h"

extern "C" {
#include "ff_proxy.h"
#include "printer.h"
}

#include <sys/statvfs.h>

using namespace hitspool;

int main(void) {

	// FIL *fil = NULL;
	// FRESULT fres;
	// fres = f_open(fil, "hitspool/test.spool", 1);
	// print("fres = %d\r\n", fres);

	// u32 bw = 0;
	// u32 btw = 4;
	// f_write(fil, "ABCD", btw, &bw);
	// print("BTW: %d BW: %d\r\n", btw, bw);

	// return 0;

	// char test_filename[1024];
	// sprintf(test_filename, "hitspool/test.txt");
	// FIL fp;
	// // fp = *fopen("test.txt", "w+");
	// BYTE mode = get_mode_from_str((char*)"w+");
	// print("mode: %x\t desired: %x\r\n", mode,
	//       FA_CREATE_ALWAYS | FA_WRITE | FA_READ);

	// fflush(stdout);
	// f_open(&fp, test_filename, get_mode_from_str((char*)"w+"));

    G_STATUS gres = G_NOTOK;
    gres = hs_hit_io_unit_test();
    print("hs_unit_write_loop()\t %s (%d)\r\n", gres == G_OK ? "PASSED" : "FAILED", gres);

	// s->read_hit();

	// print("Hit: %d\r\n", sizeof(Hit));
	// print("SPEHit: %d\r\n", sizeof(SPEHit));
	// print("MPEHit: %d\r\n", sizeof(MPEHit));
	// mpep->hit->print_samples(100);

	struct statvfs fiData;

	// //Lets loopyloop through the argvs
	//   char path[] = "/";
	//   if((statvfs(path,&fiData)) < 0 ) {
	//       print("Failed to stat %s\r\n", path);
	//   } else {
	//       //cout << "\nDisk: " <<  argv[i];
	//       print("Block size:      0x%8X\r\n", fiData.f_frsize);
	//       print("Total no blocks: 0x%8X\r\n", fiData.f_blocks);
	//       print("Free blocks:     0x%8X\r\n", fiData.f_bfree);
	//       unsigned long long total_space = ((unsigned long long)fiData.f_blocks
	//       * (unsigned long long)fiData.f_frsize)/1.e9; unsigned long long
	//       free_space = ((unsigned long long)fiData.f_bfree  * (unsigned long
	//       long)fiData.f_frsize)/1.e9; print("Total size:      %lu\r\n",
	//       total_space); print("Free space:      %lu\r\n", free_space);
	//   }

	// print("Disk space available/total: %u/%u GB\r\n",
	// GetAvailableSpace("/")/1024/1024/1024, GetTotalSpace("/")/1024/1024/1024);

	print("Done.\r\n");
}