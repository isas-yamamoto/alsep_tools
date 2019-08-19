/*! @file pse2csv2.c
 *  @brief Convert from PSE original binary to CSV
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
#include "pse.h"
#include "error.h"
#include "util.h"
#include "csv.h"

void usage(const char* cmd) {
  fprintf(stderr, "usage: %s psefile\n", cmd);
}

void csv_output(const char* filename, pse_record pr, pse_frame pf) {
  int i;
  uint64_t msec_of_year;
  double dmsec = 64 * 10 / 1060.0 * 1000;

  if (pr.format == FORMAT_OLD) {
    for(i=0; i<32; ++i) {
      msec_of_year = pf.msec_of_year + dmsec * i / 32;
      print_format(filename, pr.year, msec_of_year, pr.apollo_station, "spz",
        pf.frame_count, pf.spz[i]);
    }
  }

  for(i=0; i<4; ++i) {
    msec_of_year = pf.msec_of_year + dmsec * i / 4;
    print_format(filename, pr.year, msec_of_year, pr.apollo_station, "lpx",
      pf.frame_count, pf.lpx[i]);
    print_format(filename, pr.year, msec_of_year, pr.apollo_station, "lpy",
      pf.frame_count, pf.lpy[i]);
    print_format(filename, pr.year, msec_of_year, pr.apollo_station, "lpz",
      pf.frame_count, pf.lpz[i]);
  }

  if (pf.frame_count%2 == 0) {
    print_format(filename, pr.year, pf.msec_of_year, pr.apollo_station, "tdx",
      pf.frame_count, pf.TidX);

    print_format(filename, pr.year, pf.msec_of_year, pr.apollo_station, "tdy",
      pf.frame_count, pf.TidY);
  } else {
    print_format(filename, pr.year, pf.msec_of_year, pr.apollo_station, "tdz",
      pf.frame_count, pf.TidZ);

    print_format(filename, pr.year, pf.msec_of_year, pr.apollo_station, "ist",
      pf.frame_count, pf.InstT);
  }
}

int main(int argc, char** argv) {

  //Generic variables
  FILE *f;
  size_t r;
  char filename[PATH_MAX+1];
  uint32_t process_flag;
  uint32_t error_flag;
  int size_part;
  int i;
  long frame_offset;
  uint32_t doy, hh, mm, ss, ms;
  char *basec, *bname;

  //initial value of Frame time error at last frame in one record
  uint64_t msec_of_year_fmax = 0;

  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_RECORD];
  pse_record pr;
  pse_frame pf[MAX_PSE_FRAME+1];

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

  basec = strdup(filename);
  bname = basename(basec);

  // ----------------------------------------
  // Frame
  // ----------------------------------------
  process_flag = FLAG_FIRST_DATA_OF_FILE;
  while ((r=fread(record, sizeof(unsigned char), SIZE_RECORD, f))>0) {
    if (r != SIZE_RECORD) {
      log_printf(LOG_ERROR, __FILE__, __LINE__, "invalid data size: %zd", r);
      break;
    }

    pr = binary2pse_record(record);
    error_flag = check_pse_record(pr);

    if (ERROR_INVALID_FORMAT && error_flag) {
      log_printf(LOG_ERROR, __FILE__, __LINE__, "invalid format");
      break;
    }

    if (pr.format == FORMAT_OLD) {
      size_part = SIZE_DATA_PART_OLD;
    } else {
      size_part = SIZE_DATA_PART_NEW;
    }

    // register first frame into database
    frame_offset = SIZE_PSE_HEADER;
    pf[0] = binary2pse_frame(pr,&record[frame_offset]);
    pf[0].spz[0] = pf[0].spz[1];
    pf[0].time_diff = pf[0].msec_of_year - msec_of_year_fmax;
    pf[0].prev_frame = -1;
    pf[0].process_flag = process_flag | FLAG_TOP_OF_RECORD | FLAG_FIRST_DATA_COPIED;
    pf[0].error_flag = check_pse_frame(pf[0], pr.apollo_station, pr.year);

    csv_output(bname, pr, pf[0]);

    // register remnant frames into database
    for(i = 1; i < (SIZE_LOGICAL_RECORD * pr.phys_records); i++) {
      frame_offset = SIZE_PSE_HEADER+size_part*i;
      unsigned char* frame = &record[frame_offset];

      pf[i] = binary2pse_frame(pr,frame);
      pf[i].time_diff = pf[i].msec_of_year - pf[i-1].msec_of_year;
      pf[i].prev_frame = pf[i-1].frame_count;
      pf[i].process_flag = 0;
      pf[i].error_flag = check_pse_frame(pf[i], pr.apollo_station, pr.year);

      //! ALSEP WORD 2
      pf[i].spz[0] = interp(
        pf[i-1].spz[30],
        pf[i-1].spz[31],
        pf[i  ].spz[ 1],
        pf[i  ].spz[ 2]);

      csv_output(bname, pr, pf[i]);
    }
    msec_of_year_fmax = pf[i-1].msec_of_year;
    process_flag = 0;
  }
  fclose(f);
  free(bname);
  return EXIT_SUCCESS;
}
