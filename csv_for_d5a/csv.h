#ifndef __CSV_H__
#define __CSV_H__
#include <stdint.h>

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
