#ifndef __WTN2CSV_FOR_D5A_PRINT_H__

#define WTN_FILEPOINTER_SPZ 0
#define WTN_FILEPOINTER_LPXYZ 1
#define WTN_FILEPOINTER_TDXY 2
#define WTN_FILEPOINTER_TDZI 3
#define WTN_FILEPOINTER_META 4
#define WTN_FILEPOINTER_LSG 5
#define SIZE_WTN_FILEPOINTERS 6


void print_wtn_record_header(FILE *f, wtn_record *wnr);
void print_wtn_frame_header(FILE *f, wtn_frame *wnf);

void print_headers(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr, wtn_frame *wnf);

void print_wtn_spz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf,
    int32_t index);

void print_wtn_lsg(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf,
    int32_t index);

void print_wtn_lpxyz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf,
    int32_t index);

void print_wtn_tdxy(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf);

void print_wtn_tdzi(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    wtn_record *wnr,
    wtn_frame *wnf);

void print_wtn_meta(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    double us_offset,
    int32_t record_no,
    int32_t frame_no,
    wtn_record *wnr,
    wtn_frame *wnf);

#endif
