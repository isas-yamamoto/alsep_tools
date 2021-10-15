/*! @file util.h
 *  @brief 共通的に利用する関数郡の宣言
 *  @author: Yukio Yamamoto,Ryuhei Yamada
 *  @date 2010/08/25
 */
#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

void msec_of_year_to_date(int64_t msec_of_year, uint32_t *doy, uint32_t* hh, uint32_t *mm, uint32_t *ss, uint32_t *ms );
int is_numeric(const char *s);
char* intary2str(int *data, size_t size, char *str, size_t maxstr);
ssize_t filesize(const char *filename);
int32_t interp(int32_t x1, int32_t x2, int32_t x3, int32_t x4);
int validate_date(int apollo_station, int year, uint64_t msec);
void doy_to_date_string(uint32_t year, uint32_t doy, char date_string[11]);
void msec_of_year_to_date_string(uint32_t year, int64_t msec_of_year, uint32_t us_offset, char date_string[24]);

#endif
