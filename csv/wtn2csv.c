/*! @file wtninfo.c
 *  @brief Convert from WTN original binary to CSV
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2019/07/17
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <libgen.h>

#include "define.h"
#include "wtn.h"
#include "error.h"
#include "util.h"
#include "csv.h"

void usage(const char* cmd) {
  fprintf(stderr, "usage: %s wtnfile\n", cmd);
}

void csv_output(const char *filename, wtn_record wnr, wtn_frame wnf) {
  int i;
  uint32_t doy, hh, mm, ss, ms;
  uint64_t msec_of_year;
  double dmsec = 64 * 10 / 1060.0 * 1000;
  int apollo_station[] = {-1, 12, 15, 16, 14, 17};

  if (wnf.alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17) {
    for(i=0; i<32; ++i) {
      msec_of_year = wnf.msec_of_year + dmsec * i / 32;
      print_format(filename, wnr.year, msec_of_year, 
        apollo_station[wnf.alsep_package_id], "spz",
        wnf.frame_count, wnf.spz[i], wnf.process_flag, wnr.error_flag, wnf.error_flag);
    }

    for(i=0; i<4; ++i) {
      msec_of_year = wnf.msec_of_year + dmsec * i / 4;
      print_format(filename, wnr.year, msec_of_year,
        apollo_station[wnf.alsep_package_id], "lpx",
        wnf.frame_count, wnf.lpx[i], wnf.process_flag, wnr.error_flag, wnf.error_flag);
      print_format(filename, wnr.year, msec_of_year,
        apollo_station[wnf.alsep_package_id], "lpy",
        wnf.frame_count, wnf.lpy[i], wnf.process_flag, wnr.error_flag, wnf.error_flag);
      print_format(filename, wnr.year, msec_of_year,
        apollo_station[wnf.alsep_package_id], "lpz",
        wnf.frame_count, wnf.lpz[i], wnf.process_flag, wnr.error_flag, wnf.error_flag);
    }

    if (wnf.frame_count%2 == 0) {
      print_format(filename, wnr.year, wnf.msec_of_year,
        apollo_station[wnf.alsep_package_id], "tdx",
        wnf.frame_count, wnf.TidX, wnf.process_flag, wnr.error_flag, wnf.error_flag);
      print_format(filename, wnr.year, wnf.msec_of_year,
        apollo_station[wnf.alsep_package_id], "tdy",
        wnf.frame_count, wnf.TidY, wnf.process_flag, wnr.error_flag, wnf.error_flag);
    } else {
      print_format(filename, wnr.year, wnf.msec_of_year,
        apollo_station[wnf.alsep_package_id], "tdz",
        wnf.frame_count, wnf.TidZ, wnf.process_flag, wnr.error_flag, wnf.error_flag);
      print_format(filename, wnr.year, wnf.msec_of_year,
        apollo_station[wnf.alsep_package_id], "ist",
        wnf.frame_count, wnf.InstT, wnf.process_flag, wnr.error_flag, wnf.error_flag);
    }
  } else {
    for(i=0; i<32; ++i) {
      msec_of_year = wnf.msec_of_year + dmsec * i / 32;
      print_format(filename, wnr.year, msec_of_year,
        apollo_station[wnf.alsep_package_id], "lsg",
        wnf.frame_count, wnf.lsg[i], wnf.process_flag, wnr.error_flag, wnf.error_flag);
    }

    print_format(filename, wnr.year, wnf.msec_of_year,
      apollo_station[wnf.alsep_package_id], "lsg_tide",
      wnf.frame_count, wnf.lsg_tide, wnf.process_flag, wnr.error_flag, wnf.error_flag);
    print_format(filename, wnr.year, wnf.msec_of_year,
      apollo_station[wnf.alsep_package_id], "lsg_free",
      wnf.frame_count, wnf.lsg_free, wnf.process_flag, wnr.error_flag, wnf.error_flag);
    print_format(filename, wnr.year, wnf.msec_of_year,
      apollo_station[wnf.alsep_package_id], "lsg_temp",
      wnf.frame_count, wnf.lsg_temp, wnf.process_flag, wnr.error_flag, wnf.error_flag);
  }
}

int main(int argc, char** argv) {

  //Generic variables
  FILE *f;
  size_t r;
  char filename[PATH_MAX+1];
  uint32_t process_flag;
  uint32_t error_flag;
  int i;
  char *basec, *bname;

  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_HEADER];
  unsigned char header[SIZE_HEADER];
  unsigned char frame[SIZE_FRAME];
  wtn_record wnr;
  wtn_frame* wnf = NULL;
  int fsize;
  int max_wtn_frame;
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
    return -1;
  }

  // get filesize
  if ((fsize = filesize(filename))<0) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "invalid filesize: %s", filename);
    goto main_finish;
  }

  basec = strdup(filename);
  bname = basename(basec);

  // ----------------------------------------
  // Frame registration
  // ----------------------------------------
  while ((r=fread(record, sizeof(unsigned char), SIZE_HEADER, f))>0) {
    if (r != SIZE_HEADER) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }

    wnr = binary2wtn_record(record);
    error_flag = check_wtn_record(wnr);

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
        fseek(f, -SIZE_HEADER, SEEK_CUR);
        num_header = 1;
        goto main_finish;
      }
    }

    // Allocate memory
    max_wtn_frame = (fsize-SIZE_HEADER*num_header)/SIZE_FRAME;
    if ((fsize-SIZE_HEADER*num_header)%SIZE_FRAME!=0) {
      max_wtn_frame++;
    }
    wnf = (wtn_frame*)malloc(max_wtn_frame * sizeof(wtn_frame));
    if (wnf == NULL) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "cannot allocate memory\n");
      goto main_finish;
    }

    // Read Frame
    fmax = 0;
    while ((r=fread(frame, sizeof(unsigned char), SIZE_FRAME, f))>0) {
      if ( fmax >= max_wtn_frame ) {
        log_printf(LOG_ERROR, __FILE__, __LINE__,
                   "max_wtn_frame is %d\n", max_wtn_frame);
        goto main_finish;
      }
      wnf[fmax] = binary2wtn_frame(wnr, frame);
      fmax++;
    }

    // first frame
    for (i=0; i<wnr.num_asta; i++) {
      if (wnf[i].alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17) {
        wnf[i].spz[0] = wnf[i].spz[1];
      }
      wnf[i].time_diff = wnf[i].msec_of_year;
      wnf[i].process_flag = process_flag | FLAG_TOP_OF_RECORD | FLAG_FIRST_DATA_COPIED;
      wnf[i].prev_frame = -1;

      wnf[i].error_flag = check_wtn_frame(wnf[i], wnr.year);
      if (wnf[i].error_flag) {
        log_printf(LOG_WARNING, __FILE__, __LINE__,
                   "frame error: error code=0x%04x  file offset=%d msec_of_year=%" PRId64,
                   wnf[i].error_flag,
                   SIZE_HEADER*num_header+SIZE_FRAME*i,
                   wnf[i].msec_of_year);
      }

      csv_output(bname, wnr, wnf[i]);
    }

    for(i = wnr.num_asta; i<fmax; i++) {

      if (wnf[i].alsep_package_id == wnf[i-wnr.num_asta].alsep_package_id) {
        wnf[i].time_diff = wnf[i].msec_of_year - wnf[i-wnr.num_asta].msec_of_year;
        wnf[i].prev_frame = wnf[i-wnr.num_asta].frame_count;
      } else {
        wnf[i].time_diff = wnf[i].msec_of_year;
        wnf[i].process_flag = FLAG_FIRST_DATA_COPIED;
        wnf[i].prev_frame = -1;
      }

      wnf[i].error_flag = check_wtn_frame(wnf[i], wnr.year);
      if (wnf[i].error_flag) {
        log_printf(LOG_WARNING, __FILE__, __LINE__,
                   "frame error: error code=0x%04x  file offset=%d msec_of_year=%" PRId64,
                   wnf[i].error_flag,
                   SIZE_HEADER*num_header+SIZE_FRAME*i,
                   wnf[i].msec_of_year);
      }

      if (wnf[i].alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17) {
        if (wnf[i].process_flag & FLAG_FIRST_DATA_COPIED) {
          wnf[i].spz[0] = wnf[i].spz[1];
        } else {
          //! ALSEP WORD 2
          wnf[i].spz[0] = interp(wnf[i-wnr.num_asta].spz[30],
                                  wnf[i-wnr.num_asta].spz[31],
                                  wnf[i].spz[1],
                                  wnf[i].spz[2]);
        }
      }
      csv_output(bname, wnr, wnf[i]);
    }
  }

main_finish:
  if (f) {
    fclose(f);
  }

  if (wnf) {
    free(wnf);
    wnf = NULL;
  }

  if (bname) {
    free(bname);
    bname = NULL;
  }
  return 0;
}
