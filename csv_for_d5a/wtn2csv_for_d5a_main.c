/*! @file wtn2csv.c
 *  @brief Convert from WTN original binary to CSV
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2019/07/17
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
#include "wtn.h"
#include "error.h"
#include "util.h"
#include "wtn2csv_for_d5a_print.h"

void usage(const char *cmd)
{
  fprintf(stderr, "usage: %s dirname wtnfile\n", cmd);
}

void wtn_csv_output(FILE *fps_write[SIZE_WTN_FILEPOINTERS],
                    const char *filename, long file_offset,
                    int frame_no, int active_id,
                    wtn_record wnr, wtn_frame wnf)
{
  int i;
  uint64_t msec_of_year;
  double dmsec = 64 * 10 / 1060.0 * 1000;
  int apollo_station[] = {-1, 12, 15, 16, 14, 17};

  print_wtn_meta(
      fps_write[WTN_FILEPOINTER_META],
      filename,
      file_offset,
      0,
      frame_no, active_id,
      &wnr, &wnf);

  if (wnf.alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17)
  {
    for (i = 0; i < COUNTS_PER_FRAME_FOR_WTN_SP; ++i)
    {
      msec_of_year = wnf.msec_of_year + dmsec * i / COUNTS_PER_FRAME_FOR_WTN_SP;
      print_wtn_spz(fps_write[WTN_FILEPOINTER_SPZ],
                filename,
                file_offset,
                i * dmsec / 32,
                &wnr, &wnf, i);
    }

    for (i = 0; i < COUNTS_PER_FRAME_FOR_WTN_LP; ++i)
    {
      msec_of_year = wnf.msec_of_year + dmsec * i / COUNTS_PER_FRAME_FOR_WTN_LP;
      print_wtn_lpxyz(fps_write[WTN_FILEPOINTER_LPXYZ],
                  filename,
                  file_offset,
                  i * dmsec / 4,
                  &wnr, &wnf, i);
    }

    if (wnf.frame_count % 2 == 0)
    {
      print_wtn_tdxy(fps_write[WTN_FILEPOINTER_TDXY],
                 filename,
                 file_offset,
                 0,
                 &wnr, &wnf);
    }
    else
    {
      print_wtn_tdzi(fps_write[WTN_FILEPOINTER_TDZI],
                 filename,
                 file_offset,
                 0,
                 &wnr, &wnf);
    }
  }
  else
  {
    for (i = 0; i < COUNTS_PER_FRAME_FOR_WTN_LSG; ++i)
    {
      print_wtn_lsg(fps_write[WTN_FILEPOINTER_LSG],
                filename,
                file_offset,
                dmsec * i / COUNTS_PER_FRAME_FOR_WTN_LSG,
                &wnr, &wnf, i);
    }
  }
}

int main(int argc, char **argv)
{

  // Generic variables
  FILE *fp_read;
  FILE *fps_write[SIZE_WTN_FILEPOINTERS];
  size_t r;
  char filename[PATH_MAX + 1];
  char dirname[PATH_MAX + 1];
  char pathname[PATH_MAX + 1];
  uint32_t process_flag = 0;
  uint32_t error_flag;
  int i;
  char *basec = NULL;
  char *bname = NULL;
  long file_offset;
  int frame_no;

  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_HEADER];
  unsigned char header[SIZE_HEADER];
  unsigned char frame[SIZE_FRAME];
  wtn_record wnr;
  wtn_frame *wnf = NULL;
  int fsize;
  int max_wtn_frame;
  int fmax = -1;
  int num_header = 2;

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

  // get filesize
  if ((fsize = filesize(filename)) < 0)
  {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "invalid filesize: %s", filename);
    goto main_finish;
  }

  basec = strdup(filename);
  bname = basename(basec);

  mkdir(dirname, S_IRWXU);

  // ----------------------------------------
  // Prepare File Pointers
  // ----------------------------------------
  sprintf(pathname, "%s/%s_spz.csv", dirname, bname);
  fps_write[WTN_FILEPOINTER_SPZ] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_lp.csv", dirname, bname);
  fps_write[WTN_FILEPOINTER_LPXYZ] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_tdxy.csv", dirname, bname);
  fps_write[WTN_FILEPOINTER_TDXY] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_tdzi.csv", dirname, bname);
  fps_write[WTN_FILEPOINTER_TDZI] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_lsg.csv", dirname, bname);
  fps_write[WTN_FILEPOINTER_LSG] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_meta.csv", dirname, bname);
  fps_write[WTN_FILEPOINTER_META] = fopen(pathname, "w");

  // ----------------------------------------
  // Frame registration
  // ----------------------------------------
  file_offset = ftell(fp_read);
  while ((r = fread(record, sizeof(unsigned char), SIZE_HEADER, fp_read)) > 0)
  {
    if (r != SIZE_HEADER)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }

    wnr = binary2wtn_record(record);
    error_flag = check_wtn_record(wnr);

    if (ERROR_INVALID_FORMAT && error_flag)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid format");
      goto main_finish;
    }

    r = fread(header, sizeof(unsigned char), SIZE_HEADER, fp_read);
    if (r != SIZE_HEADER)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }

    // check duplicated header
    for (i = 0; i < SIZE_HEADER; ++i)
    {
      if (record[i] != header[i])
      {
        log_printf(LOG_ERROR, __FILE__, __LINE__,
                   "header is not duplicated.");
        fseek(fp_read, -SIZE_HEADER, SEEK_CUR);
        num_header = 1;
        goto main_finish;
      }
    }

    // Allocate memory
    max_wtn_frame = (fsize - SIZE_HEADER * num_header) / SIZE_FRAME;
    if ((fsize - SIZE_HEADER * num_header) % SIZE_FRAME != 0)
    {
      max_wtn_frame++;
    }
    wnf = (wtn_frame *)malloc(max_wtn_frame * sizeof(wtn_frame));
    if (wnf == NULL)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "cannot allocate memory\n");
      goto main_finish;
    }

    // Read Frame
    fmax = 0;
    frame_no = 0;
    while ((r = fread(frame, sizeof(unsigned char), SIZE_FRAME, fp_read)) > 0)
    {
      if (fmax >= max_wtn_frame)
      {
        log_printf(LOG_ERROR, __FILE__, __LINE__,
                   "max_wtn_frame is %d\n", max_wtn_frame);
        goto main_finish;
      }
      wnf[fmax] = binary2wtn_frame(wnr, frame);
      fmax++;
    }

    // first frame
    for (i = 0; i < wnr.num_asta; i++)
    {
      if (wnf[i].alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17)
      {
        wnf[i].spz[0] = wnf[i].spz[1];
      }
      wnf[i].time_diff = wnf[i].msec_of_year;
      wnf[i].process_flag = process_flag | FLAG_TOP_OF_RECORD | FLAG_FIRST_DATA_COPIED;
      wnf[i].prev_frame = -1;

      wnf[i].error_flag = check_wtn_frame(wnf[i], wnr.year);
      if (wnf[i].error_flag)
      {
        log_printf(LOG_WARNING, __FILE__, __LINE__,
                   "frame error: error code=0x%04x  file offset=%d msec_of_year=%" PRId64,
                   wnf[i].error_flag,
                   SIZE_HEADER * num_header + SIZE_FRAME * i,
                   wnf[i].msec_of_year);
      }

      wtn_csv_output(fps_write,
                     bname, file_offset,
                     frame_no, i,
                     wnr, wnf[i]);
    }

    for (i = wnr.num_asta; i < fmax; i++)
    {

      if (wnf[i].alsep_package_id == wnf[i - wnr.num_asta].alsep_package_id)
      {
        wnf[i].time_diff = wnf[i].msec_of_year - wnf[i - wnr.num_asta].msec_of_year;
        wnf[i].prev_frame = wnf[i - wnr.num_asta].frame_count;
      }
      else
      {
        wnf[i].time_diff = wnf[i].msec_of_year;
        wnf[i].process_flag = FLAG_FIRST_DATA_COPIED;
        wnf[i].prev_frame = -1;
      }

      wnf[i].error_flag = check_wtn_frame(wnf[i], wnr.year);
      if (wnf[i].error_flag)
      {
        log_printf(LOG_WARNING, __FILE__, __LINE__,
                   "frame error: error code=0x%04x  file offset=%d msec_of_year=%" PRId64,
                   wnf[i].error_flag,
                   SIZE_HEADER * num_header + SIZE_FRAME * i,
                   wnf[i].msec_of_year);
      }

      if (wnf[i].alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17)
      {
        if (wnf[i].process_flag & FLAG_FIRST_DATA_COPIED)
        {
          wnf[i].spz[0] = wnf[i].spz[1];
        }
        else
        {
          //! ALSEP WORD 2
          wnf[i].spz[0] = interp(wnf[i - wnr.num_asta].spz[30],
                                 wnf[i - wnr.num_asta].spz[31],
                                 wnf[i].spz[1],
                                 wnf[i].spz[2]);
        }
      }
      wtn_csv_output(fps_write,
                     bname, file_offset,
                     frame_no, i,
                     wnr, wnf[i]);
    }
    frame_no++;
    file_offset = ftell(fp_read);
  }

main_finish:
  if (fp_read)
  {
    fclose(fp_read);
  }

  if (wnf)
  {
    free(wnf);
    wnf = NULL;
  }

  if (basec)
  {
    free(basec);
    basec = NULL;
  }
  return 0;
}
