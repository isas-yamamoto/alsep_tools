/*! @file wth2csv.c
 *  @brief Convert from WTH original binary to CSV
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2019/07/17
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>

#include "define.h"
#include "wth.h"
#include "error.h"
#include "util.h"
#include "csv.h"

void usage(const char *cmd)
{
  fprintf(stderr, "usage: %s dirname wthfile\n", cmd);
}

void wth_csv_output(FILE *fps_write[SIZE_FILEPOINTERS],
                    const char *filename, long file_offset,
                    int record_no, int frame_no,
                    wth_record whr, wth_frame whf)
{
  int i;
  uint64_t msec_of_year;
  double dmsec = 20 * 30 / 3533.0 * 1000;

  for (i = 0; i < COUNTS_PER_FRAME_FOR_WTH_GP; ++i)
  {
    msec_of_year = whf.msec_of_year + dmsec * i / COUNTS_PER_FRAME_FOR_WTH_GP;
    print_gp(fps_write[FILEPOINTER_GP],
             17,
             filename,
             whr.year,
             msec_of_year,
             i * dmsec / COUNTS_PER_FRAME_FOR_WTH_GP,
             whf.dp1[i], whf.dp6[i], whf.dp11[i], whf.dp16[i], whf.status[i]);
  }
}

int main(int argc, char **argv)
{

  // Generic variables
  FILE *fp_read = NULL;
  FILE *fps_write[SIZE_FILEPOINTERS];
  size_t r;
  char filename[PATH_MAX + 1];
  char dirname[PATH_MAX + 1];
  char pathname[PATH_MAX + 1];
  int error_flag;
  int i;
  char *basec, *bname;
  long file_offset;

  // ----------------------------------------
  // Apollo related variables
  // ----------------------------------------
  unsigned char record[SIZE_HEADER];
  unsigned char header[SIZE_HEADER];
  unsigned char frame[SIZE_FRAME];
  wth_record whr;
  wth_frame *whf = NULL;
  int fsize;
  int max_wth_frame;
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

  mkdir(dirname, S_IRWXU);

  // ----------------------------------------
  // PROGRAM MAIN
  // ----------------------------------------
  fp_read = fopen(filename, "rb");
  if (fp_read == NULL)
  {
    log_printf(LOG_ERROR, __FILE__, __LINE__,
               "no such file: %s", filename);
    return EXIT_FAILURE;
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

  // ----------------------------------------
  // Prepare File Pointers
  // ----------------------------------------
  sprintf(pathname, "%s/%s_gp.csv", dirname, bname);
  fps_write[FILEPOINTER_GP] = fopen(pathname, "w");

  sprintf(pathname, "%s/%s_meta.csv", dirname, bname);
  fps_write[FILEPOINTER_META] = fopen(pathname, "w");

  // ----------------------------------------
  // Frame registration
  // ----------------------------------------
  while ((r = fread(record, sizeof(unsigned char), SIZE_HEADER, fp_read)) > 0)
  {
    if (r != SIZE_HEADER)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "invalid data size: %zd", r);
      goto main_finish;
    }

    whr = binary2wth_record(record);
    error_flag = check_wth_record(whr);

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
        goto main_finish;
      }
    }

    // Allocate memory
    max_wth_frame = (fsize - SIZE_HEADER * num_header) / SIZE_FRAME;
    if ((fsize - SIZE_HEADER * num_header) % SIZE_FRAME != 0)
    {
      max_wth_frame++;
    }
    whf = (wth_frame *)malloc(max_wth_frame * sizeof(wth_frame));
    if (whf == NULL)
    {
      log_printf(LOG_ERROR, __FILE__, __LINE__,
                 "cannot allocate memory\n");
      goto main_finish;
    }

    // Read Frame
    fmax = 0;
    file_offset = ftell(fp_read);
    while ((r = fread(frame, sizeof(unsigned char), SIZE_FRAME, fp_read)) > 0)
    {
      if (fmax >= max_wth_frame)
      {
        log_printf(LOG_ERROR, __FILE__, __LINE__,
                   "max_wth_frame is %d\n", max_wth_frame);
        goto main_finish;
      }
      whf[fmax] = binary2wth_frame(whr, frame);
      error_flag = check_wth_frame(whf[fmax], whr.year);
      wth_csv_output(fps_write,
                     bname, file_offset,
                     fmax, i,
                     whr, whf[fmax]);
    }
    fmax++;
    file_offset = ftell(fp_read);
  }

main_finish:
  if (fp_read)
  {
    fclose(fp_read);
    fp_read = NULL;
  }

  if (whf)
  {
    free(whf);
    whf = NULL;
  }

  if (basec)
  {
    free(basec);
    basec = NULL;
  }

  return EXIT_SUCCESS;
}
