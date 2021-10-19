#ifndef __CSV_H__
#define __CSV_H__
#include <stdint.h>

#define FILEPOINTER_SPZ 0
#define FILEPOINTER_LPXYZ 1
#define FILEPOINTER_TDXY 2
#define FILEPOINTER_TDZI 3
#define FILEPOINTER_LSG 4
#define FILEPOINTER_GP 5
#define FILEPOINTER_META 6
#define SIZE_FILEPOINTERS 7

void print_format(
    const char *filename,
    int year,
    uint64_t msec_of_year,
    int apollo_station,
    const char *data_type,
    int frame_count,
    int value,
    uint32_t process_flag,
    uint32_t record_error,
    uint32_t frame_error);

void print_spz(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int value);

void print_lpxyz(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int lpx, int lpy, int lpz);

void print_tdxy(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int tdx, int tdy);

void print_tdzi(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int tdz, int instT);

void print_lsg(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int value);

void print_gp(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int dp1, int dp6, int dp11, int dp16,
    int status);

void print_meta(
    FILE *f,
    const char *filename,
    long file_offset,
    int record_no,
    int frame_no,
    int frame_count,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    uint32_t process_flag,
    uint32_t record_error,
    uint32_t frame_error);

#endif
