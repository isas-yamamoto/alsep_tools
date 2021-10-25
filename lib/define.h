#ifndef __DEFINE_H__
#define __DEFINE_H__

#define SIZE_SQL         1024
#define SIZE_FILEPATH    1024
#define DATA_NONE        9999
#define SIZE_TIME_STRING 27

#define VALID_SYNC_CODE 1810U
#define VALID_SYNC_CODE_WTH 59U

#define FLAG_FIRST_DATA_OF_FILE 0x0001
#define FLAG_TOP_OF_RECORD      0x0002
#define FLAG_FIRST_DATA_COPIED  0x0004

#define FRAME_COUNT_INIT -1

//! 64 word/frame, 1word=10bit, 1060bits/sec, (64*10/1060=603.77[msec])
#define VALID_FRAME_RATE 604

//! 20 word/subframe, 1word=30bit, 3533bits/sec, (20*30/3533=169.82[msec])
#define VALID_FRAME_RATE_WTH 170

#ifdef DEBUG
#define dbg_printf(s...) printf(s)
#else
#define dbg_printf(s...)
#endif

#define SET_ARG(var,n,size) {strncpy(var, argv[n], size); var[size] = '\0';}

#endif
