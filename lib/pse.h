/*! @file pse.h
 *  @brief PSE関連の共通項目を束ねたヘッダ
 *  @author: Yukio Yamamoto, Ryuhei Yamada
 *  @date 2011/06/15
 */
#ifndef __PSE_H__
#define __PSE_H__

// ------------------------------
// PSE data record size
// ------------------------------
// header_size: 16 octets
// frame_size:  72 octets
// record_size = header_size + frame_size x 270 frames

#define SIZE_PSE_HEADER 16
#define SIZE_RECORD 19456

#define FORMAT_OLD 0U
#define FORMAT_NEW 1U

#define COUNTS_PER_FRAME_FOR_PSE_SP 32
#define COUNTS_PER_FRAME_FOR_PSE_LP  4
#define SIZE_LOGICAL_RECORD 90
#define MAX_PSE_FRAME 540

#define SIZE_DATA_PART_OLD 72
#define SIZE_DATA_PART_NEW 36

#define ALSEP_PSE_APOLLO_STATION_11 11U
#define ALSEP_PSE_APOLLO_STATION_12 12U
#define ALSEP_PSE_APOLLO_STATION_14 14U
#define ALSEP_PSE_APOLLO_STATION_15 15U
#define ALSEP_PSE_APOLLO_STATION_16 16U

typedef struct tag_pse_record {
  
  //! Tape type (1 for PSE tapes, 2 for Event tapes)
  uint32_t tape_type;

  //! Apollo station number
  uint32_t apollo_station;

  //! original tape sequence number for PSE tapes;
  //! 2-digit station code plus 3-digit original
  uint32_t tape_seq;
  uint32_t record_number;
  uint32_t year;
  uint32_t format;
  uint32_t phys_records;
  uint32_t read_err;

  //! Error flag
  uint32_t error_flag;

} pse_record;

typedef struct tag_pse_frame {
  
  //! software time flag
  uint32_t software_time_flag;

  //! time of the year in msec
  int64_t msec_of_year;

  //! ALSEP tracking station ID
  uint32_t alsep_tracking_station_id;
  
  //! bit error rate
  uint32_t bit_error_rate;
  
  //! data rate (1=1060bps, 0=530bps)
  uint32_t data_rate;
  
  //! data rate (1=1060bps, 0=530bps)
  uint32_t alsep_word5;
  
  //! sync pattern Baker code
  uint32_t sync_code;
  
  //! sync pattern Baker code compliment
  uint32_t sync_code_comp;
  
  //! frame counter
  uint32_t frame_count;
  
  //! mode bit (1 for frame1=normal bit rate, 1 for frame2=slow bit rate)
  uint32_t mode_bit;
  
  //! Data of Short period seismometer Z-component
  int32_t spz[COUNTS_PER_FRAME_FOR_PSE_SP];
  
  //! Data of Long period seismometer X-component
  int32_t lpx[COUNTS_PER_FRAME_FOR_PSE_LP];
  
  //! Data of Long period seismometer Y-component
  int32_t lpy[COUNTS_PER_FRAME_FOR_PSE_LP];

  //! Data of Long period seismometer Z-component
  int32_t lpz[COUNTS_PER_FRAME_FOR_PSE_LP];
  
  //! Data of Tidal X-component
  int32_t TidX;
  
  //! Data of Tidal Y-component
  int32_t TidY;
  
  //! Data of Tidal Z-component
  int32_t TidZ;
  
  //! Instrument Temperature
  int32_t InstT;
  
  //! House Keeping
  int32_t hk;
  
  //! Command Verification
  int32_t cv;

  //! Frame time error in one record
  int64_t time_diff;

  //! Process flag
  uint32_t process_flag;

  //! Error flag
  uint32_t error_flag;

  //! frame counter
  uint32_t prev_frame;

} pse_frame;

int check_pse_record(pse_record pr);
int check_pse_frame(pse_frame pf, int apollo_station, int year);
pse_record binary2pse_record(const unsigned char *record);
pse_frame binary2pse_frame(pse_record pr, const unsigned char *frame);

#endif
