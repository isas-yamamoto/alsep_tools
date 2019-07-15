/*! @file wtn.h
 *   @brief Work Tape Normal Bit Rate
 *   @author: Yukio Yamamoto, Ryuhei Yamada
 *   @date: 2011/06/15
 */

#ifndef __ALSEP_WTN_H__
#define __ALSEP_WTN_H__

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

#define COUNTS_PER_FRAME_FOR_WTN_SP   32
#define COUNTS_PER_FRAME_FOR_WTN_LP    4
#define COUNTS_PER_FRAME_FOR_WTN_LSG  31
#define COUNTS_PER_FRAME_FOR_WTN_LSM   6
#define SIZE_LOGICAL_RECORD           90U

#define ALSEP_PACKAGE_ID_APOLLO_12 1U
#define ALSEP_PACKAGE_ID_APOLLO_15 2U
#define ALSEP_PACKAGE_ID_APOLLO_16 3U
#define ALSEP_PACKAGE_ID_APOLLO_14 4U
#define ALSEP_PACKAGE_ID_APOLLO_17 5U

typedef struct tag_wtn_record {
  
  //! 3 to identify Normal-Bit-Rate Work tape
  uint32_t id_normal;

  //! Active Station code
  uint32_t active_station[5];
  
  //! Number of active stations (1:site12 2:site15 3:site16 4:site14 5:site17)
  uint32_t num_asta;
  
  //! Original 9 track ID
  uint32_t original_id;
  
  //! Year
  uint32_t year;
  
  //! Time of the year of the first data in msec
  int64_t first_msec;

  //! Error flag
  uint32_t error_flag;
  
} wtn_record;

typedef struct tag_wtn_frame {

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

  //! frame counter
  uint32_t frame_count;

  //! mode bit (1 for frame1=normal bit rate, 1 for frame2=slow bit rate)
  uint32_t mode_bit;

  //! Data of Short period seismometer Z-component
  int32_t spz[COUNTS_PER_FRAME_FOR_WTN_SP];

  //! Data of Long period seismometer X-component
  int32_t lpx[COUNTS_PER_FRAME_FOR_WTN_LP];

  //! Data of Long period seismometer Y-component
  int32_t lpy[COUNTS_PER_FRAME_FOR_WTN_LP];

  //! Data of Long period seismometer Z-component
  int32_t lpz[COUNTS_PER_FRAME_FOR_WTN_LP];

  //! Data of Tidal X-component
  int32_t TidX;

  //! Data of Tidal Y-component
  int32_t TidY;

  //! Data of Tidal Z-component
  int32_t TidZ;

  //! Data of Seismic of Lunar Surface Gravimeter
  int32_t lsg[COUNTS_PER_FRAME_FOR_WTN_LSG];

  //! Data of Tide of Lunar Surface Gravimeter
  int32_t lsg_tide;
  
  //! Data of Free Mode of Lunar Surface Gravimeter
  int32_t lsg_free;

  //! Data of Sensor Temperature of Lunar Surface Gravimeter
  int32_t lsg_temp;

  //! Data of Lunar Surface Magnetometer
  int32_t lsm[COUNTS_PER_FRAME_FOR_WTN_LSM];

  //! LSM Engineering Status
  int32_t lsm_stat;

  //! Instrument Temperature
  int32_t InstT;

  //! House Keeping
  int32_t hk;

  //! Command Verification
  int32_t cv;

  //! Frame time difference in one record
  int64_t time_diff;

  //! Process flag
  uint32_t process_flag;
  
  //! Error flag
  uint32_t error_flag;
  
  //! frame counter
  uint32_t prev_frame;
  
} wtn_frame;

int check_wtn_record(wtn_record wnr);
int check_wtn_frame(wtn_frame wnf, int year);
wtn_record binary2wtn_record(const unsigned char *header);
wtn_frame binary2wtn_frame(wtn_record wnr, const unsigned char *frame);
int package_id2station_id(uint32_t package_id);

#endif
