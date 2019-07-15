/*! @file error.h
 *  @brief エラー処理に関係する定義群および関数群
 *  @author: Yukio Yamamoto,Ryuhei Yamada
 *  @date 2010/08/25
 */
#ifndef __ERROR_H__
#define __ERROR_H__

#define ERROR_NONE                     0x0000
#define ERROR_INVALID_SYNC_CODE        0x4000
#define ERROR_ORIGINAL_DATA_HAS_ERROR  0x2000
#define ERROR_INVALID_DATETIME         0x1000
#define ERROR_INVALID_FRAME_COUNTER    0x0800
#define ERROR_INVALID_VALUE            0x0400
#define ERROR_INVALID_FORMAT           0x0200

#define ERROR_FRAME_COUNT_SEQUENCE     0x0010
#define ERROR_INVALID_APOLLO_STATION   0x0008
#define ERROR_INVALID_GROUND_STATION   0x0004
#define ERROR_FRAME_LARGE_TIME_ERROR   0x0002
#define ERROR_FRAME_SMALL_TIME_ERROR   0x0001

#define LOG_INFO    0
#define LOG_WARNING 1
#define LOG_ERROR   2

void log_printf(int type, const char* filename, int line, char *format, ...);

#endif
