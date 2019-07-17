/*! @file wthinfo.c
 *  @brief Convert from WTH original binary to CSV
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2019/07/17
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "define.h"
#include "wth.h"
#include "error.h"
#include "util.h"

void usage(const char* cmd) {
  fprintf(stderr, "usage: %s wthfile\n", cmd);
}

void csv_output(wth_record whr, wth_frame whf) {
  int i;
  uint32_t doy, hh, mm, ss, ms;
  uint64_t msec_of_year;
  double dmsec = 20 * 30 / 3533.0 * 1000;

  for(i=0; i<20; ++i) {
    msec_of_year = whf.msec_of_year + dmsec * i / 20;
    msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
    printf("dp1");
    printf(",%d", whr.year);
    printf(",%d,%02d:%02d:%02d.%06d", doy, hh, mm, ss, ms*1000);
    printf(",%d",whf.dp1[i]);
    putchar('\n');
  }

  for(i=0; i<20; ++i) {
    msec_of_year = whf.msec_of_year + dmsec * i / 20;
    msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
    printf("dp6");
    printf(",%d", whr.year);
    printf(",%d,%02d:%02d:%02d.%06d", doy, hh, mm, ss, ms*1000);
    printf(",%d",whf.dp6[i]);
    putchar('\n');
  }

  for(i=0; i<20; ++i) {
    msec_of_year = whf.msec_of_year + dmsec * i / 20;
    msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
    printf("dp6");
    printf(",%d", whr.year);
    printf(",%d,%02d:%02d:%02d.%06d", doy, hh, mm, ss, ms*1000);
    printf(",%d",whf.dp11[i]);
    putchar('\n');
  }

  for(i=0; i<20; ++i) {
    msec_of_year = whf.msec_of_year + dmsec * i / 20;
    msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
    printf("dp6");
    printf(",%d", whr.year);
    printf(",%d,%02d:%02d:%02d.%06d", doy, hh, mm, ss, ms*1000);
    printf(",%d",whf.dp16[i]);
    putchar('\n');
  }

  for(i=0; i<20; ++i) {
    msec_of_year = whf.msec_of_year + dmsec * i / 20;
    msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
    printf("dp_status");
    printf(",%d", whr.year);
    printf(",%d,%02d:%02d:%02d.%06d", doy, hh, mm, ss, ms*1000);
    printf(",%d",whf.dp16[i]);
    putchar('\n');
  }
}

int main(int argc, char** argv) {

  // Generic variables
  FILE *f = NULL;
  size_t r;
  char filename[PATH_MAX+1];
  int error_flag;
  int i;
  uint32_t doy, hh, mm, ss, ms;

  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_HEADER];
  unsigned char header[SIZE_HEADER];
  unsigned char frame[SIZE_FRAME];
  wth_record whr;
  wth_frame* whf = NULL;
  int fsize;
  int max_wth_frame;
  int fmax = -1;
  int num_header = 2;

  // ----------------------------------------
  // Command line option
  // ----------------------------------------
  if (argc != 2) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  SET_ARG(filename, 1, PATH_MAX);

  // ----------------------------------------
  // PROGRAM MAIN
  // ----------------------------------------
  f=fopen(filename, "rb");
  if(f==NULL) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "no such file: %s", filename);
    return EXIT_FAILURE;
  }

  // get filesize
  if ((fsize = filesize(filename))<0) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "invalid filesize: %s", filename);
    goto main_finish;
  }

  // ----------------------------------------
  // Frame registration
  // ----------------------------------------
  while ((r=fread(record, sizeof(unsigned char), SIZE_HEADER, f))>0) {
    if (r != SIZE_HEADER) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }

    whr = binary2wth_record(record);
    error_flag = check_wth_record(whr);

    if (ERROR_INVALID_FORMAT && error_flag) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid format");
      goto main_finish;
    }

    r = fread(header, sizeof(unsigned char), SIZE_HEADER, f);
    if (r != SIZE_HEADER) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }

    // check duplicated header
    for(i=0; i<SIZE_HEADER; ++i) {
      if (record[i] != header[i]) {
        log_printf(LOG_ERROR, __FILE__, __LINE__,
                   "header is not duplicated.");
        goto main_finish;
      }
    }

    // Allocate memory
    max_wth_frame = (fsize-SIZE_HEADER*num_header)/SIZE_FRAME;
    if ((fsize-SIZE_HEADER*num_header)%SIZE_FRAME!=0) {
      max_wth_frame++;
    }
    whf = (wth_frame*)malloc(max_wth_frame * sizeof(wth_frame));
    if (whf == NULL) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "cannot allocate memory\n");
      goto main_finish;
    }

    // Read Frame
    fmax = 0;
    while ((r=fread(frame, sizeof(unsigned char), SIZE_FRAME, f))>0) {
      if ( fmax >= max_wth_frame ) {
        log_printf(LOG_ERROR, __FILE__, __LINE__,
                   "max_wth_frame is %d\n", max_wth_frame);
        goto main_finish;
      }
      whf[fmax] = binary2wth_frame(whr,frame);
      error_flag = check_wth_frame(whf[fmax], whr.year);
      csv_output(whr, whf[fmax]);
    }
    fmax++;
  }

main_finish:
  if (f) {
    fclose(f);
    f = NULL;
  }

  if (whf) {
    free(whf);
    whf = NULL;
  }

  return EXIT_SUCCESS;
}
