#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "define.h"
#include "util.h"
#include "wth.h"

int validate_lspe_date(int year, uint64_t msec);

/*!
 * @brief WTHレコードをチェックしエラーを返す。
 *
 * @param[in] whr WTHレコード構造体
 * @return エラーがない場合は0を返す。
 * エラーがある場合はエラーのマスク値を返す。
 */
int check_wth_record(wth_record whr) {
  int ret = ERROR_NONE;
  
  if(whr.id != 4U) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid id normal: %d", whr.id);
    ret |= ERROR_INVALID_FORMAT;
  }
  
  if(whr.num_asta != 1U) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid number of active stations: %d", whr.num_asta);
    ret |= ERROR_INVALID_FORMAT;
  }
  
  if( !(whr.year == 1976U || whr.year == 1977U) ) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid year: %d",whr.year);
    ret |= ERROR_INVALID_DATETIME;
  }
  
  return ret;
}

/*!
 * @brief WTHフレームをチェックしエラーを返す。
 *
 * @param[in] whf WTHフレーム構造体
 * @return 成功したときは0を返す。
 * 失敗したときはエラー番号マスク値を返す。
 */
int check_wth_frame(wth_frame whf, int year) {
  int error_flag = 0;
  int64_t delta;
  
  if ( whf.msec_of_year == 0 ) {
    error_flag |= ERROR_INVALID_DATETIME;
  }

  if (!validate_lspe_date(year, whf.msec_of_year)) {
  }
  
  if ( whf.alsep_package_id != 5U ) {
    error_flag |= ERROR_INVALID_APOLLO_STATION;
  }
  
  if (whf.sync_code != VALID_SYNC_CODE_WTH) {
    error_flag |= ERROR_INVALID_SYNC_CODE;
  }
  
  //! these judgement using 10, 100 below is not so meaningful.
  delta = whf.time_diff - VALID_FRAME_RATE_WTH;
  delta = (delta < 0) ? -delta : delta;
  if (delta >10 && delta <= 100) {
    error_flag |= ERROR_FRAME_SMALL_TIME_ERROR;
  }
  if (delta >100) {
    error_flag |= ERROR_FRAME_LARGE_TIME_ERROR;
  }
  
  return error_flag;
}

/*!
 * @brief バイナリデータをWTHフレーム構造体に展開する
 *
 * @param[in] wnr WTHレコード構造体
 * @param[in] frame バイナリデータ
 * @return WTHフレーム構造体を返す。
 */
wth_record binary2wth_record(const unsigned char *header) {
  wth_record whr;
  
  // Header Format ------------------------------
  // 1-2 ... 3 to identify Normal-Bit-Rate Work tape
  whr.id = header[0];
  whr.id = (whr.id << 8) + header[1];
  
  // 3-4 Active Station code
  whr.active_station[0] = header[3] & 0x07U;
  whr.active_station[1] = 0U;
  whr.active_station[2] = 0U;
  whr.active_station[3] = 0U;
  whr.active_station[4] = 0U;
  
  // 5-6 Number of active stations
  whr.num_asta = header[4];
  whr.num_asta = (whr.num_asta << 8) + header[5];
  
  //7-8 Original 9 track ID
  whr.original_id = header[6];
  whr.original_id = (whr.original_id << 8) + header[7];
  
  //9-10 year
  whr.year = header[8];
  whr.year = (whr.year << 8) + header[9];
  
  //11-14 and first 4bit of byte 15;
  //Time of the year of the first data in msec
  whr.first_msec = header[10];
  whr.first_msec = (whr.first_msec << 8) + header[11];
  whr.first_msec = (whr.first_msec << 8) + header[12];
  whr.first_msec = (whr.first_msec << 8) + header[13];
  whr.first_msec = (whr.first_msec << 4) + (header[14] >> 4);
  
  return whr;
}

/*!
 * @brief バイナリデータをWTHフレーム構造体に展開する
 *
 * @param[in] wnr WTHレコード構造体
 * @param[in] frame バイナリデータ
 * @return WTHフレーム構造体を返す。
 */
wth_frame binary2wth_frame(wth_record whr, const unsigned char *frame) {
  int i, j;
  int part1, part2, part3, part4;
  wth_frame whf;
  int sub_frame_array[] = {-1, 2, 3, 1};
  
  memset(&whf, 0, sizeof(whf));
  
  // Set header part
  whf.flag_bit = frame[0] >> 7;
  
  whf.msec_of_year = frame[0] & 0x7f;
  whf.msec_of_year = (whf.msec_of_year << 8) + frame[1];
  whf.msec_of_year = (whf.msec_of_year << 8) + frame[2];
  whf.msec_of_year = (whf.msec_of_year << 8) + frame[3];
  whf.msec_of_year = (whf.msec_of_year << 4) + (frame[4] >> 4);
  
  whf.alsep_tracking_station_id = frame[4] & 0x0f;
  
  whf.alsep_package_id = frame[5] >> 5;
  whf.bit_search = frame[5] & 0x0f;
  whf.bit_search = whf.bit_search >> 4;
  whf.bit_verify = frame[5] & 0x08;
  whf.bit_verify = whf.bit_verify >> 3;
  whf.bit_confirm = frame[5] & 0x04;
  whf.bit_confirm = whf.bit_confirm >> 2;
  whf.bit_loc = frame[5] & 0x02;
  whf.bit_loc = whf.bit_loc >> 1;
  whf.bit_il = frame[5] & 0x01;

  whf.original_rec_num = frame[6];
  whf.original_rec_num = (whf.original_rec_num << 8) + frame[7];

  whf.sync_code = frame[8];
  whf.sync_code = (whf.sync_code << 2) + (frame[9] >> 6);

  whf.status[0] = -1;
  whf.dp1[0] = (frame[9] >> 1) & 0x1f;
  whf.dp1[0] <<= 3;

  whf.dp6[0] = frame[9] & 0x01;
  whf.dp6[0] = (whf.dp6[0] << 4) | ((frame[10] >> 4) & 0x0f);
  whf.dp6[0] <<= 3;

  whf.dp11[0] = frame[10] & 0x0f;
  whf.dp11[0] = (whf.dp11[0] << 1) | (frame[11] >> 7);
  whf.dp11[0] <<= 3;

  whf.dp16[0] = (frame[11] >> 2) & 0x1f;
  whf.dp16[0] <<= 3;

  for(i=1; i<20; ++i) {
    j = (i+2)*4;

    part1 = frame[j] >> 1;
    part1 <<= 1;

    part2 = frame[j] & 0x01;
    part2 = (part2 << 6) | (frame[j+1] >> 2);
    part2 <<= 1;

    part3 = frame[j+1] & 0x03;
    part3 = (part3 << 5) | (frame[j+2] >> 3);
    part3 <<= 1;

    part4 = frame[j+2] & 0x07;
    part4 = (part4 << 4) | (frame[j+3] >> 4);
    part4 <<= 1;
    
    whf.dp1[i] = part1;
    whf.dp6[i] = part2;
    whf.dp11[i] = part3;
    whf.dp16[i] = part4;

    whf.status[i] = (frame[j+3] >> 2) & 0x03;
  }
  whf.sub_frame = sub_frame_array[whf.status[19]];

  return whf;
}

int validate_lspe_date(int year, uint64_t msec) {
  uint32_t doy, hh, mm, ss, ms;
  
  msec_of_year_to_date(msec, &doy, &hh, &mm, &ss, &ms);

  if (!((year == 1976 && doy >= 228) ||
	(year == 1977 && doy <= 115))) {
    return FALSE;
  }

  return TRUE;
}
