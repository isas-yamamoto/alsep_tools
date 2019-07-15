#include <stdint.h>
#include <string.h>
#include "error.h"
#include "define.h"
#include "wtn.h"

/*!
 * @brief WTNレコードをチェックしエラーを返す。
 *
 * @param[in] wnr WTNレコード構造体
 * @return エラーがない場合は0を返す。
 * エラーがある場合はエラーのマスク値を返す。
 */
int check_wtn_record(wtn_record wnr) {
  int ret = ERROR_NONE;
  
  if(wnr.id_normal != 3U) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid id normal: %d", wnr.id_normal);
    ret |= ERROR_INVALID_FORMAT;
  }
  
  if(wnr.num_asta<1U || wnr.num_asta>5U) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid number of active stations: %d", wnr.num_asta);
    ret |= ERROR_INVALID_FORMAT;
  }
  
  if( !(wnr.year == 1976U || wnr.year == 1977U) ) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid year: %d",wnr.year);
    ret |= ERROR_INVALID_DATETIME;
  }
  
  return ret;
}


/*!
 * @brief WTNフレームをチェックしエラーを返す。
 *
 * @param[in] wnf WTNフレーム構造体
 * @return 成功したときは0を返す。
 * 失敗したときはエラー番号マスク値を返す。
 */
int check_wtn_frame(wtn_frame wnf, int year) {
  int error_flag = 0;
  int64_t delta;
  
  if ( wnf.alsep_package_id < 1U || wnf.alsep_package_id > 5U ) {
    error_flag |= ERROR_INVALID_APOLLO_STATION;
  }
  
  if (!validate_date(package_id2station_id(wnf.alsep_package_id),
                     year, wnf.msec_of_year)) {
    error_flag |= ERROR_INVALID_DATETIME;
  }
  
  if ( wnf.frame_count >= SIZE_LOGICAL_RECORD ) {
    error_flag |= ERROR_INVALID_FORMAT;
  }
  
  if (wnf.sync_code != VALID_SYNC_CODE) {
    error_flag |= ERROR_INVALID_SYNC_CODE;
  }
  
  //! these judgment using 10, 100 below is not so meaningful.
  delta = wnf.time_diff - VALID_FRAME_RATE;
  delta = (delta < 0) ? -delta : delta;
  if (delta >10 && delta <= 100) {
    error_flag |= ERROR_FRAME_SMALL_TIME_ERROR;
  }
  if (delta >100) {
    error_flag |= ERROR_FRAME_LARGE_TIME_ERROR;
  }
  
  if (wnf.frame_count - wnf.prev_frame != 1U) {
    if (!(wnf.frame_count == 0U && wnf.prev_frame == 89U)) {
      error_flag |= ERROR_FRAME_COUNT_SEQUENCE;
    }
  }

  return error_flag;
}

/*!
 * @brief バイナリデータをWTNレコード構造体に展開する
 *
 * @param[in] record バイナリデータ
 * @return WTNレコード構造体を返す。
 */
wtn_record binary2wtn_record(const unsigned char *header) {
  wtn_record wnr;
  
  // Header Format ------------------------------
  // 1-2 ... 3 to identify Normal-Bit-Rate Work tape
  wnr.id_normal = header[0];
  wnr.id_normal = (wnr.id_normal << 8) + header[1];
  
  // 3-4 Active Station code
  wnr.active_station[0] = header[2] & 0x70U;
  wnr.active_station[0] = (wnr.active_station[0] >> 4);
  wnr.active_station[1] = header[2] & 0x0eU;
  wnr.active_station[1] = (wnr.active_station[1] >> 1);
  wnr.active_station[2] = header[2] & 0x01U;
  wnr.active_station[2] = (wnr.active_station[2] << 2) + (header[3] >> 6);
  wnr.active_station[3] = header[3] & 0x38U;
  wnr.active_station[3] = (wnr.active_station[3] >> 3);
  wnr.active_station[4] = header[3] & 0x07U;
  
  // 5-6 Number of active stations
  wnr.num_asta = header[4];
  wnr.num_asta = (wnr.num_asta << 8) + header[5];
  
  //7-8 Original 9 track ID
  wnr.original_id = header[6];
  wnr.original_id = (wnr.original_id << 8) + header[7];
  
  //9-10 year
  wnr.year = header[8];
  wnr.year = (wnr.year << 8) + header[9];
  
  //11-14 and first 4bit of byte 15;
  //Time of the year of the first data in msec
  wnr.first_msec = (int64_t)header[10];
  wnr.first_msec = (wnr.first_msec << 8) + (int64_t)header[11];
  wnr.first_msec = (wnr.first_msec << 8) + (int64_t)header[12];
  wnr.first_msec = (wnr.first_msec << 8) + (int64_t)header[13];
  wnr.first_msec = (wnr.first_msec << 4) + (int64_t)(header[14] >> 4);
  
  return wnr;
}

