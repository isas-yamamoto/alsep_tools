/*! @file pse2csv_for_d5a.c
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
#include "csv.h"

void usage(const char *cmd)
{
  fprintf(stderr, "usage: %s output_dirname psefile\n", cmd);
}

void pse_csv_output(FILE *fps_write[SIZE_FILEPOINTERS],
                    const char *filename, long file_offset,
                    int record_no, int frame_no,
                    pse_record pr, pse_frame pf)
{
  int i;
  uint64_t msec_of_year;
  double dmsec = 64 * 10 / 1060.0 * 1000;

  print_meta(fps_write[FILEPOINTER_META],
             filename,
             file_offset,
             record_no,
             frame_no,
             pf.frame_count,
             pr.year,
             pf.msec_of_year,
             0,
             pf.process_flag, pr.error_flag, pf.error_flag);

  if (pr.format == FORMAT_OLD)
  {
    for (i = 0; i < COUNTS_PER_FRAME_FOR_PSE_SP; ++i)
    {
      msec_of_year = pf.msec_of_year + dmsec * i / COUNTS_PER_FRAME_FOR_PSE_SP;
      print_spz(fps_write[FILEPOINTER_SPZ],
                pr.apollo_station,
                filename,
                pr.year,
                msec_of_year,
                i * 640.0 / 1060 / 32 * 1000,
                pf.spz[i]);
    }
  }

  for (i = 0; i < COUNTS_PER_FRAME_FOR_PSE_LP; ++i)
  {
    msec_of_year = pf.msec_of_year + dmsec * i / COUNTS_PER_FRAME_FOR_PSE_LP;
    print_lpxyz(fps_write[FILEPOINTER_LPXYZ],
                pr.apollo_station,
                filename,
                pr.year,
                msec_of_year,
                i * 640.0 / 1060 / 4 * 1000,
                pf.lpx[i], pf.lpy[i], pf.lpz[i]);
  }

  if (pf.frame_count % 2 == 0)
  {
    print_tdxy(fps_write[FILEPOINTER_TDXY],
               pr.apollo_station,
               filename,
               pr.year,
               msec_of_year,
               0,
               pf.TidX, pf.TidY);
  }
  else
  {
    print_tdzi(fps_write[FILEPOINTER_TDZI],
               pr.apollo_station,
               filename,
               pr.year,
               msec_of_year,
               0,
               pf.TidZ, pf.InstT);
  }
}

int main(int argc, char **argv)
{

  // Generic variables
  FILE *fp_read;
  FILE *fps_write[SIZE_FILEPOINTERS]; // file pointers for write
  size_t r;
  char filename[PATH_MAX + 1];
  char dirname[PATH_MAX + 1];
  char pathname[PATH_MAX + 1];
  uint32_t process_flag;
  int size_part;
  int i;
  long frame_offset;
  char *basec, *bname;
  long file_offset;
  int record_no;
  int frame_no;

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
  fps_write[FILEPOINTER_SPZ] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_lp.csv", dirname, bname);
  fps_write[FILEPOINTER_LPXYZ] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_tdxy.csv", dirname, bname);
  fps_write[FILEPOINTER_TDXY] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_tdzi.csv", dirname, bname);
  fps_write[FILEPOINTER_TDZI] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_meta.csv", dirname, bname);
  fps_write[FILEPOINTER_META] = fopen(pathname, "w");

  // ----------------------------------------
  // Frame
  // ----------------------------------------
  process_flag = FLAG_FIRST_DATA_OF_FILE;
  file_offset = ftell(fp_read);
  record_no = 0;
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
    pf[0].prev_frame = -1;
    pf[0].process_flag = process_flag | FLAG_TOP_OF_RECORD | FLAG_FIRST_DATA_COPIED;
    pf[0].error_flag = check_pse_frame(pf[0], pr.apollo_station, pr.year);

    frame_no = 0;
    pse_csv_output(fps_write, bname, file_offset, record_no, frame_no, pr, pf[0]);
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

      pse_csv_output(fps_write, bname, file_offset, record_no, frame_no, pr, pf[i]);
      frame_no++;
    }
    msec_of_year_fmax = pf[i - 1].msec_of_year;
    process_flag = 0;
    file_offset = ftell(fp_read);
    record_no++;
  }
  fclose(fp_read);
  free(basec);
  return EXIT_SUCCESS;
}
