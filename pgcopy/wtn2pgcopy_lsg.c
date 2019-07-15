/*! @file wtn2pgcopy_lsg.c
 *  @brief WTNデータのLSG部をRDBMSに格納するプログラム
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2011/06/19
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#include "define.h"
#include "wtn.h"
#include "error.h"
#include "util.h"

#define SET_ARG(var,n,size) {strncpy(var, argv[n], size); var[size] = '\0';}

void print_pg_copy_init();
void print_pg_copy(int id, int offset, int len, wtn_record wnr, wtn_frame wnf);

void set_independent_data(wtn_frame* wnf);
void set_related_data(wtn_frame* wnf, wtn_frame* before);

int main(int argc, char** argv) {
  
  //Generic variables
  int id;
  FILE *f;
  size_t r;
  char filename[PATH_MAX+1];
  uint32_t process_flag;
  int i;
  
  long rec_offset;
  
  //initial value of Frame time error at last frame in one record
  uint32_t msec_of_year_fmax = 0;
  
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
  int count_header = 2;
  
  // PROGRAM MAIN
  if (argc < 2){
    fprintf(stderr, "%s id filename\n", argv[0]);
    return -1;
  }
  
  id = atoi(argv[1]);
  SET_ARG(filename,2,PATH_MAX);
  
  f=fopen(filename, "rb");
  if(f==NULL) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
	       "no such file: %s", filename);
    return -1;
  }
  log_printf(LOG_INFO, __FILE__, __LINE__, "processing: %s", filename);
  
  // get filesize
  if ((fsize = filesize(filename))<0) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "invalid filesize: %s", filename);
    goto main_finish;
  }
  
  // ----------------------------------------
  // Frame registration
  // ----------------------------------------
  process_flag = FLAG_FIRST_DATA_OF_FILE;
  rec_offset = ftell(f);
  while ((r=fread(record, sizeof(unsigned char), SIZE_HEADER, f))>0) {
    if (r != SIZE_HEADER) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }
    
    wnr = binary2wtn_record(record);
    wnr.error_flag = check_wtn_record(wnr);
    
    r = fread(header, sizeof(unsigned char), SIZE_HEADER, f);
    if (r != SIZE_HEADER) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
		 "invalid data size: %zd", r);
      goto main_finish;
    }
    
    // check duplicated header
    for(i=0;i<SIZE_HEADER;++i) {
      if (record[i] != header[i]) {
	log_printf(LOG_WARNING, __FILE__, __LINE__,
		   "header is not duplicated.");
	fseek(f, -SIZE_HEADER, SEEK_CUR);
	count_header = 1;
	break;	
      }
    }
    
    // Allocate memory
    max_wtn_frame = (fsize-SIZE_HEADER*count_header)/SIZE_FRAME;
    if ((fsize-SIZE_HEADER*count_header)%SIZE_FRAME!=0) {
      max_wtn_frame++;
    }
    wnf = (wtn_frame*)malloc(max_wtn_frame * sizeof(wtn_frame));
    if (wnf == NULL) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
		 "cannot allocate memory\n");
      goto main_finish;
    }
    
    print_pg_copy_init();
    
    // Read Frame
    int fmax = 0;
    while ((r=fread(frame, sizeof(unsigned char), SIZE_FRAME, f))>0) {
      if ( fmax >= max_wtn_frame ) {
	log_printf(LOG_ERROR, __FILE__, __LINE__,
		   "max_wtn_frame is %d\n", max_wtn_frame);
	goto main_finish;
      }
      wnf[fmax] = binary2wtn_frame(wnr,frame);
      fmax++;
    }
    
    // Register first frame into database
    for (i=0; i<wnr.num_asta; i++) {
      if (wnf[i].alsep_package_id == ALSEP_PACKAGE_ID_APOLLO_17 &&
          wnf[i].alsep_package_id == wnf[i-1].alsep_package_id) {
	set_independent_data(&wnf[i]);
	wnf[i].process_flag |= process_flag | FLAG_TOP_OF_RECORD;

	wnf[i].error_flag = check_wtn_frame(wnf[i], wnr.year);
	print_pg_copy(id, SIZE_HEADER*count_header+i*SIZE_FRAME, SIZE_FRAME, wnr, wnf[i]);
      }
    }
    
    for(i = wnr.num_asta; i<fmax ;i++) {
      if (wnf[i].alsep_package_id == ALSEP_PACKAGE_ID_APOLLO_17 &&
          wnf[i].alsep_package_id == wnf[i-wnr.num_asta].alsep_package_id) {
	set_related_data(&wnf[i], &wnf[i-wnr.num_asta]);
      }
      wnf[i].error_flag = check_wtn_frame(wnf[i], wnr.year);      
      print_pg_copy(id, SIZE_HEADER*count_header+i*SIZE_FRAME, SIZE_FRAME, wnr, wnf[i]);
    }
    
    msec_of_year_fmax = wnf[i-1].msec_of_year;
  }
  printf("\\.\n");
  
 main_finish:
  if (f) {
    fclose(f);
  }
  
  if (wnf) {
    free(wnf);
    wnf = NULL;
  }
  
  putchar('\n');
  return 0;
}

void print_pg_copy_init() {
  printf("COPY tbl_lsg ("
	 "file_id, pos, length, frame_count, ap_station, ground_station,"
	 "time_original, \"time\", time_diff,"
	 "lsg,lsg_tide,lsg_free,lsg_temp, process_flag, error_flag, time_flag"
	 ") FROM stdin;\n");
}

void print_pg_copy(int id, int offset, int len, wtn_record wnr, wtn_frame wnf) {
  
  int apollo_station;
  
  char time_org[SIZE_TIME_STRING];
  char time[SIZE_TIME_STRING];
  uint32_t doy, hh, mm, ss, ms;
  char sql_lsg[SIZE_SQL];
  
  apollo_station = package_id2station_id(wnf.alsep_package_id);
  
  if (apollo_station != 17) {
    return;
  }
  
  msec_of_year_to_date(wnf.msec_of_year, &doy, &hh, &mm, &ss, &ms);
  
  if (doy >= 1 && doy <= 366) {
    sprintf(time,"%04d.%03d %02d:%02d:%02d.%03d", wnr.year, doy, hh,mm,ss,ms);
  } else {
    sprintf(time,"\\N");
  }
  strcpy(time_org, time);
  
  
  printf(
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%s\t"
	 "%s\t"
	 "%"PRId64"\t"
	 "{%s}\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\n",
	 id,
	 offset,
	 len,
	 wnf.frame_count,
	 apollo_station,
	 wnf.alsep_tracking_station_id,
	 time_org,
	 time_org,
	 wnf.time_diff,
	 intary2str(wnf.lsg,COUNTS_PER_FRAME_FOR_WTN_LSG,sql_lsg,SIZE_SQL),
	 wnf.lsg_tide,
	 wnf.lsg_free,
	 wnf.lsg_temp,
	 wnf.process_flag,
	 wnf.error_flag,
	 0);
}

void set_independent_data(wtn_frame* wnf) {
  wnf->time_diff = wnf->msec_of_year;
  wnf->prev_frame = -1;
}

void set_related_data(wtn_frame* wnf, wtn_frame* before) {
  wnf->time_diff = wnf->msec_of_year - before->msec_of_year;
  wnf->prev_frame = before->frame_count;
}
