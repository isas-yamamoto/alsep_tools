#include <stdio.h>
#include <stdint.h>
#include "csv.h"
#include "util.h"

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
    uint32_t frame_error)
{
  uint32_t doy, hh, mm, ss, ms;
  msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
  printf("%s", filename);
  printf(",%d", apollo_station);
  printf(",%s", data_type);
  printf(",%d", frame_count);
  printf(",%d", year);
  printf(",%d", doy);
  printf("%02d:%02d:%02d.%03d", hh, mm, ss, ms);
  printf(",%d", value);
  printf(",%d", process_flag);
  printf(",%d", record_error);
  printf(",%d", frame_error);
  putchar('\n');
}
