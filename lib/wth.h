/*! @file wth.h
 *   @brief Work Tape High Bit Rate
 *   @author: Yukio Yamamoto, Ryuhei Yamada
 *   @date: 2011/06/15
 */

#ifndef __WTH_H__
#define __WTH_H__

// ------------------------------
// Work Tape Normal Bit Rate data record size
// ------------------------------
// header_size: 16 octets
// frame_size:  96 octets
//
// Data structure
// -----------------------------
// |    header (16 octets)     |
// |---------------------------|
// |    header (16 octets)     |
// |---------------------------|
// |    frame (96 octets)      |
// |                           |
// |---------------------------|
// |    frame (96 octets)      |
// |                           |
// |---------------------------|
// |    ...                    |
// -----------------------------
//

#define SIZE_HEADER 16
#define SIZE_FRAME  96

#define COUNTS_PER_FRAME_FOR_WTH_GP   20

#define ALSEP_PACKAGE_ID_APOLLO_17 5

typedef struct tag_wth_record {

  //! 4 to identify High-Bit-Rate Work tape
  uint32_t id;

  //! Active Station code 5 for Apollo 17 stations
  uint32_t active_station[5];

  //! Number of active stations (always 1)
  uint32_t num_asta;

  //! Original 9 track ID
  uint32_t original_id;

  //! Year
  uint32_t year;

  //! Time of the year of the first data in msec
  int64_t first_msec;

  //! Error flag
  uint32_t error_flag;

} wth_record;

typedef struct tag_wth_frame {

  //! a flag bit indicating usage of computer-generated time code
  uint32_t flag_bit;

  //! time of the year in msec
  int64_t msec_of_year;

  //! ALSEP tracking station ID
  uint32_t alsep_tracking_station_id;

  //! ALSEP package ID
  uint32_t alsep_package_id;

  //! bit synchronizer status (search)
  uint32_t bit_search;

  //!  bit synchronizer status (verify)
  uint32_t bit_verify;

  //! bit synchronizer status (confirm)
  uint32_t bit_confirm;

  //! bit synchronizer status (lock)
  uint32_t bit_loc;

  //! bit synchronizer status (input level)
  uint32_t bit_il;

  //! original 7-track record number (first subframe only)
  uint32_t original_rec_num;

  //! sync pattern Baker code
  uint32_t sync_code;

  //! sync pattern Baker code compliment
  uint32_t sync_code_comp;

  //! dp
  int32_t dp1[20], dp6[20], dp11[20], dp16[20];

  //! status
  int32_t status[20];

  //! sub frame
  uint32_t sub_frame;

  //! Frame time difference in one record
  int64_t time_diff;

  //! Process flag
  uint32_t process_flag;
  
  //! Error flag
  uint32_t error_flag;
  
  //! frame counter
  uint32_t prev_frame;

} wth_frame;

int check_wth_record(wth_record whr);
int check_wth_frame(wth_frame whf, int year);
wth_record binary2wth_record(const unsigned char *header);
wth_frame binary2wth_frame(wth_record whr, const unsigned char *frame);

#endif
