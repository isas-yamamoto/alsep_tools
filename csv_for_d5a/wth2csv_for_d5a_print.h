#ifndef __WTH2CSV_FOR_D5A_PRINT_H__
#define __WTH2CSV_FOR_D5A_PRINT_H__

#define WTH_FILEPOINTER_GP 0
#define WTH_FILEPOINTER_META 1
#define SIZE_WTH_FILEPOINTERS 2

void print_wth_record_header(FILE *f, wth_record *whr);
void print_wth_frame_header(FILE *f, wth_frame *whf);

void print_headers(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    wth_record *whr, wth_frame *whf);

void print_wth_gp(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    wth_record *whr,
    wth_frame *whf,
    int32_t index);

void print_wth_meta(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    int32_t frame_no,
    int32_t active_id,
    wth_record *whr,
    wth_frame *whf);

#endif
