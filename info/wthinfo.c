/*! @file wthinfo.c
 *  @brief WTHデータの情報を表示するプログラム
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2011/06/15
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

static void usage(const char* cmd) {
  fprintf(stderr, "%s [-rfdi] filename\n", cmd);
}

void display_frame(wth_frame whf) {
  uint32_t doy, hh, mm, ss, ms;
  
  msec_of_year_to_date(whf.msec_of_year, &doy, &hh, &mm, &ss, &ms);
  printf("doy=%d,time=%02d:%02d:%02d.%03d", doy, hh, mm, ss, ms);
  printf(",flag=%d",whf.flag_bit);
  printf(",msec_of_year=%"PRId64,whf.msec_of_year);
  printf(",track=%d",whf.alsep_tracking_station_id);
  printf(",package=%d",whf.alsep_package_id);
  printf(",orig=%d",whf.original_rec_num);
  printf(",sync=%d", whf.sync_code);
  printf(",sub_frame=%d", whf.sub_frame);
  putchar('\n');
}

void display_data(wth_frame whf) {
  int i;
  printf("#dp1: %d", whf.dp1[0]);
  for(i=1; i<20; ++i) {
    printf(",%d",whf.dp1[i]);
  }
  putchar('\n');

  printf("#dp6: %d", whf.dp6[0]);
  for(i=1; i<20; ++i) {
    printf(",%d",whf.dp6[i]);
  }
  putchar('\n');

  printf("#dp11: %d", whf.dp11[0]);
  for(i=1; i<20; ++i) {
    printf(",%d",whf.dp11[i]);
  }
  putchar('\n');

  printf("#dp16: %d", whf.dp16[0]);
  for(i=1; i<20; ++i) {
    printf(",%d",whf.dp16[i]);
  }
  putchar('\n');

  printf("#status: %d", whf.status[0]);
  for(i=1; i<20; ++i) {
    printf(",%d",whf.status[i]);
  }
  putchar('\n');
}
  
int main(int argc, char** argv) {
  
  // Generic variables
  FILE *f = NULL;
  size_t r;
  char filename[PATH_MAX+1];
  int error_flag;
  int i;
  
  // getopt
  int ch;
  extern char *optarg;
  extern int optind, opterr;

  // for print
  uint32_t doy, hh, mm, ss, ms;
  uint32_t doy_last=-1,hh_last=-1,mm_last=-1,ss_last=-1,ms_last=-1;

  // verboseフラグ
  int verbose_record = 0;
  int verbose_frame = 0;
  int verbose_data = 0;
  int ignore_duplicated = 0;
  int select_package = -1;

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

  // 引数の確認
  while ((ch=getopt(argc, argv, "rfdi"))!=-1) {
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
    case 'i':
      ignore_duplicated = 1;
      break;
    default:
      usage(argv[0]);
      break;
    }
  }
  argc -= optind;
  if (argc != 1) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  argv += optind;

  SET_ARG(filename,0,PATH_MAX);

  // PROGRAM MAIN
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

    if (verbose_record) {
      printf("--- record info ---\n");
      printf("id: %d\n", whr.id);
      printf("active_stations: %d%d%d%d%d\n",
	     whr.active_station[0],
	     whr.active_station[1],
	     whr.active_station[2],
	     whr.active_station[3],
	     whr.active_station[4]);
      printf("num_asta: %d\n", whr.num_asta);
      printf("original 9 track ID: %d\n", whr.original_id);
      printf("year: %d\n", whr.year);
      printf("first msec: %"PRId64"\n", whr.first_msec);
      putchar('\n');
    }

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
    for(i=0;i<SIZE_HEADER;++i) {
      if (record[i] != header[i]) {
	log_printf(LOG_ERROR, __FILE__, __LINE__,
		   "header is not duplicated.");
	if (!ignore_duplicated) {
	  goto main_finish;
	}
	fseek(f, -SIZE_HEADER, SEEK_CUR);
	num_header = 1;
	break;
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
      
      if (verbose_frame) {
	display_frame(whf[fmax]);
      }
	
      if (verbose_data) {
	display_data(whf[fmax]);
      }
      
      if (whf[fmax].original_rec_num != 0) {
	msec_of_year_to_date(whf[fmax].msec_of_year,
			     &doy_last,
			     &hh_last,
			     &mm_last,
			     &ss_last,
			     &ms_last);
      }
      
      fmax++;

    }
  }

  printf("%s,%d,%d%d%d%d%d,%d,",
	 filename,
	 whr.original_id,
	 whr.active_station[0],
	 whr.active_station[1],
	 whr.active_station[2],
	 whr.active_station[3],
	 whr.active_station[4],
	 whr.year);
  
  msec_of_year_to_date(whf[0].msec_of_year, &doy, &hh, &mm, &ss, &ms);
  printf("%d,%02d:%02d:%02d.%03d", doy, hh, mm, ss, ms);
  
  printf(",%d,%02d:%02d:%02d.%03d",
	 doy_last, hh_last, mm_last, ss_last, ms_last);
  printf(",%d\n", fmax/60);
  
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
