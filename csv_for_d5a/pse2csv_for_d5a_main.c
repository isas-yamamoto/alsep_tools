/*! @file pse2csv_for_d5a_main.c
 *  @brief Convert from PSE original binary to CSV
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2021/10/19
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>

#include "define.h"
#include "pse.h"
#include "error.h"
#include "util.h"
#include "pse2csv_for_d5a_print.h"


void usage(const char *cmd)
{
  fprintf(stderr, "usage: %s output_dirname psefile\n", cmd);
}

void pse_csv_output(FILE *fps_write[SIZE_PSE_FILEPOINTERS],
                    const char *filename, long file_offset,
                    int record_no, int frame_no,
                    pse_record pr, pse_frame pf)
{
  int i;
  double us_offset;
  double dmsec = 64 * 10 / 1060.0 * 1000;
  print_pse_meta(
      fps_write[PSE_FILEPOINTER_META],
      filename,
      file_offset,
      0,
      record_no, frame_no,
      &pr, &pf);

  if (pr.format == FORMAT_OLD)
  {
    for (i = 0; i < COUNTS_PER_FRAME_FOR_PSE_SP; ++i)
    {
      us_offset = dmsec * i / COUNTS_PER_FRAME_FOR_PSE_SP * 1000;
      print_pse_spz(
          fps_write[PSE_FILEPOINTER_SPZ],
          filename,
          file_offset,
          us_offset,
          &pr, &pf,
          i);
    }
  }

  for (i = 0; i < COUNTS_PER_FRAME_FOR_PSE_LP; ++i)
  {
    us_offset = dmsec * i / COUNTS_PER_FRAME_FOR_PSE_LP * 1000;
    print_pse_lpxyz(
        fps_write[PSE_FILEPOINTER_LPXYZ],
        filename,
        file_offset,
        us_offset,
        &pr, &pf,
        i);
  }

  if (pf.frame_count % 2 == 0)
  {
    print_pse_tdxy(
        fps_write[PSE_FILEPOINTER_TDXY],
        filename,
        file_offset,
        0,
        &pr, &pf);
  }
  else
  {
    print_pse_tdzi(
        fps_write[PSE_FILEPOINTER_TDZI],
        filename,
        file_offset,
        0,
        &pr, &pf);
  }
}

int main(int argc, char **argv)
{

  // Generic variables
  FILE *fp_read;
  FILE *fps_write[SIZE_PSE_FILEPOINTERS]; // file pointers for write
  size_t r;
  char filename[PATH_MAX + 1];
  char dirname[PATH_MAX + 1];
  char pathname[PATH_MAX + 1];
  uint32_t process_flag;
  int size_part;
  int i;
  char *basec, *bname;
  int record_no;
  int frame_no;
  long rec_offset, frame_offset;
  uint32_t prev_frame;

  // initial value of Frame time error at last frame in one record
  uint64_t msec_of_year_fmax = 0;

  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_RECORD];
  pse_record pr;
  pse_frame pf[MAX_PSE_FRAME + 1];

  // ----------------------------------------
  // Command line option
  // ----------------------------------------
  if (argc != 3)
  {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  SET_ARG(dirname, 1, PATH_MAX);
  SET_ARG(filename, 2, PATH_MAX);

  // ----------------------------------------
  // PROGRAM MAIN
  // ----------------------------------------
  fp_read = fopen(filename, "rb");
  if (fp_read == NULL)
  {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "no such file: %s", filename);
    return -1;
  }

  basec = strdup(filename);
  bname = basename(basec);

  mkdir(dirname, S_IRWXU);

  // ----------------------------------------
  // Prepare File Pointers
  // ----------------------------------------
  sprintf(pathname, "%s/%s_spz.csv", dirname, bname);
  fps_write[PSE_FILEPOINTER_SPZ] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_lp.csv", dirname, bname);
  fps_write[PSE_FILEPOINTER_LPXYZ] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_tdxy.csv", dirname, bname);
  fps_write[PSE_FILEPOINTER_TDXY] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_tdzi.csv", dirname, bname);
  fps_write[PSE_FILEPOINTER_TDZI] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_meta.csv", dirname, bname);
  fps_write[PSE_FILEPOINTER_META] = fopen(pathname, "w");

  // ----------------------------------------
  // Frame
  // ----------------------------------------
  process_flag = FLAG_FIRST_DATA_OF_FILE;
  rec_offset = ftell(fp_read);
  record_no = 0;
  prev_frame = -1;
  while ((r = fread(record, sizeof(unsigned char), SIZE_RECORD, fp_read)) > 0)
  {
    if (r != SIZE_RECORD)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__, "invalid data size: %zd", r);
      break;
    }

    pr = binary2pse_record(record);
    pr.error_flag = check_pse_record(pr);

    if (ERROR_INVALID_FORMAT && pr.error_flag)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__, "invalid format");
      break;
    }

    if (pr.format == FORMAT_OLD)
    {
      size_part = SIZE_DATA_PART_OLD;
    }
    else
    {
      size_part = SIZE_DATA_PART_NEW;
    }

    // register first frame into database
    frame_offset = SIZE_PSE_HEADER;
    pf[0] = binary2pse_frame(pr, &record[frame_offset]);
    pf[0].spz[0] = pf[0].spz[1];
    pf[0].time_diff = pf[0].msec_of_year - msec_of_year_fmax;
    pf[0].prev_frame = prev_frame;
    pf[0].process_flag = process_flag | FLAG_TOP_OF_RECORD | FLAG_FIRST_DATA_COPIED;
    pf[0].error_flag = check_pse_frame(pf[0], pr.apollo_station, pr.year);

    frame_no = 0;
    pse_csv_output(fps_write, bname, rec_offset + frame_offset, record_no, frame_no, pr, pf[0]);
    frame_no++;

    // register remnant frames into database
    for (i = 1; i < (SIZE_LOGICAL_RECORD * pr.phys_records); i++)
    {
      frame_offset = SIZE_PSE_HEADER + size_part * i;
      unsigned char *frame = &record[frame_offset];

      pf[i] = binary2pse_frame(pr, frame);
      pf[i].time_diff = pf[i].msec_of_year - pf[i - 1].msec_of_year;
      pf[i].prev_frame = pf[i - 1].frame_count;
      pf[i].process_flag = 0;
      pf[i].error_flag = check_pse_frame(pf[i], pr.apollo_station, pr.year);

      //! ALSEP WORD 2
      pf[i].spz[0] = interp(
          pf[i - 1].spz[30],
          pf[i - 1].spz[31],
          pf[i].spz[1],
          pf[i].spz[2]);

      pse_csv_output(fps_write, bname, rec_offset + frame_offset, record_no, frame_no, pr, pf[i]);
      frame_no++;
    }
    msec_of_year_fmax = pf[i - 1].msec_of_year;
    process_flag = 0;
    rec_offset = ftell(fp_read);
    prev_frame = pf[i - 1].frame_count;
    record_no++;
  }
  fclose(fp_read);
  free(basec);
  return EXIT_SUCCESS;
}