/*!
 * @brief バイナリデータをWTNフレーム構造体に展開する
 *
 * @param[in] wnr WTNレコード構造体
 * @param[in] frame バイナリデータ
 * @return WTNフレーム構造体を返す。
 */
wtn_frame binary2wtn_frame(wtn_record wnr, const unsigned char *frame) {
  int i, j;
  wtn_frame wnf;
  int32_t la[21], lb[21], lc[21];

  memset(&wnf, 0, sizeof(wnf));
  
  // Set header part
  wnf.flag_bit = frame[0] >> 7;
  
  wnf.msec_of_year = (int64_t)(frame[0] & 0x7fU);
  wnf.msec_of_year = (wnf.msec_of_year << 8) + (int64_t)frame[1];
  wnf.msec_of_year = (wnf.msec_of_year << 8) + (int64_t)frame[2];
  wnf.msec_of_year = (wnf.msec_of_year << 8) + (int64_t)frame[3];
  wnf.msec_of_year = (wnf.msec_of_year << 4) + (int64_t)(frame[4] >> 4);
  
  wnf.alsep_tracking_station_id = frame[4] & 0x0fU;
  
  wnf.alsep_package_id = frame[5] >> 5;
  wnf.bit_search = frame[5] & 0x0fU;
  wnf.bit_search = wnf.bit_search >> 4;
  wnf.bit_verify = frame[5] & 0x08U;
  wnf.bit_verify = wnf.bit_verify >> 3;
  wnf.bit_confirm = frame[5] & 0x04U;
  wnf.bit_confirm = wnf.bit_confirm >> 2;
  wnf.bit_loc = frame[5] & 0x02U;
  wnf.bit_loc = wnf.bit_loc >> 1;
  wnf.bit_il = frame[5] & 0x01U;

  wnf.original_rec_num = frame[6];
  wnf.original_rec_num = (wnf.original_rec_num << 8) + frame[7];

  wnf.sync_code = frame[8];
  wnf.sync_code = (wnf.sync_code << 3) + (frame[9] >> 5);

  wnf.sync_code_comp = frame[9] & 0x0fU;
  wnf.sync_code_comp = (wnf.sync_code_comp << 7) +(frame[10] >> 1);

  wnf.frame_count = frame[11] >> 1;
  wnf.mode_bit = frame[11] & 0x01U;

  // Set data part
  for(i=0; i<=20; ++i){
    la[i] = (int32_t)frame[12+i*4];
    la[i] = (la[i] << 2) | (frame[12+i*4+1] >> 6);

    lb[i] = (int32_t)(frame[12+i*4+1] & 0x1fU);
    lb[i] = (lb[i] << 5) | (( frame[12+i*4+2] >> 3) & 0x1f);

    lc[i] = (int32_t)(frame[12+i*4+2] & 0x03U);
    lc[i] = (lc[i] << 8) | (int32_t)frame[12+i*4+3];
  }

  //Get Passive Seismic Experiment Data (+ HK & Command Verify)
  if (wnf.alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_17) {
    j=0;
    for(i=0; i<20; i=i+2){
      wnf.spz[j+1] = la[i];
      wnf.spz[j+2] = lc[i];
      wnf.spz[j+3] = lb[i+1];
      j=j+3;
    }
    wnf.spz[31] = la[20];

    if (wnf.alsep_package_id == ALSEP_PACKAGE_ID_APOLLO_15) {
      wnf.spz[11] = interp(wnf.spz[9], wnf.spz[10], wnf.spz[12], wnf.spz[13]);
    }
    if (wnf.alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_14){
      wnf.spz[22]=interp(wnf.spz[20], wnf.spz[21], wnf.spz[23], wnf.spz[24]);
    }
    wnf.spz[27]=interp(wnf.spz[25], wnf.spz[26], wnf.spz[28], wnf.spz[29]);

    wnf.lpx[0] = lc[1];
    wnf.lpy[0] = lb[2];
    wnf.lpz[0] = la[3];
    wnf.lpx[1] = la[7];
    wnf.lpy[1] = lc[7];
    wnf.lpz[1] = lb[8];
    wnf.lpx[2] = lb[12];
    wnf.lpy[2] = la[13];
    wnf.lpz[2] = lc[13];
    wnf.lpx[3] = lc[17];
    wnf.lpy[3] = lb[18];
    wnf.lpz[3] = la[19];

    if (wnf.frame_count%2U == 0U) {
      wnf.TidX = lb[10];
      wnf.TidY = la[11];
    } else {
      wnf.TidZ = lb[10];
      wnf.InstT = la[11];
    }

    switch(wnf.alsep_package_id) {
    case ALSEP_PACKAGE_ID_APOLLO_12:
    case ALSEP_PACKAGE_ID_APOLLO_15:
    case ALSEP_PACKAGE_ID_APOLLO_16:
      wnf.lsm_stat = lb[0];
      wnf.lsm[0] = lb[4];
      wnf.lsm[1] = la[5];
      wnf.lsm[2] = lc[5];
      wnf.lsm[3] = la[15];
      wnf.lsm[4] = lc[15];
      wnf.lsm[5] = lb[16];
      break;
    default:
      for(i=0; i<COUNTS_PER_FRAME_FOR_WTN_LSM; i++) {
        wnf.lsm[i] = DATA_NONE;
      }
      break;
    }

    wnf.hk = lc[9];

    if (wnf.alsep_package_id != ALSEP_PACKAGE_ID_APOLLO_14) {
      wnf.cv = la[14] >> 1;
    } else {
      wnf.cv = lb[0] >> 1;
    }
  } else {
    j=0;
    for(i=0; i<20; i=i+2){
      wnf.lsg[j] = 511 - la[i];
      wnf.lsg[j+1] = 511 - lc[i];
      wnf.lsg[j+2] = 511 - lb[i+1];
      j=j+3;
    }
    wnf.lsg[30] = 511 - la[20];
    
    wnf.lsg_tide = 511 - la[7];
    wnf.lsg_free = 511 - lc[7];
    wnf.lsg_temp = 511 - lb[8];
  }
  
  return wnf;
}

/*!
 * @brief パッケージIDからアポロ号数を取得する
 *
 * @param[in] package_id パッケージID
 * @return アポロ号数
 */
int package_id2station_id(uint32_t package_id) {
  int apollo_station = -1;

  switch (package_id) {
  case ALSEP_PACKAGE_ID_APOLLO_14:
    apollo_station = 14;
    break;
    
  case ALSEP_PACKAGE_ID_APOLLO_15:
    apollo_station = 15;
    break;
    
  case ALSEP_PACKAGE_ID_APOLLO_17:
    apollo_station = 17;
    break;
    
  case ALSEP_PACKAGE_ID_APOLLO_12:
    apollo_station = 12;
    break;
    
  case ALSEP_PACKAGE_ID_APOLLO_16:
    apollo_station = 16;
    break;

  default:
    break;
  }
  
  return apollo_station;
}
