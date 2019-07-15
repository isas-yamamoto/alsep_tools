/*! @file wtninfo.c
 *  @brief WTNデータの情報を表示するプログラム
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2011/02/02
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
#include "wtn.h"
#include "error.h"
#include "util.h"

void usage(const char* cmd) {
  fprintf(stderr, "%s [-rfdi] [-p package_id] filename\n", cmd);
}

void display_frame(wtn_frame wnf) {
  uint32_t doy, hh, mm, ss, ms;
  int i;
  
  msec_of_year_to_date(wnf.msec_of_year, &doy, &hh, &mm, &ss, &ms);
  printf("frame info:");
  printf(" frame_count=%d", wnf.frame_count);
  printf(",doy=%d,time=%02d:%02d:%02d.%03d", doy, hh, mm, ss, ms);
  printf(",flag=%d",wnf.flag_bit);
  printf(",msec_of_year=%"PRId64,wnf.msec_of_year);
  printf(",track=%d",wnf.alsep_tracking_station_id);
  printf(",package=%d",wnf.alsep_package_id);
  printf(",orig=%d",wnf.original_rec_num);
  printf(",sync=%d", wnf.sync_code);
  printf(",process=0x%04x", wnf.process_flag);
  printf(",error=0x%04x", wnf.error_flag);
  putchar('\n');
}

void display_data(wtn_frame wnf) {
  int i;
  if (wnf.alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17) {
    printf("#spz: %d" ,wnf.spz[0]);
    for(i=1; i<32; ++i) {
      printf(",%d",wnf.spz[i]);
    }
    putchar('\n');
    
    printf("#lpx: %d", wnf.lpx[0]);
    for(i=1; i<4; ++i) {
      printf(",%d",wnf.lpx[i]);
    }
    putchar('\n');

    printf("#lpy: %d", wnf.lpy[0]);
    for(i=1; i<4; ++i) {
      printf(",%d",wnf.lpy[i]);
    }
    putchar('\n');

    printf("#lpz: %d", wnf.lpz[0]);
    for(i=1; i<4; ++i) {
      printf(",%d",wnf.lpz[i]);
    }
    putchar('\n');
    
    if (wnf.frame_count%2 == 0) {
      printf("#tidal_x: %d\n", wnf.TidX);
      printf("#tidal_y: %d\n", wnf.TidY);
    } else {
      printf("#tidal_z: %d\n", wnf.TidZ);
      printf("#inst_temp: %d\n", wnf.InstT);
    }

    switch(wnf.alsep_package_id) {
    case ALSEP_PACKAGE_ID_APOLLO_12:
    case ALSEP_PACKAGE_ID_APOLLO_15:
    case ALSEP_PACKAGE_ID_APOLLO_16:
      printf("#lsm_eng_stat: %d\n", wnf.lsm_stat);
      printf("#lsm: %d", wnf.lsm[0]);
      for(i=1; i<6; ++i) {
        printf(",%d",wnf.lsm[i]);
      }
      putchar('\n');
      break;
    }
    
  } else {
    printf("#lsg: %d", wnf.lsg[0]);
    for(i=1; i<31; ++i) {
      printf(",%d",wnf.lsg[i]);
    }
    putchar('\n');
    
    printf("#lsg_tide: %d\n", wnf.lsg_tide);
    printf("#lsg_free: %d\n", wnf.lsg_free);
    printf("#lsg_temp: %d\n", wnf.lsg_temp);
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
  int i;

  //getopt
  int ch;
  extern char *optarg;
  extern int optind, opterr;

  //flags
  int verbose_record = 0;
  int verbose_frame = 0;
  int verbose_data = 0;
  int ignore_duplicated = 0;
  int select_package = -1;
  
  // for print
  uint32_t doy, hh, mm, ss, ms;
  uint32_t doy_last=-1,hh_last=-1,mm_last=-1,ss_last=-1,ms_last=-1;
  
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

  while ((ch = getopt(argc, argv, "rfdip:")) != -1) {
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
    case 'p':
      select_package = atoi(optarg);
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
    
    wnr = binary2wtn_record(record);
    error_flag = check_wtn_record(wnr);
    
    if (verbose_record) {
      printf("--- record info ---\n");
      printf("id: %d\n", wnr.id_normal);
      printf("active_stations: %d%d%d%d%d\n",
	     wnr.active_station[0],
	     wnr.active_station[1],
	     wnr.active_station[2],
	     wnr.active_station[3],
	     wnr.active_station[4]);
      printf("num_asta: %d\n", wnr.num_asta);
      printf("original 9 track ID: %d\n", wnr.original_id);
      printf("year: %d\n", wnr.year);
      printf("first msec: %"PRId64"\n", wnr.first_msec);
      printf("error flag: %d\n", error_flag);
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
      wnf[fmax] = binary2wtn_frame(wnr,frame);
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
		   "frame error: error code=0x%04x  file offset=%d msec_of_year=%"PRId64,
                   wnf[i].error_flag,
		   SIZE_HEADER*num_header+SIZE_FRAME*i,
                   wnf[i].msec_of_year);
      }
      
      if (select_package == -1 ||
	  select_package == wnf[i].alsep_package_id) {
	
	if (verbose_frame) {
	  display_frame(wnf[i]);
	}
	
	if (verbose_data) {
	  display_data(wnf[i]);
	}
      }
    }
    
    for(i = wnr.num_asta; i<fmax ;i++) {
      
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
		   "frame error: error code=0x%04x  file offset=%d msec_of_year=%"PRId64,
                   wnf[i].error_flag,
		   SIZE_HEADER*num_header+SIZE_FRAME*i,
                   wnf[i].msec_of_year);
      }
      
      msec_of_year_to_date(wnf[i].msec_of_year, &doy, &hh, &mm, &ss, &ms);
      if (wnf[i].original_rec_num != 0 && doy > 0 &&
	  wnf[i].alsep_package_id != 0) {
	doy_last = doy;
	hh_last = hh;
	mm_last = mm;
	ss_last = ss;
	ms_last = ms;
      }
      
      if (select_package == -1 ||
	  select_package == wnf[i].alsep_package_id) {
	
	if (verbose_frame) {
	  display_frame(wnf[i]);
	}
	
	if (verbose_data) {
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
	  display_data(wnf[i]);
	}
      }
    }
  }
  
  printf("%s,%d,%d%d%d%d%d,%d,",
	 filename,
	 wnr.original_id,
	 wnr.active_station[0],
	 wnr.active_station[1],
	 wnr.active_station[2],
	 wnr.active_station[3],
	 wnr.active_station[4],
	 wnr.year);
  
  msec_of_year_to_date(wnf[0].msec_of_year, &doy, &hh, &mm, &ss, &ms);
  printf("%d,%02d:%02d:%02d.%03d", doy, hh, mm, ss, ms);
  printf(",%d,%02d:%02d:%02d.%03d",
	 doy_last, hh_last, mm_last, ss_last, ms_last);
  printf(",%d\n", fmax/wnr.num_asta/12-1);
  
main_finish:
  if (f) {
    fclose(f);
  }

  if (wnf) {
    free(wnf);
    wnf = NULL;
  }
  return 0;
  }
