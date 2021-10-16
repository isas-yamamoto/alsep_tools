/*! @file
 *  @brief PSE,WT-N,WT-Hで共通的に利用する関数郡
 *  @author: Yukio Yamamoto,Ryuhei Yamada
 *  @date 2010/08/25
 *  @version 0.1
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

/*!
 * @brief 年単位の通算ミリ秒を年単位の通算日(DOY)と時刻に変換する
 *
 * @param[in] msec_of_year 年単位の通算ミリ秒
 * @param[out] doy 年単位の通算日
 * @param[out] hh 時 (0-23)
 * @param[out] mm 分 (0-59)
 * @param[out] ss 秒 (0-59)
 * @param[out] ms ミリ秒 (0-999)
 * @attention 入力値msec_of_yearの最小値は8640000[msec]、出力のdoyの最小値は1である
 */
void msec_of_year_to_date(int64_t msec_of_year,
                          uint32_t *doy, uint32_t* hh, uint32_t *mm, uint32_t *ss, uint32_t *ms ) {
  
  uint32_t doy_rem, hh_rem;
  
  int64_t sec_of_year = (int)(msec_of_year / 1000);
  
  *ms = (uint32_t)(msec_of_year % 1000);
  
  *doy    = (uint32_t)(sec_of_year / 86400);
  doy_rem = (uint32_t)(sec_of_year % 86400);
  
  *hh    = doy_rem / 3600U;
  hh_rem = doy_rem % 3600U;
  
  *mm = hh_rem / 60U;
  *ss = hh_rem % 60U;
}

/*!
 * @brief convert DOY to date
 *
 * @param[in] year year
 * @param[in] doy day of year
 * @param[out] date_string date_string
 */
int doy_to_date_string(uint32_t year, uint32_t doy, char date_string[11]) {
  uint32_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  uint32_t month, day;
  size_t i;
  if (year == 1972 || year == 1976) {
    days_in_month[1] = 29; // add leap day
  }

  month = 1;
  for (i = 0; i < 12; ++i) {
    if (doy <= days_in_month[i]) {
      break;
    }
    month += 1;
    doy -= days_in_month[i];
  }
  if (month > 12) {
    return FALSE;
  }

  day = doy;
  sprintf(date_string, "%04d-%02d-%02d", year, month, day);
  return TRUE;
}

/*!
 * @brief 年単位の通算ミリ秒を年単位の通算日(DOY)と時刻に変換する
 *
 * @param[in] year year
 * @param[in] msec_of_year milliseconds of year
 * @param[in] us_offset microseconds offset
 * @param[out] date_string date_string
 */
void msec_of_year_to_date_string(uint32_t year, int64_t msec_of_year, uint32_t us_offset, char date_string[24] ) {
  uint32_t doy, hh, mm, ss, ms, us;
  char date[11]; /* YYYY-mm-dd */
  msec_of_year_to_date(msec_of_year, &doy, &hh, &mm, &ss, &ms);
  doy_to_date_string(year, doy, date);
  us = ms + 1000;
  us += us_offset;

  sprintf(date_string, "%s %02d:%02d:%02d.%06d", date, hh, mm, ss, us);
}

/*!
 * @brief 文字列が数値かどうか確認する。
 *
 * @param[in] s 確認対象の文字列
 * @return 文字列が数値の場合1を返す。
 * 文字列が数値でない場合0を返す。
 */
int is_numeric(const char *s) {
  size_t i;
  int ret = 1;
  size_t len = strlen(s);
 
  if (s[0] != '-' && !(s[0] >= '0' && s[0] <= '9')) {
    ret = 0;
  } else {
    for(i=1U; i<len && ret==1; ++i) {
      if ( !(s[i] >= '0' && s[i] <= '9') ) {
        ret = 0;
      }
    }
  }
  return ret;
}

/*!
 * @brief int型配列からSQL99のARRAY形式文字列を作成する。
 *
 * @Param[in] data int型配列
 * @Param[in] size int型配列のサイズ
 * @Param[out] str  返却される文字列へのポインタ
 * @Param[in] maxstr 文字列型のサイズ
 * @return 作成したSQL99文字列を返す。
 * 失敗した場合はNULLを返す。
 */
