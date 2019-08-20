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
    uint32_t frame_error
    );
#endif
