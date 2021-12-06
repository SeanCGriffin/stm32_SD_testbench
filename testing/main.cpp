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
    return 0;
	// s->read_hit();

	// print("Hit: %d\r\n", sizeof(Hit));
	// print("SPEHit: %d\r\n", sizeof(SPEHit));
	// print("MPEHit: %d\r\n", sizeof(MPEHit));
	// mpep->hit->print_samples(100);

	struct statvfs fs_buffer;

    const unsigned int GB = (1024 * 1024) * 1024;

    char dirpath[1024];
    sprintf(dirpath, "/Users/sgriffin");
    int ret = statvfs(dirpath, &fs_buffer);

    if (!ret) {
        double blocks = fs_buffer.f_blocks;
        double total = (double)(fs_buffer.f_blocks * fs_buffer.f_frsize) / GB;
        double available = (double)(fs_buffer.f_bfree * fs_buffer.f_frsize) / GB;
        double used = total - available;
        double usedPercentage = (double)(used / total) * (double)100;
        printf("Blocks:          %8.3f\r\n", blocks);
        printf("Total:           %8.3f --> %.0f\n", total, total);
        printf("Available:       %8.3f --> %.0f\n", available, available);
        printf("Used:            %8.3f --> %.0f\n", used, used);
        printf("Used Percentage: %8.3f --> %.0f\n", usedPercentage, usedPercentage);
    }


	print("Done.\r\n");
}