char* intary2str(int *data, size_t size, char *str, size_t maxstr) {
  size_t i, len;
  char num[12]; // -2147483648
  str[0] = '\0';
  for(i=0U; i<size; ++i) {
    sprintf(num,"%d,", data[i]);
    if( strlen(num)+strlen(str) < maxstr ) {
      strcat(str,num);
    } else {
      return NULL;
    }
  }
  len = strlen(str);
  if (len > 0U) {
    str[len-1U] = '\0';
  }
  return str;
}

/*!
 * @brief ファイル名からファイルサイズを取得する
 *
 * @Param[in] filename ファイル名
 * @return ファイルサイズを返す。
 * 失敗した場合は負の値を返す。
 */
ssize_t filesize(const char *filename) {
  ssize_t fsize = -1;
  FILE* f = fopen(filename,"rb");
  if (f != NULL) {
    if (fseek(f,0,SEEK_END) != 0) {
      fsize = -2;
    } else {
      fsize = ftell(f);
    }
    fclose(f);
  }
  
  return fsize;
}

/*!
 * @brief 前後2点ずつ合計4点を利用してラグランジ法によりデータを補間する。
 *
 * @param[in] x1 補間対象点の2つ前の点
 * @param[in] x2 補間対象点の1つ前の点
 * @param[in] x4 補間対象点の1つ後の点
 * @param[in] x5 補間対象点の2つ後の点
 * @return 補間された値を返す
 */
int32_t interp(int32_t x1, int32_t x2, int32_t x3, int32_t x4) {
  return ((x2+x3)*4-(x1+x4)+3)/6;
}

/*!
 * @brief Validate date
 *
 * @param[in] year
 * @param[in] msec
 * @return True if the year and msec is valid
 */
int validate_date(int apollo_station, int year, uint64_t msec) {
  uint32_t doy, hh, mm, ss, ms;
  
  msec_of_year_to_date(msec, &doy, &hh, &mm, &ss, &ms);
  
  if (doy < 1) {
    return FALSE;
  }

  // check year and doy considering leap year
  switch(year) {
  case 1969:
  case 1970:
  case 1971:
  case 1973:
  case 1974:
  case 1975:
  case 1977:
    if (doy > 365) {
      return FALSE;
    }
    break;
    
  case 1972:
  case 1976:
    if (doy > 366) {
      return FALSE;
    }
    break;
    
  default:
    return FALSE;
    break;
  }
  

  // check year and doy considering mission period
  switch (apollo_station) {
  case 11:
    // 1969/202 - 1969/214, 1969/231 - 1969/237
    if (!(year == 1969 && ((doy >=202 && doy <= 214) || (doy >= 231 && doy <= 237)))) {
      return FALSE;
    }
    break;
    
  case 12: 
    // 1969/323-1977/273
    if (!((year == 1969 && doy >= 323) ||
	  (year >= 1970 && year <= 1976) ||
	  (year == 1977 && doy <= 273))) {
      return FALSE;
    }
    break;
    
  case 14:
    // 1971/036-1977/273
    if (!((year == 1971 && doy >= 36) ||
	  (year >= 1972 && year <= 1976) ||
	  (year == 1977 && doy <= 273))) {
      return FALSE;
    }
    break;
    
    // 1971/212-1977/273
  case 15:
    if (!((year == 1971 && doy >= 212) ||
	  (year >= 1972 && year <= 1976) ||
	  (year == 1977 && doy <= 273))) {
      return FALSE;
    }
    break;
    
    // 1972/112-1977/273
  case 16:
    if (!((year == 1972 && doy >= 112) ||
	  (year >= 1973 && year <= 1976) ||
	  (year == 1977 && doy <= 273))) {
      return FALSE;
    }
    break;

    // LSPE:1976/228-1977/115    
    // LSG: 1976/061-1977/273
  case 17:
    if (!((year == 1976 && doy >= 61) ||
	  (year == 1977 && doy <= 273))) {
      return FALSE;
    }
    break;
  }

  return TRUE;
}
