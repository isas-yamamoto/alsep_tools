/*! @file pseinfo.c
 *  @brief PSEデータをダンプするプログラム
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2011/02/01
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "define.h"
#include "pse.h"
#include "error.h"
#include "util.h"

void usage(const char* cmd) {
  fprintf(stderr, "%s [-rfd] filename\n", cmd);
}

void display_frame(pse_frame pf) {
  uint32_t doy, hh, mm, ss, ms;
  msec_of_year_to_date(pf.msec_of_year, &doy, &hh, &mm, &ss, &ms);
  printf("frame info:");
  printf(" frame_count=%d", pf.frame_count);
  printf(",doy=%d,time=%02d:%02d:%02d.%03d", doy, hh, mm, ss, ms);
  printf(",msec=%"PRId64, pf.msec_of_year);
  printf(",diff=%"PRId64, pf.time_diff);
  printf(",track_id=%d", pf.alsep_tracking_station_id);
  printf(",sync_code=%d", pf.sync_code);
  printf(",time_flag=%d", pf.software_time_flag);
  printf(",process=0x%02x", pf.process_flag);
  printf(",error=0x%02x", pf.error_flag);
  putchar('\n');
}

void display_data(pse_record pr, pse_frame pf) {
  int i;

  if (pr.format == FORMAT_OLD) {
    printf("#spz: %d", pf.spz[0]);
    for(i=1; i<32; ++i) {
      printf(",%d",pf.spz[i]);
    }
    putchar('\n');
  }
  
  printf("#lpx: %d", pf.lpx[0]);
  for(i=1; i<4; ++i) {
    printf(",%d",pf.lpx[i]);
  }
  putchar('\n');
  
  printf("#lpy: %d", pf.lpy[0]);
  for(i=1; i<4; ++i) {
    printf(",%d",pf.lpy[i]);
  }
  putchar('\n');
  
  printf("#lpz: %d", pf.lpz[0]);
  for(i=1; i<4; ++i) {
    printf(",%d",pf.lpz[i]);
  }
  putchar('\n');
  
  if (pf.frame_count%2 == 0) {
    printf("#tidal_x: %d\n", pf.TidX);
    printf("#tidal_y: %d\n", pf.TidY);
  } else {
    printf("#tidal_z: %d\n", pf.TidZ);
    printf("#inst_temp: %d\n", pf.InstT);
  }
  putchar('\n');
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
  
  //getopt
  int ch;
  extern char *optarg;
  extern int optind, opterr;
  
  //flags
  int verbose_frame = 0;
  int verbose_record = 0;
  int verbose_data = 0;

  long rec_offset, frame_offset;

  //initial value of Frame time error at last frame in one record
  uint64_t msec_of_year_fmax = 0;
  
  //program util
  int record_index = 0;
  
  // print datetime
  uint32_t doy, hh, mm, ss, ms;
  uint32_t doy_first=-1,hh_first=-1,mm_first=-1,ss_first=-1,ms_first=-1;
  uint32_t doy_last=-1,hh_last=-1,mm_last=-1,ss_last=-1,ms_last=-1;
  
  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_RECORD];
  pse_record pr;
  pse_frame pf[MAX_PSE_FRAME+1];
  
  while ((ch = getopt(argc, argv, "rfd")) != -1) {
    switch(ch) {
    case 'r':
      verbose_record = 1;
      break;
    case 'f':
      verbose_frame = 1;
      break;
    case 'd':
      verbose_data = 1;
      break;
    default:
      usage(argv[0]);
      break;
    }
  }
  argc -= optind;
  if (argc != 1){
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  argv += optind;

  // PROGRAM MAIN  
  SET_ARG(filename,0,PATH_MAX);
  
  f=fopen(filename, "rb");
  if(f==NULL) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
	       "no such file: %s", filename);
    return -1;
  }
  
  // ----------------------------------------
  // Frame
  // ----------------------------------------
  process_flag = FLAG_FIRST_DATA_OF_FILE;
  rec_offset = ftell(f);
  while ((r=fread(record, sizeof(unsigned char), SIZE_RECORD, f))>0) {
    if (r != SIZE_RECORD) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }
    
    pr = binary2pse_record(record);
    error_flag = check_pse_record(pr);
    
    if (verbose_record) {
      printf("--- record info ---\n");
      printf("offset in file: %ld\n", rec_offset);
      printf("tape type: %d\n", pr.tape_type);
      printf("apollo station: %d\n", pr.apollo_station);
      printf("tape seq: %d\n", pr.tape_seq);
      printf("record number: %d\n", pr.record_number);
      printf("year: %d\n", pr.year);
      printf("format: %d\n", pr.format);
      printf("phys records: %d\n", pr.phys_records);
      printf("read error: %d\n", pr.read_err);
      printf("error flag: %d\n", error_flag);
      putchar('\n');
    }
    
    if (ERROR_INVALID_FORMAT && error_flag) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
		 "invalid format");
      goto main_finish;
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
    
    msec_of_year_to_date(pf[0].msec_of_year, &doy, &hh, &mm, &ss, &ms);
    if (record_index == 0) {
      doy_first = doy;
      hh_first = hh;
      mm_first = mm;
      ss_first = ss;
      ms_first = ms;
    }
    
    if (verbose_frame) {
      display_frame(pf[0]);
    }
    
    if (verbose_data) {
      display_data(pr,pf[0]);
    }
    
    // register remnant frames into database
    for(i = 1; i < (SIZE_LOGICAL_RECORD * pr.phys_records);i++) {
      frame_offset = SIZE_PSE_HEADER+size_part*i;
      unsigned char* frame = &record[frame_offset];
      
      pf[i] = binary2pse_frame(pr,frame);
      pf[i].time_diff = pf[i].msec_of_year - pf[i-1].msec_of_year;
      pf[i].prev_frame = pf[i-1].frame_count;
      pf[i].process_flag = 0;
      pf[i].error_flag = check_pse_frame(pf[i], pr.apollo_station, pr.year);
      
      msec_of_year_to_date(pf[i].msec_of_year, &doy, &hh, &mm, &ss, &ms);
      if (doy > 0) {
	doy_last = doy;
	hh_last = hh;
	mm_last = mm;
	ss_last = ss;
	ms_last = ms;
      }
      
      if (verbose_frame) {
	display_frame(pf[i]);
      }
      
      if (verbose_data) {
	//! ALSEP WORD 2
	pf[i].spz[0] = interp(
			      pf[i-1].spz[30],
			      pf[i-1].spz[31],
			      pf[i  ].spz[ 1],
			      pf[i  ].spz[ 2]);      
	display_data(pr,pf[i]);
      }
    }
    record_index++;
    msec_of_year_fmax = pf[i-1].msec_of_year;
    rec_offset = ftell(f);
    process_flag = 0;
  }
  
  printf("%s,%d,%d,%d,%d,",
	 filename,
	 pr.apollo_station,
	 pr.tape_seq,
	 pr.year,
	 pr.format);
  
  printf("%d,%02d:%02d:%02d.%03d",
	 doy_first, hh_first, mm_first, ss_first, ms_first);
  printf(",%d,%02d:%02d:%02d.%03d,%d\n",
	 doy_last, hh_last, mm_last, ss_last, ms_last, record_index);
  
main_finish:
  if (f) {
    fclose(f);
  }
  
  return 0;
}
