#include <stdio.h>
#include <inttypes.h>
#include "define.h"
#include "error.h"
#include "util.h"
#include "wth.h"
#include "wth2csv_for_d5a_print.h"

void print_wth_record_header(FILE *f, wth_record *whr)
{
    int i;
    fprintf(f, "%d", whr->id);
    for (i = 0; i < 5; ++i)
    {
        fprintf(f, ",%d", whr->active_station[i]);
    }
    fprintf(f, ",%d,%d,%d,%" PRId64,
            whr->num_asta, whr->original_id, whr->year, whr->first_msec);
}

void print_wth_frame_header(FILE *f, wth_frame *whf)
{
    int apollo_station = package_id2station_id(whf->alsep_package_id);
    fprintf(f, "%d,%" PRId64 ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            whf->flag_bit,
            whf->msec_of_year,
            apollo_station,
            whf->alsep_tracking_station_id,
            whf->alsep_package_id,
            whf->bit_search,
            whf->bit_verify,
            whf->bit_confirm,
            whf->bit_loc,
            whf->bit_il,
            whf->original_rec_num,
            whf->sync_code,
            whf->sync_code_comp);
}

void print_headers(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wth_record *whr, wth_frame *whf)
{
    char date_string[SIZE_TIME_STRING];
    fprintf(f, "%s,%"PRId64",", filename, file_offset);
    print_wth_record_header(f, whr);
    fprintf(f, ",");
    print_wth_frame_header(f, whf);
    msec_of_year_to_date_string(whr->year, whf->msec_of_year, us_offset, date_string);
    fprintf(f, ",%s", date_string);
}

void print_wth_gp(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wth_record *whr,
    wth_frame *whf,
    int32_t index)
{
    print_headers(f, filename, file_offset, us_offset, whr, whf);
    fprintf(f, ",%d,%d,%d,%d,%d\n",
            whf->dp1[index], whf->dp6[index], whf->dp11[index], whf->dp16[index],
            whf->status[index]);
}

void print_wth_meta(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    int32_t frame_no,
    int32_t active_id,
    wth_record *whr,
    wth_frame *whf)
{
    print_headers(f, filename, file_offset, us_offset, whr, whf);
    fprintf(f, ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            frame_no, active_id,
            (whf->process_flag & FLAG_FIRST_DATA_OF_FILE) ? 1 : 0,
            (whf->process_flag & FLAG_TOP_OF_RECORD) ? 1 : 0,
            (whf->process_flag & FLAG_FIRST_DATA_COPIED) ? 1 : 0,
            (whr->error_flag & ERROR_INVALID_FORMAT) ? 1 : 0,
            (whr->error_flag & ERROR_INVALID_APOLLO_STATION) ? 1 : 0,
            (whr->error_flag & ERROR_INVALID_DATETIME) ? 1 : 0, // year
            (whf->error_flag & ERROR_INVALID_DATETIME) ? 1 : 0, // milliseconds
            (whf->error_flag & ERROR_INVALID_HK) ? 1 : 0,
            (whf->error_flag & ERROR_FRAME_SMALL_TIME_ERROR) ? 1 : 0,
            (whf->error_flag & ERROR_FRAME_LARGE_TIME_ERROR) ? 1 : 0,
            (whf->error_flag & ERROR_INVALID_SYNC_CODE) ? 1 : 0,
            (whf->error_flag & ERROR_FRAME_COUNT_SEQUENCE) ? 1 : 0);
}
