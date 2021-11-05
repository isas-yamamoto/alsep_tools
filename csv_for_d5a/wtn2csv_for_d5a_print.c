#include <stdio.h>
#include <inttypes.h>
#include "define.h"
#include "error.h"
#include "util.h"
#include "wtn.h"
#include "wtn2csv_for_d5a_print.h"

void print_wtn_record_header(FILE *f, wtn_record *wnr)
{
    int i;
    fprintf(f, "%d", wnr->id_normal);
    for (i = 0; i < 5; ++i)
    {
        fprintf(f, ",%d", wnr->active_station[i]);
    }
    fprintf(f, ",%d,%d,%d,%" PRId64,
            wnr->num_asta, wnr->original_id, wnr->year, wnr->first_msec);
}

void print_wtn_frame_header(FILE *f, wtn_frame *wnf)
{
    int apollo_station = package_id2station_id(wnf->alsep_package_id);
    fprintf(f, "%d,%" PRId64 ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            wnf->flag_bit,
            wnf->msec_of_year,
            apollo_station,
            wnf->alsep_tracking_station_id,
            wnf->alsep_package_id,
            wnf->bit_search,
            wnf->bit_verify,
            wnf->bit_confirm,
            wnf->bit_loc,
            wnf->bit_il,
            wnf->original_rec_num,
            wnf->sync_code,
            wnf->sync_code_comp,
            wnf->frame_count,
            wnf->mode_bit);
}

void print_headers(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr, wtn_frame *wnf)
{
    char date_string[SIZE_TIME_STRING];
    int32_t ret;
    fprintf(f, "%s,%"PRId64",", filename, file_offset);
    print_wtn_record_header(f, wnr);
    fprintf(f, ",");
    print_wtn_frame_header(f, wnf);
    msec_of_year_to_date_string(wnr->year, wnf->msec_of_year, us_offset, date_string);
    fprintf(f, ",%s", date_string);
}

void print_wtn_spz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf,
    int32_t index)
{
    char date_string[SIZE_TIME_STRING];
    msec_of_year_to_date_string(wnr->year, wnf->msec_of_year, us_offset, date_string);
    print_headers(f, filename, file_offset, us_offset, wnr, wnf);
    fprintf(f, ",%d\n", wnf->spz[index]);
}

void print_wtn_lsg(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf,
    int32_t index)
{
    char date_string[SIZE_TIME_STRING];
    msec_of_year_to_date_string(wnr->year, wnf->msec_of_year, us_offset, date_string);
    print_headers(f, filename, file_offset, us_offset, wnr, wnf);
    fprintf(f, ",%d\n", wnf->lsg[index]);
}

void print_wtn_lpxyz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf,
    int32_t index)
{
    print_headers(f, filename, file_offset, us_offset, wnr, wnf);
    fprintf(f, ",%d,%d,%d\n", wnf->lpx[index], wnf->lpy[index], wnf->lpz[index]);
}

void print_wtn_tdxy(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf)
{
    print_headers(f, filename, file_offset, us_offset, wnr, wnf);
    fprintf(f, ",%d,%d\n", wnf->TidX, wnf->TidY);
}

void print_wtn_tdzi(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf)
{
    print_headers(f, filename, file_offset, us_offset, wnr, wnf);
    fprintf(f, ",%d,%d\n", wnf->TidZ, wnf->InstT);
}

void print_wtn_meta(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    int32_t frame_no,
    int32_t active_id,
    wtn_record *wnr,
    wtn_frame *wnf)
{
    print_headers(f, filename, file_offset, us_offset, wnr, wnf);
    fprintf(f, ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            frame_no, active_id,
            (wnf->process_flag & FLAG_FIRST_DATA_OF_FILE) ? 1 : 0,
            (wnf->process_flag & FLAG_TOP_OF_RECORD) ? 1 : 0,
            (wnf->process_flag & FLAG_FIRST_DATA_COPIED) ? 1 : 0,
            (wnr->error_flag & ERROR_INVALID_FORMAT) ? 1 : 0,
            (wnr->error_flag & ERROR_INVALID_APOLLO_STATION) ? 1 : 0,
            (wnr->error_flag & ERROR_INVALID_DATETIME) ? 1 : 0, // year
            (wnf->error_flag & ERROR_INVALID_DATETIME) ? 1 : 0, // milliseconds
            (wnf->error_flag & ERROR_INVALID_HK) ? 1 : 0,
            (wnf->error_flag & ERROR_FRAME_SMALL_TIME_ERROR) ? 1 : 0,
            (wnf->error_flag & ERROR_FRAME_LARGE_TIME_ERROR) ? 1 : 0,
            (wnf->error_flag & ERROR_INVALID_SYNC_CODE) ? 1 : 0,
            (wnf->error_flag & ERROR_FRAME_COUNT_SEQUENCE) ? 1 : 0);
}
