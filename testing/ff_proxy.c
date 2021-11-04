
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <assert.h>
#include <libgen.h>

#include "local_main.h"
#include "ff_proxy.h"

extern void print(const char *fmt, ...); 

extern char* strdupa(const char*);

uint64_t proxy_drive_size = 32000000000; //32 GB in B
uint64_t drive_space_consumed = 0;
uint64_t drive_sapce_remaining = 0;

uint32_t get_total_space_KiB(char* volume){
  return proxy_drive_size / 1024;
}
uint32_t get_free_space_KiB(char* volume){
  return (proxy_drive_size - drive_space_consumed) / 1024;
}

//https://stackoverflow.com/a/9210960
int mkpath(char* file_path, mode_t mode) {
    print("mkpath(%s)\r\n", file_path);
    assert(file_path && *file_path);

    for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';
        if (mkdir(file_path, mode) == -1) {
            if (errno != EEXIST) {
                *p = '/';
                return -1;
            }
        }
        *p = '/';
    }
    return 0;
}

FRESULT f_open(FIL *fp, char *path, BYTE mode) {
  if(mode && FA_READ == 0)
    mkpath(path, 0755);

  char strmode[10];
  get_mode_from_byte(strmode, mode);
  print("Opening '%s' in mode '%s'.\r\n", path, strmode);
  fflush(stdout);
  *fp = *fopen(path, strmode);

  if(fp == NULL){
    print("ERROR OPENING FILE: %s\r\n", strerror(errno));
    return FR_NOT_READY;    
  } else{
    print("File opened ok\r\n");
    return FR_OK;
  }

}

FRESULT f_write(FIL *fp, void *buff, UINT btw, UINT *bw) {

  fflush(stdout);
  if (fp == NULL) {
    return FR_NOT_READY;
  }

  *bw = fwrite(buff, 1, btw, fp);
  if (*bw == btw)
    return FR_OK;
  else
    return FR_NOT_READY;
}

FRESULT f_read (
  FIL* fp,  /* Pointer to the file object */
  void* buff, /* Pointer to data buffer */
  UINT btr, /* Number of bytes to read */
  UINT* br  /* Pointer to number of bytes read */
)
{
  //size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
  FRESULT res;
  *br = fread(buff, 1, btr, fp);

  if(btr == br)
    return FR_OK;
  else
    return FR_DISK_ERR;

}

FRESULT f_close (FIL* fp){

  int ret = fclose(fp);

  if(ret == 0)
    return FR_OK;
  else
    return FR_NOT_READY;
}



BYTE get_mode_from_str(char* cmode){

    if(strcmp("r", cmode) == 0)
      return FA_READ;
    else if(strcmp("r+", cmode) == 0)
      return FA_READ | FA_WRITE;
    else if(strcmp("w", cmode) == 0)
      return FA_CREATE_ALWAYS | FA_WRITE;
    else if(strcmp("w+", cmode) == 0)
      return FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
    else if(strcmp("a", cmode) == 0)
      return FA_OPEN_APPEND | FA_WRITE;
    else if(strcmp("a+", cmode) == 0)
      return FA_OPEN_APPEND | FA_WRITE | FA_READ;
    else if(strcmp("wx", cmode) == 0)
      return FA_CREATE_NEW | FA_WRITE;
    else if(strcmp("w+x", cmode) == 0)
      return FA_CREATE_NEW | FA_WRITE | FA_READ;
    else
      return 0;
  

}               

void get_mode_from_byte(char* cmode, BYTE mode){

  switch(mode){
    case FA_READ:
      sprintf(cmode, "r");
      break;
    case (FA_READ | FA_WRITE):  
      sprintf(cmode, "r+");
      break;
    case (FA_CREATE_ALWAYS | FA_WRITE):
      sprintf(cmode, "w");
      break;
    case (FA_CREATE_ALWAYS | FA_WRITE | FA_READ):
      sprintf(cmode, "w+");
      break;
    case (FA_OPEN_APPEND | FA_WRITE):
      sprintf(cmode, "a");
      break;
    case (FA_OPEN_APPEND | FA_WRITE | FA_READ):
      sprintf(cmode, "a+");
      break;
    case (FA_CREATE_NEW | FA_WRITE):
      sprintf(cmode, "wx");
      break;
    case (FA_CREATE_NEW | FA_WRITE | FA_READ):
      sprintf(cmode, "w+x");
      break;
    default:
      sprintf(cmode, "");

  }

  return;
}

