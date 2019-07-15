/*! @file error.c
 *  @brief エラー処理に関係する関数群
 *  @author: Yukio Yamamoto,Ryuhei Yamada
 *  @date 2010/08/25
 */
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "error.h"

#define SIZE_TIME_STRING 256

/*!
 * @brief エラー発生時にログを残す
 *
 * @param[in] type エラーの種類(INFO,WARNING,ERROR)
 * @param[in] filename 対象するファイル名
 * @param[in] line 行番号
 * @param[in] format 出力メッセージフォーマット(printf形式)
 * @param[in] ... 可変引数
 */
void log_printf(int type, const char* filename, int line, char *format, ...) {
  char timestr[SIZE_TIME_STRING];
  va_list ap;
  time_t t = time(NULL);
  struct tm* date = localtime(&t);

  strftime(timestr, (size_t)(SIZE_TIME_STRING-1), "%b %d %H:%M:%S", date);
  fprintf(stderr, "%s ", timestr);
  

  va_start(ap, format);
  
  switch(type) {
    case LOG_INFO:
      fprintf(stderr,"INFO: ");
      break;
    case LOG_WARNING:
      fprintf(stderr,"WARNING: ");
      break;
    case LOG_ERROR:
      fprintf(stderr,"ERROR: ");
      break;
    default:
      break;
  }
  
  fprintf(stderr,"%s[%d] ", filename, line);
  vfprintf(stderr, format, ap);
  fprintf(stderr,"\n");
}
