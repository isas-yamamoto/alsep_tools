#include <stdio.h>
#include <inttypes.h>
#include "define.h"
#include "error.h"
#include "util.h"
#include "pse.h"
#include "pse2csv_for_d5a_print.h"

void print_pse_record_header(FILE *f, pse_record *pr)
{
  fprintf(f, "%d,%d,%d,%d,%d,%d,%d,%d",
          pr->tape_type,
          pr->apollo_station,
          pr->tape_seq,
          pr->record_number,
          pr->year,
          pr->format,
          pr->phys_records,
          pr->read_err);
}

void print_pse_frame_header(FILE *f, pse_frame *pf)
{
  fprintf(f, "%d,%" PRId64 ",%d,%d,%d,%d,%d,%d,%d,%d",
          pf->software_time_flag,
          pf->msec_of_year,
          pf->alsep_tracking_station_id,
          pf->bit_error_rate,
          pf->data_rate,
          pf->alsep_word5,
          pf->sync_code,
          pf->sync_code_comp,
          pf->frame_count,
          pf->mode_bit);
}

void print_headers(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr, pse_frame *pf)
{
  char date_string[SIZE_TIME_STRING];
  fprintf(f, "%s,%ld,", filename, file_offset);
  print_pse_record_header(f, pr);
  fprintf(f, ",");
  print_pse_frame_header(f, pf);
  msec_of_year_to_date_string(pr->year, pf->msec_of_year, us_offset, date_string);
  fprintf(f, ",%s", date_string);
}

void print_pse_spz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf,
    int32_t index)
{
  char date_string[SIZE_TIME_STRING];
  msec_of_year_to_date_string(pr->year, pf->msec_of_year, us_offset, date_string);
  print_headers(f, filename, file_offset, us_offset, pr, pf);
  fprintf(f, ",%d\n", pf->spz[index]);
}

void print_pse_lpxyz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf,
    int32_t index)
{
  print_headers(f, filename, file_offset, us_offset, pr, pf);
  fprintf(f, ",%d,%d,%d\n", pf->lpx[index], pf->lpy[index], pf->lpz[index]);
}

void print_pse_tdxy(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf)
{
  print_headers(f, filename, file_offset, us_offset, pr, pf);
  fprintf(f, ",%d,%d\n", pf->TidX, pf->TidY);
}

void print_pse_tdzi(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf)
{
  print_headers(f, filename, file_offset, us_offset, pr, pf);
  fprintf(f, ",%d,%d\n", pf->TidZ, pf->InstT);
}

void print_pse_meta(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    int32_t record_no,
    int32_t frame_no,
    pse_record *pr,
    pse_frame *pf)
{
  print_headers(f, filename, file_offset, us_offset, pr, pf);
  fprintf(f, ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
          record_no, frame_no,
          (pf->process_flag & FLAG_FIRST_DATA_OF_FILE) ? 1 : 0,
          (pf->process_flag & FLAG_TOP_OF_RECORD) ? 1 : 0,
          (pf->process_flag & FLAG_FIRST_DATA_COPIED) ? 1 : 0,
          (pr->error_flag & ERROR_INVALID_FORMAT) ? 1 : 0,
          (pr->error_flag & ERROR_INVALID_APOLLO_STATION) ? 1 : 0,
          (pr->error_flag & ERROR_INVALID_DATETIME) ? 1 : 0, // year
          (pf->error_flag & ERROR_INVALID_DATETIME) ? 1 : 0, // milliseconds
          (pf->error_flag & ERROR_INVALID_HK) ? 1 : 0,
          (pf->error_flag & ERROR_FRAME_SMALL_TIME_ERROR) ? 1 : 0,
          (pf->error_flag & ERROR_FRAME_LARGE_TIME_ERROR) ? 1 : 0,
          (pf->error_flag & ERROR_INVALID_SYNC_CODE) ? 1 : 0,
          (pf->error_flag & ERROR_FRAME_COUNT_SEQUENCE) ? 1 : 0);
}
