/*! @file wth2pgcopy.c
 *  @brief WTHデータをRDBMSに格納するプログラム
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2012/02/24
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#include "define.h"
#include "wth.h"
#include "error.h"
#include "util.h"

#define SET_ARG(var,n,size) {strncpy(var, argv[n], size); var[size] = '\0';}

void print_pg_copy_init();
void print_pg_copy(int id, int offset, int len, wth_record whr, wth_frame whf);
void set_independent_data(wth_frame* whf);
void set_related_data(wth_frame* whf, wth_frame* before);

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
  wth_record whr;
  wth_frame* whf = NULL;
  int fsize;
  int max_wth_frame;
  int num_header = 2;
  
  // PROGRAM MAIN
  if (argc < 3){
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
    
    whr = binary2wth_record(record);
    whr.error_flag = check_wth_record(whr);
    
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
	num_header = 1;
	break;	
      }
    }
    
    // Allocate memory
    max_wth_frame = (fsize-SIZE_HEADER*2)/SIZE_FRAME;
    if ((fsize-SIZE_HEADER*num_header)%SIZE_FRAME!=0) {
      max_wth_frame++;
    }
    whf = (wth_frame*)malloc(max_wth_frame * sizeof(wth_frame));
    if (whf == NULL) {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
		 "cannot allocate memory\n");
      goto main_finish;
    }
    
    print_pg_copy_init();
    
    // Read Frame
    int fmax = 0;
    while ((r=fread(frame, sizeof(unsigned char), SIZE_FRAME, f))>0) {
      if ( fmax >= max_wth_frame ) {
	log_printf(LOG_ERROR, __FILE__, __LINE__,
		   "max_wth_frame is %d\n", max_wth_frame);
	goto main_finish;
      }
      whf[fmax] = binary2wth_frame(whr,frame);
      fmax++;
    }
    
    // Register first frame into database
    for (i=0; i<whr.num_asta; i++) {
      
      set_independent_data(&whf[i]);
      whf[i].process_flag |= process_flag | FLAG_TOP_OF_RECORD;
      
      whf[i].error_flag = check_wth_frame(whf[i], whr.year);
      if (whf[i].error_flag >= 0x0100) {
	log_printf(LOG_WARNING, __FILE__, __LINE__,
		   "frame error: error code=0x%04x  file offset=%d msec_of_year=%"PRId64,
                   whf[i].error_flag,
		   SIZE_HEADER*num_header+SIZE_FRAME*i,
                   whf[i].msec_of_year);
      }
      print_pg_copy(id, SIZE_HEADER*num_header+i*SIZE_FRAME, SIZE_FRAME, whr, whf[i]);
    }
    
    for(i = whr.num_asta; i<fmax ;i++) {
      
      if (whf[i].alsep_package_id != whf[i-whr.num_asta].alsep_package_id) {
	set_independent_data(&whf[i]);
      } else {
	set_related_data(&whf[i], &whf[i-whr.num_asta]);
      }
      
      whf[i].error_flag = check_wth_frame(whf[i], whr.year);
      if (whf[i].error_flag >= 0x0100) {
	log_printf(LOG_WARNING, __FILE__, __LINE__,
		   "frame error: error code=0x%04x  file offset=%d msec_of_year=%"PRId64,
                   whf[i].error_flag,
		   SIZE_HEADER*num_header+SIZE_FRAME*i,
                   whf[i].msec_of_year);
      }
      
      print_pg_copy(id, SIZE_HEADER*num_header+i*SIZE_FRAME, SIZE_FRAME, whr, whf[i]);
    }
    
    msec_of_year_fmax = whf[i-1].msec_of_year;
  }
  printf("\\.\n");
  
 main_finish:
  if (f) {
    fclose(f);
  }
  
  if (whf) {
    free(whf);
    whf = NULL;
  }
  
  putchar('\n');
  return 0;
}

void print_pg_copy_init() {
  printf("COPY tbl_lspe ("
	 "file_id, pos, length, ap_station, ground_station,"
	 " time_original, \"time\", time_diff, gp1, gp2, gp3, gp4, status,"
	 " process_flag, error_flag, time_flag"
	 ") FROM stdin;\n");
}

void print_pg_copy(int id, int offset, int len, wth_record whr, wth_frame whf) {

  int apollo_station;

  char time_org[SIZE_TIME_STRING];
  char time[SIZE_TIME_STRING];
  uint32_t doy, hh, mm, ss, ms;
  char sql_gp1[SIZE_SQL], sql_gp2[SIZE_SQL], sql_gp3[SIZE_SQL], sql_gp4[SIZE_SQL];
  char sql_status[SIZE_SQL];
  
  msec_of_year_to_date(whf.msec_of_year, &doy, &hh, &mm, &ss, &ms);
  
  if (doy >= 1 && doy <= 366) {
    sprintf(time,"%04d.%03d %02d:%02d:%02d.%03d", whr.year, doy, hh,mm,ss,ms);
  } else {
    sprintf(time,"\\N");
  }
  strcpy(time_org, time);

  apollo_station = package_id2station_id(whf.alsep_package_id);
  if (apollo_station != 17) {
    return;
  }
  
  printf(
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%s\t"
	 "%s\t"
	 "%"PRId64"\t"
	 "{%s}\t"
	 "{%s}\t"
	 "{%s}\t"
	 "{%s}\t"
	 "{%s}\t"
	 "%d\t"
	 "%d\t"
	 "%d\n",
	 id,
	 offset,
	 len,
	 apollo_station,
	 whf.alsep_tracking_station_id,
	 time_org,
	 time_org,
	 whf.time_diff,
	 intary2str(whf.dp1,COUNTS_PER_FRAME_FOR_WTH_GP,sql_gp1,SIZE_SQL),
	 intary2str(whf.dp6,COUNTS_PER_FRAME_FOR_WTH_GP,sql_gp2,SIZE_SQL),
	 intary2str(whf.dp11,COUNTS_PER_FRAME_FOR_WTH_GP,sql_gp3,SIZE_SQL),
	 intary2str(whf.dp16,COUNTS_PER_FRAME_FOR_WTH_GP,sql_gp4,SIZE_SQL),
	 intary2str(whf.status,COUNTS_PER_FRAME_FOR_WTH_GP,sql_status,SIZE_SQL),
	 whf.process_flag,
	 whf.error_flag,
	 0);
}


void set_independent_data(wth_frame* whf) {
  if (whf->alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17) {
    whf->process_flag = FLAG_FIRST_DATA_COPIED;
  }
  whf->time_diff = whf->msec_of_year;
  whf->prev_frame = -1;
}

void set_related_data(wth_frame* whf, wth_frame* before) {
  whf->time_diff = whf->msec_of_year - before->msec_of_year;
}
