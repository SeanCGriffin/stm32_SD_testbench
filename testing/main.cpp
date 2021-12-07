#include "base_types.h"
//#include "local_main.h"

//#include <stdio.h> //FILE
//#include <vector>

#include "packet.h"
#include "streamer.h"

#include "ff_proxy.h"

extern "C" {
#include "printer.h"
}

#include <sys/statvfs.h>

using namespace hitspool;

int main(int argc, char *argv[]){

	// int retVal;
	// FILE *fp;
	// char buffer[] = "Writing to a file using fwrite.";

	// fp = fopen("data.txt", "w");
	// retVal = fwrite(buffer, sizeof(buffer), 1, fp);

	// print("fwrite returned %d\r\n", retVal);

	// FIL *fil = NULL;
	// FRESULT fres;
	// fres = f_open(fil, "hitspool/test.spool", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	// print("fres = %d\r\n", fres);

	// u32 bw = 0;
	// u32 btw = 4;
	// print("BTW: %d BW: %d\r\n", btw, bw);
	// fflush(stdout);
	// return 0;
	// f_write(fil, (void *)"ABCD", btw, &bw);
	// print("BTW: %d BW: %d\r\n", btw, bw);

	// return 0;

	char test_filename[1024];

	// FIL *fp;
	// fp = fopen("test.txt", "w+");
	// // BYTE mode = get_mode_from_str((char *)"w+");
	// // print("mode: %x\t desired: %x\r\n", mode, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);

	// fflush(stdout);

	u32 j = 0;
	// fwrite(test_filename, 1, 10, fp);
	// print("written\r\n");
	// fclose(fp);

	FIL *ff[5];
	// for (int i = 0; i < 5; i++)
	// 	ff[i] = (FILE *)malloc(sizeof(FILE *) * 5);

	sprintf(test_filename, "test.txt");

	// This function has the right signiature.
	f_open(&ff[0], test_filename, get_mode_from_str((char *)"w+"));
	// ff[0] = *fopen(test_filename, "w+");

	// This is standard function call.
	fwrite(test_filename, 1, 10, ff[0]);

	print("written\t fwrite()\r\n");
	f_write(&ff[0], test_filename, 10, &j);
	print("written\t f_write()\r\n");
	f_close(&ff[0]);
	print("%d\r\n", j);

	// return 0;
	
	double available_space, total_space;
	available_space = (double)dir_get_available("/home/sean");
	total_space = (double)dir_get_space("/home/sean");
	print("Starting disk space: %8.3f/%8.3f\r\n", available_space/MB, total_space/MB);
	G_STATUS gres = G_NOTOK;
	gres = hs_hit_io_unit_test();
	print("hs_unit_write_loop()\t %s (%d)\r\n", gres == G_OK ? "PASSED" : "FAILED", gres);
	print("Ending disk space: %8.3f/%8.3f\r\n", available_space/MB, total_space/MB);
	//return 0;
	// s->read_hit();

	// print("Hit: %d\r\n", sizeof(Hit));
	// print("SPEHit: %d\r\n", sizeof(SPEHit));
	// print("MPEHit: %d\r\n", sizeof(MPEHit));
	// mpep->hit->print_samples(100);



	print("Done.\r\n");
}
