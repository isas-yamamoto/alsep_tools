#ifndef __PSE2CSV_FOR_D5A_PRINT_H__

#define PSE_FILEPOINTER_SPZ 0
#define PSE_FILEPOINTER_LPXYZ 1
#define PSE_FILEPOINTER_TDXY 2
#define PSE_FILEPOINTER_TDZI 3
#define PSE_FILEPOINTER_META 4
#define SIZE_PSE_FILEPOINTERS 5

void print_pse_record_header(FILE *f, pse_record *pr);
void print_pse_frame_header(FILE *f, pse_frame *pf);

void print_headers(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr, pse_frame *pf);

void print_pse_spz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf,
    int32_t index);

void print_pse_lpxyz(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf,
    int32_t index);

void print_pse_tdxy(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf);

void print_pse_tdzi(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    pse_record *pr,
    pse_frame *pf);

void print_pse_meta(
    FILE *f,
    const char *filename,
    int64_t file_offset,
    uint32_t us_offset,
    int32_t record_no,
    int32_t frame_no,
    pse_record *pr,
    pse_frame *pf);

#endif
