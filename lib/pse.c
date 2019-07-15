#include <stdint.h>
#include <string.h>
#include "error.h"
#include "define.h"
#include "pse.h"

/*!
 * @brief PSEレコードをチェックしエラーを返す。
 *
 * @param[in] pr PSEレコード構造体
 * @return エラーがない場合は0を返す。
 * エラーがある場合はエラーのマスク値を返す。
 */
int check_pse_record(pse_record pr) {
  int ret = ERROR_NONE;
  
  //! See. University of Texas Institute for Geophysics Technical Report No. 118
  //! page 7, TAPE FORMAT DESCRIPTION
  if(pr.tape_type != 1U && pr.tape_type != 2U) {
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid tape type:%d", pr.tape_type);
    ret |= ERROR_INVALID_FORMAT;
  }
  
  switch(pr.apollo_station) {
  case 11U:
  case 12U:
  case 14U:
  case 15U:
  case 16U:
    break;
  default:
    ret |= ERROR_INVALID_APOLLO_STATION;
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid apollo station number: %d", pr.apollo_station);
    break;
  }
  
  if( !(pr.year >= 1969U && pr.year <= 1977U)) {
    ret |= ERROR_INVALID_DATETIME;
    log_printf(LOG_WARNING, __FILE__, __LINE__,
	       "invalid year: %d", pr.year);
  }
  
  if (pr.format == FORMAT_OLD) {
    if( !(pr.phys_records >= 1U && pr.phys_records <= 3U) ) {
      log_printf(LOG_WARNING, __FILE__, __LINE__,
		 "invalid physical records: %d", pr.phys_records);
      ret |= ERROR_INVALID_FORMAT;
    }
  } else {
    if( !(pr.phys_records >= 1U && pr.phys_records <= 6U) ) {
      log_printf(LOG_WARNING, __FILE__, __LINE__,
		 "invalid physical records: %d", pr.phys_records);
      ret |= ERROR_INVALID_FORMAT;
    }
  }
  
  return ret;
}

/*!
 * @brief PSEフレームをチェックしエラーを返す。
 *
 * @param[in] pf PSEフレーム構造体
 * @return 成功したときは0を返す。
 * 失敗したときはエラー番号マスク値を返す。
 */
int check_pse_frame(pse_frame pf, int apollo_station, int year) {
  int error_flag = 0;
  int64_t delta;

  if (!validate_date(apollo_station, year, pf.msec_of_year)) {
    error_flag |= ERROR_INVALID_DATETIME;
  }
  
  //! HK must be equal to and less than 255 due to 8 effective bits.
  //! see 'APOLLO LUNAR SURFACE EXPERIMENT PACKAGE
  //! ARCHIVE TAPE DESCRIPTION DOCUMENT (JSC-09652)'
  //! 1.4.1.2.3 ALSEP Word 33 - Housekeeping
  if ( pf.hk > 255 ) {
    error_flag |= ERROR_INVALID_VALUE;
  }
  
  //! these judgment using 10, 100 below is not so meaningful.
  delta = pf.time_diff - VALID_FRAME_RATE;
  delta = (delta < 0) ? -delta : delta;
  if (delta >10 && delta <= 100) {
    error_flag |= ERROR_FRAME_SMALL_TIME_ERROR;
  }
  if (delta >100) {
    error_flag |= ERROR_FRAME_LARGE_TIME_ERROR;
  }
  
  if (pf.sync_code != VALID_SYNC_CODE) {
    error_flag |= ERROR_INVALID_SYNC_CODE;
  }

  if (pf.frame_count - pf.prev_frame != 1U) {
    if (!(pf.frame_count == 0U && pf.prev_frame == 89U)) {
      error_flag |= ERROR_FRAME_COUNT_SEQUENCE;
    }
  }
  
  return error_flag;
}

/*!
 * @brief バイナリデータをPSEレコード構造体に展開する
 *
 * @param[in] record バイナリデータ
 * @return PSEレコード構造体を返す。
 */
