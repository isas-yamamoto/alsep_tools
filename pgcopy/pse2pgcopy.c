/*! @file pse2pgcopy.c
 *  @brief PSEデータをRDBMSに格納するプログラム
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2011/02/01
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#include "define.h"
#include "pse.h"
#include "error.h"
#include "util.h"

#define SET_ARG(var,n,size) {strncpy(var, argv[n], size); var[size] = '\0';}

void print_sql(int id, int offset, int len, pse_record pr, pse_frame pf);

void print_pg_copy_init();
void print_pg_copy(int id, int offset, int len, pse_record pr, pse_frame pf);

void usage(const char* cmd) {
  fprintf(stderr, "%s [-y year] id filename\n", cmd);
}

int main(int argc, char** argv) {
  
  // ----------------------------------------
  // Generic variables
  // ----------------------------------------
  int id;
  FILE *f;
  size_t r;
  char filename[PATH_MAX+1];
  uint32_t process_flag;
  int size_part;
  int i;
  long rec_offset, frame_offset;
  
  //initial value of Frame time error at last frame in one record
  uint64_t msec_of_year_fmax = 0;

  // ----------------------------------------
  // getopt
  // ----------------------------------------
  int ch;
  extern char *optarg;
  extern int optind, opterr;
  int year_override = -1;
  
  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_RECORD];
  pse_record pr;
  pse_frame pf[MAX_PSE_FRAME+1];

  while ((ch = getopt(argc, argv, "y:")) != -1) {
    switch(ch) {
    case 'y':
      year_override = atoi(optarg);
      break;
    default:
      usage(argv[0]);
      break;
    }
  }
  argc -= optind;
  if (argc != 2){
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  argv += optind;

  // ----------------------------------------  
  // PROGRAM MAIN
  // ----------------------------------------
  id = atoi(argv[0]);
  SET_ARG(filename,1,PATH_MAX);
  
  f=fopen(filename, "rb");
  if(f==NULL) {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
	       "no such file: %s", filename);
    return -1;
  }
  
  log_printf(LOG_INFO, __FILE__, __LINE__, "processing: %s", filename);
  
  // ----------------------------------------
  // Frame registration
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
    if (year_override != -1) {
      pr.year = year_override;
    }

    pr.error_flag = check_pse_record(pr);

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
    if (pf[0].error_flag >= 0x0100) {
	log_printf(LOG_WARNING, __FILE__, __LINE__,
		   "frame error: code=0x%04x %s offset=%d msec_of_year=%"PRId64,
                   pf[i].error_flag,
                   filename,
		   frame_offset,
                   pf[i].msec_of_year);      
    }
      
    print_pg_copy_init();    
    print_pg_copy(id, rec_offset+frame_offset, size_part, pr,pf[0]);
    
    // register remnant frames into database
    for(i = 1; i < (SIZE_LOGICAL_RECORD * pr.phys_records);i++) {
      frame_offset = SIZE_PSE_HEADER+size_part*i;
      unsigned char* frame = &record[frame_offset];
      pf[i] = binary2pse_frame(pr,frame);
      pf[i].time_diff = pf[i].msec_of_year - pf[i-1].msec_of_year;      
      pf[i].prev_frame = pf[i-1].frame_count;
      pf[i].process_flag = 0;
      pf[i].error_flag = check_pse_frame(pf[i], pr.apollo_station, pr.year);

      if (pf[i].error_flag >= 0x0100) {
	  log_printf(LOG_WARNING, __FILE__, __LINE__,
		     "frame error: code=0x%04x %s offset=%d msec_of_year=%"PRId64,
		     pf[i].error_flag,
                     filename,
		     frame_offset,
		     pf[i].msec_of_year);      
      }
      
      if (pf[i].error_flag == ERROR_NONE) {
	//! ALSEP WORD 2
	pf[i].spz[0] = interp(
			      pf[i-1].spz[30],
			      pf[i-1].spz[31],
			      pf[i  ].spz[ 1],
			      pf[i  ].spz[ 2]);
      } else {
	pf[i].spz[0] = pf[i].spz[1];
	pf[i].process_flag |= FLAG_FIRST_DATA_COPIED;
      }

      print_pg_copy(id, rec_offset+frame_offset, size_part, pr,pf[i]);
    }
    msec_of_year_fmax = pf[i-1].msec_of_year;
    rec_offset = ftell(f);
    printf("\\.\n");
  }

main_finish:
  if (f) {
    fclose(f);
  }
  
  putchar('\n');
  
  return 0;
}

void print_sql(int id, int offset, int len, pse_record pr, pse_frame pf) {
  char time_org[SIZE_TIME_STRING];
  char time[SIZE_TIME_STRING];
  uint32_t doy, hh, mm, ss, ms;
  char sql_spz[SIZE_SQL];
  char sql_lpx[SIZE_SQL], sql_lpy[SIZE_SQL], sql_lpz[SIZE_SQL];

  msec_of_year_to_date(pf.msec_of_year, &doy, &hh, &mm, &ss, &ms);

  if (doy >= 1 && doy <= 366) {
    sprintf(time,"'%04d.%03d %02d:%02d:%02d.%03d'", pr.year, doy, hh,mm,ss,ms);
  } else {
    sprintf(time,"NULL");
  }
  strcpy(time_org, time);
  
  printf(
	 "INSERT INTO TBL_PSE ("
	 "FILE_ID,"
	 "POS,"
	 "LENGTH,"
	 "AP_STATION,"
	 "GROUND_STATION,"
	 "TIME_ORIGINAL,"
	 "TIME,"
	 "SP_Z,"
	 "LP_X,"
	 "LP_Y,"
	 "LP_Z,"
	 "TIDAL_X,"
	 "TIDAL_Y,"
	 "TIDAL_Z,"
	 "INST_TEMP,"
	 "PROCESS_FLAG,"
	 "ERROR_FLAG,"
	 "TIME_FLAG"
	 ") VALUES ("
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "%s,"
	 "%s,"
	 "'{%s}',"
	 "'{%s}',"
	 "'{%s}',"
	 "'{%s}',"
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "'%d',"
	 "'%d');\n",
	 id,
	 offset,
	 len,
	 pr.apollo_station,
	 pf.alsep_tracking_station_id,
	 time_org,
	 time_org,
	 (pr.format == FORMAT_OLD) ? intary2str(pf.spz,COUNTS_PER_FRAME_FOR_PSE_SP,sql_spz,SIZE_SQL) : NULL,
	 intary2str(pf.lpx,COUNTS_PER_FRAME_FOR_PSE_LP,sql_lpx,SIZE_SQL),
	 intary2str(pf.lpy,COUNTS_PER_FRAME_FOR_PSE_LP,sql_lpy,SIZE_SQL),
	 intary2str(pf.lpz,COUNTS_PER_FRAME_FOR_PSE_LP,sql_lpz,SIZE_SQL),
	 pf.TidX, pf.TidY, pf.TidZ, pf.InstT,
	 pf.process_flag,
	 pf.error_flag,
	 0);
}

void print_pg_copy_init() {
  printf("COPY tbl_pse ("
	 "file_id, pos, length, frame_count, ap_station, ground_station,"
	 "time_original, \"time\", time_diff, sp_z, lp_x, lp_y, lp_z,"
	 "tidal_x, tidal_y, tidal_z, inst_temp, process_flag, error_flag, time_flag"
	 ") FROM stdin;\n");
}

void print_pg_copy(int id, int offset, int len, pse_record pr, pse_frame pf) {
  
  char time_org[SIZE_TIME_STRING];
  char time[SIZE_TIME_STRING];
  uint32_t doy, hh, mm, ss, ms;
  char sql_spz[SIZE_SQL];
  char sql_lpx[SIZE_SQL], sql_lpy[SIZE_SQL], sql_lpz[SIZE_SQL];
  
  msec_of_year_to_date(pf.msec_of_year, &doy, &hh, &mm, &ss, &ms);
  
  if (doy >= 1 && doy <= 366) {
    sprintf(time,"%04d.%03d %02d:%02d:%02d.%03d", pr.year, doy, hh,mm,ss,ms);
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
	 "{%s}\t"
	 "{%s}\t"
	 "{%s}\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\t"
	 "%d\n",
	 id,
	 offset,
	 len,
	 pf.frame_count,
	 pr.apollo_station,
	 pf.alsep_tracking_station_id,
	 time_org,
	 time_org,
	 pf.time_diff,
	 (pr.format == FORMAT_OLD) ? intary2str(pf.spz,COUNTS_PER_FRAME_FOR_PSE_SP,sql_spz,SIZE_SQL) : "",
	 intary2str(pf.lpx,COUNTS_PER_FRAME_FOR_PSE_LP,sql_lpx,SIZE_SQL),
	 intary2str(pf.lpy,COUNTS_PER_FRAME_FOR_PSE_LP,sql_lpy,SIZE_SQL),
	 intary2str(pf.lpz,COUNTS_PER_FRAME_FOR_PSE_LP,sql_lpz,SIZE_SQL),
	 pf.TidX, pf.TidY, pf.TidZ, pf.InstT,
	 pf.process_flag,
	 pf.error_flag,
	 0);
}
