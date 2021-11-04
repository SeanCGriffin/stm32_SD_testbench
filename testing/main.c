#include <inttypes.h>
#include <stdarg.h> //for va_list
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ff_proxy.h"
#include "hs_readback.h"
#include "hs_types.h"
#include "local_main.h"
#include "hs_streamer.h"

SystemTime s = {0};
uint32_t sys_seconds = 0xFF000000;
uint16_t sys_subseconds = 0;
uint16_t dummy_time = 0xF0F0;

//Writer file handlers
extern bool handler_active[NUM_PMT];
extern FIL handlers[NUM_PMT];
extern char live_filenames[NUM_PMT][256];
extern bool daq_enabled;

//Writer data externs
extern PayloadType_t current_hit_type;
extern SPEPacket *spep;
extern MPEPacket *mpep;

//Reader externs
extern PayloadType_t mrr_hit_type;
extern SPEHit *mrr_speh; //Most recent read SPE hit.
extern MPEHit *mrr_mpeh; //Most recent read MPE hit.

void print(const char *fmt, ...) {
  static char buffer[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  printf((char *)buffer);
}

SystemTime *get_system_time() {

  s.coarse = sys_seconds;
  s.fine = dummy_time++;
  return &s;
}

int main(void) {

  u16 nsamples = 300;
  u8 sample_buf[nsamples];
  for (int i = 0; i < nsamples; i++)
    sample_buf[i] = 65 + (i % (40));

  // FIL *fil = NULL;
  // FRESULT fres;
  // fres = f_open(fil, "hitspool/test.spool", 1);
  // print("fres = %d\r\n", fres);

  // u32 bw = 0;
  // u32 btw = 4;
  // f_write(fil, "ABCD", btw, &bw);
  // print("BTW: %d BW: %d\r\n", btw, bw);

  // return 0;

  char test_filename[1024];
  sprintf(test_filename, "hitspool/test.txt");
  FIL fp;
  // fp = *fopen("test.txt", "w+");
  BYTE mode = get_mode_from_str("w+");
  print("mode: %x\t desired: %x\r\n", mode,
        FA_CREATE_ALWAYS | FA_WRITE | FA_READ);

  // fflush(stdout);
  f_open(&fp, test_filename, get_mode_from_str("w+"));

  print("\r\n---- Streamer Testbench ----\r\n");
  print("-----------------------------------\r\n\n");

  print("\r\n\n");
  print("Initializing hit buffers, write heads, file handlers.\r\n\n");
  init_write_heads();
  print("Write heads done.\r\n");
  init_file_handlers();
  print("File handlers done.\r\n");
  init_hit_buffers();
  print("Hit buffers done.\r\n");

  // //print_IO_handlers();
  // print("SPE size: %d\r\n", sizeof(SPEHit));
  // print("MPE size: %d\r\n", sizeof(MPEHit));
  // // WUBPacket *test_WUB = generate_dummy_WUBPacket(2, nsamples/2, sample_buf);
  // // print("wubp.hit size: %d\r\n", calc_WUB_hits_size(&test_WUB->hits));
  // // print_WUBPacket(test_WUB);

  // // //f_open(&fp, "test.txt", "w+");
  // // //printf("fp == NULL? %d\r\n", &fp == NULL ? 1 : 0);
  // // // fprintf(fp, "This is testing for fprintf...\n");
  // // // fputs("This is testing for fputs...\n", fp);
  // // //size_t fwrite(const void *ptr, size_t size_of_elements, size_t
  // // number_of_elements, FILE *a_file);
  // // //fwrite(sample_buf, 1, 300, fp);
  // // // print("mpep.hit size:  %d\r\n", calc_MPE_hit_size(&mpep->hit));
  // // // fwrite(&mpep->hit, calc_MPE_hit_size(&mpep->hit), 1, fp);

  // SPEPacket *test_SPE;
  // for (int i = 0; i < 10; i++) {
  //   test_SPE = generate_dummy_SPEPacket(0, 0x400 );
  //   fwrite(&test_SPE->hit, calc_SPE_hit_size(&test_SPE->hit), 1, &fp);
  //   //print_SPEHit(&test_SPE->hit);
  // }

  // // print("spep.hit size: %d\r\n", calc_SPE_hit_size(&test_SPE->hit));

  // MPEPacket *test_MPE = generate_dummy_MPEPacket(3, nsamples, sample_buf);
  // test_MPE->hit.launch_t = 0xABBA;
  // test_MPE->hit.tdc = 60;
  // test_MPE->hit.padding = 31;

  // //print_MPEHit(&test_MPE->hit);
  // print("mpep.hit size: 0x%8X\r\n", calc_MPE_hit_size(&test_MPE->hit));

  // u32 n = 0;
  // fwrite(&test_MPE->hit, calc_MPE_hit_size(&test_MPE->hit), 1, &fp);
  // fwrite(&test_SPE->hit, calc_SPE_hit_size(&test_SPE->hit), 1, &fp);
  // print("%d \t Closing.\r\n", n);
  // f_close(&fp);

  // print("\r\nOpening '%s' for readback test:\r\n", test_filename);
  // f_open(&fp, test_filename, get_mode_from_str("r"));

  // read_next_hit(&fp);
  // print("\r\n\n");
  // read_next_hit(&fp);

  // f_close(&fp);
  // return 0;
  u8 nloops = 2;
  u8 PMT = 2;


  UNIT_write_loop(nloops, PMT, 100);
  UNIT_read_loop(nloops, PMT);

  print("Done.\r\n");
}