pse_record binary2pse_record(const unsigned char *record) {
  pse_record pr;
  
  // Header Format ------------------------------
  // 1-2 ... 1 for PSE tapes; 2 for Event tapes
  pr.tape_type = record[0];
  pr.tape_type = (pr.tape_type << 8) + record[1];
  
  // 3-4 Apollo station number
  pr.apollo_station = record[2];
  pr.apollo_station = (pr.apollo_station << 8) + record[3];
  
  // 5-6 original tape sequence number for PSE tapes;
  //     2-digit station code plus 3-digit-original event tape
  //     sequence number for Event tapes
  pr.tape_seq = record[4];
  pr.tape_seq = (pr.tape_seq << 8) + record[5];
  
  //7-8 record number
  pr.record_number = record[6];
  pr.record_number = (pr.record_number << 8) + record[7];
  
  //9-10 year
  pr.year = record[8];
  pr.year = (pr.year << 8) + record[9];
  
  //11-12 format (0=old, 1=new)
  pr.format = record[10];
  pr.format = (pr.format << 8) + record[11];
  
  //13-14 number of physical records from original tape
  pr.phys_records = record[12];
  pr.phys_records = (pr.phys_records << 8) + record[13];
  
  //15-16 original tape read error flags
  pr.read_err = record[14];
  pr.read_err = (pr.read_err << 8) + record[15];
  
  return pr;
}

/*!
 * @brief バイナリデータをPSEフレーム構造体に展開する
 *
 * @param[in] pr PSEレコード構造体
 * @param[in] frame バイナリデータ
 * @return PSEフレーム構造体を返す。
 */
