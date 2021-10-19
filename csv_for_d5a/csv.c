#include <stdio.h>
#include <stdint.h>
#include "pse.h"
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
  printf(",%02d:%02d:%02d.%03d", hh, mm, ss, ms);
  printf(",%d", value);
  printf(",%d", process_flag);
  printf(",%d", record_error);
  printf(",%d", frame_error);
  putchar('\n');
}

void print_spz(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int value)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%d", apollo_station);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%d", value);
  fprintf(f, "\n");
}

void print_lpxyz(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int lpx, int lpy, int lpz)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%d", apollo_station);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%d,%d,%d", lpx, lpy, lpz);
  fprintf(f, "\n");
}

void print_tdxy(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int tdx, int tdy)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%d", apollo_station);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%d,%d", tdx, tdy);
  fprintf(f, "\n");
}

void print_tdzi(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int tdz, int instT)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%d", apollo_station);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%d,%d", tdz, instT);
  fprintf(f, "\n");
}

void print_lsg(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int value)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%d", apollo_station);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%d", value);
  fprintf(f, "\n");
}

void print_gp(
    FILE *f,
    int apollo_station,
    const char *filename,
    int year,
    uint64_t msec_of_year,
    uint32_t us_offset,
    int dp1, int dp6, int dp11, int dp16,
    int status)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%d", apollo_station);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%d,%d,%d,%d", dp1, dp6, dp11, dp16);
  fprintf(f, ",%d", status);
  fprintf(f, "\n");
}

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
    uint32_t frame_error)
{
  char date_string[SIZE_DATE_STRING];
  msec_of_year_to_date_string(year, msec_of_year, us_offset, date_string);
  fprintf(f, "%s", filename);
  fprintf(f, ",%s", date_string);
  fprintf(f, ",%ld", file_offset);
  fprintf(f, ",%d", record_no);
  fprintf(f, ",%d", frame_no);
  fprintf(f, ",%d", frame_count);
  fprintf(f, ",%d", process_flag);
  fprintf(f, ",%d", record_error);
  fprintf(f, ",%d", frame_error);
  fprintf(f, "\n");
}