pse_frame binary2pse_frame(pse_record pr, const unsigned char *frame) {
  int i;
  pse_frame pf;
  int32_t la[15], lb[15], lc[15];
  
  memset(&pf, 0, sizeof(pf));
  pf.software_time_flag= frame[0] >> 7;
  
  pf.msec_of_year = (int64_t)(frame[0] & 0x7fU);
  pf.msec_of_year = (pf.msec_of_year << 8) + (int64_t)frame[1];
  pf.msec_of_year = (pf.msec_of_year << 8) + (int64_t)frame[2];
  pf.msec_of_year = (pf.msec_of_year << 8) + (int64_t)frame[3];
  pf.msec_of_year = (pf.msec_of_year << 4) + (int64_t)(frame[4] >> 4);
  
  pf.alsep_tracking_station_id = frame[4] & 0x0fU;
  pf.bit_error_rate = (frame[5] >> 2) & 0x3fU;
  pf.data_rate = (frame[5] >> 1) & 0x01U;
  
  pf.alsep_word5 = frame[6] & 0x03U;
  pf.alsep_word5 = (pf.alsep_word5 << 8) | frame[7];
  
  pf.sync_code = frame[8];
  pf.sync_code = (pf.sync_code << 3) + (frame[9] >> 5);
  
  pf.sync_code_comp = frame[9] & 0x0fU;
  pf.sync_code_comp = (pf.sync_code_comp << 7) +(frame[10] >> 1);
  
  pf.frame_count = frame[11] >> 1;
  
  pf.mode_bit = frame[11] & 0x01U;
  
  // set data part
  for(i=0; i<15; ++i) {
    la[i] = (int32_t)frame[12+i*4];
    la[i] = (la[i] << 2) | (frame[12+i*4+1] >> 6);
    
    lb[i] = (int32_t)(frame[12+i*4+1] & 0x1fU);
    lb[i] = (lb[i] << 5) | ((frame[12+i*4+2] >> 3) & 0x1fU);
    
    lc[i] = (int32_t)(frame[12+i*4+2] & 0x03U);
    lc[i] = (lc[i] << 8) | (int32_t)frame[12+i*4+3];
  }
  
  pf.spz[ 1] = la[ 0];
  pf.spz[ 2] = lb[ 0];
  pf.spz[ 3] = lc[ 0];
  pf.spz[ 4] = lb[ 1];
  pf.spz[ 5] = la[ 2];
  pf.spz[ 6] = lc[ 2];
  pf.spz[ 7] = la[ 3];
  pf.spz[ 8] = lb[ 3];
  pf.spz[ 9] = lc[ 3];
  pf.spz[10] = la[ 4];

  pf.spz[12] = la[ 5];
  pf.spz[13] = lc[ 5];
  pf.spz[14] = lb[ 6];
  pf.spz[15] = lc[ 6];
  pf.spz[16] = lb[ 7];
  pf.spz[17] = la[ 8];
  pf.spz[18] = lc[ 8];
  pf.spz[19] = la[ 9];
  pf.spz[20] = lc[ 9];
  pf.spz[21] = lb[10];

  pf.spz[23] = lb[11];
  pf.spz[24] = lc[11];
  pf.spz[25] = la[12];
  pf.spz[26] = lb[12];

  pf.spz[28] = la[13];
  pf.spz[29] = lc[13];
  pf.spz[30] = lb[14];
  pf.spz[31] = lc[14];

  if (pr.apollo_station == ALSEP_PSE_APOLLO_STATION_15) {
    pf.spz[11] = interp(pf.spz[9],pf.spz[10],pf.spz[12],pf.spz[13]);
  } else {
    pf.spz[11] = lb[ 4];
  }
  
  if (pr.apollo_station != ALSEP_PSE_APOLLO_STATION_14){
    pf.spz[22]=interp(pf.spz[20],pf.spz[21],pf.spz[23],pf.spz[24]);
  } else {
    pf.spz[22] = la[11];
  }
  
  pf.spz[27]=interp(pf.spz[25],pf.spz[26],pf.spz[28],pf.spz[29]);
  
  switch(pr.format) {
    
  case FORMAT_OLD:
    pf.lpx[0] = la[1];
    pf.lpy[0] = lc[1];
    pf.lpz[0] = lb[2];
    
    pf.lpx[1] = lc[4];
    pf.lpy[1] = lb[5];
    pf.lpz[1] = la[6];
    
    pf.lpx[2] = lb[9];
    pf.lpy[2] = la[10];
    pf.lpz[2] = lc[10];
    
    pf.lpx[3] = lc[12];
    pf.lpy[3] = lb[13];
    pf.lpz[3] = la[14];
    
    if (pf.frame_count%2U == 0U) {
      pf.TidX  = lc[7];
      pf.TidY  = lb[8];
      pf.TidZ  = DATA_NONE;
      pf.InstT = DATA_NONE;
    } else {
      pf.TidX  = DATA_NONE;
      pf.TidY  = DATA_NONE;
      pf.TidZ  = lc[7];
      pf.InstT = lb[8];
    }

    pf.hk = la[7];
    if (pr.apollo_station != ALSEP_PSE_APOLLO_STATION_14) {
      pf.cv = la[11] >> 1;
    } else {
      pf.cv = pf.alsep_word5 >> 1;
    }

    break;

  case FORMAT_NEW:
    pf.lpx[0] = la[0];
    pf.lpy[0] = lb[0];
    pf.lpz[0] = lc[0];

    pf.lpx[1] = la[1];
    pf.lpy[1] = lb[1];
    pf.lpz[1] = lc[1];	

    pf.lpx[2] = la[3];
    pf.lpy[2] = lb[3];
    pf.lpz[2] = lc[3];

    pf.lpx[3] = lb[4];
    pf.lpy[3] = lc[4];
    pf.lpz[3] = la[5];

    if (pf.frame_count%2U == 0U) {
      pf.TidX = lb[2];
      pf.TidY = lc[2];
    } else {
      pf.TidZ = lb[2];
      pf.InstT = lc[2];
    }

    pf.hk = la[2];
    pf.cv = la[4] >> 1;
    break;

  default:
    break;
  }

  return pf;
}
