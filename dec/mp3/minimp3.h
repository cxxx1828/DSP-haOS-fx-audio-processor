#ifndef __MINIMP3_H_INCLUDED__
#define __MINIMP3_H_INCLUDED__
#include <stdint.h>

#define MP3_MAX_SAMPLES_PER_FRAME (1152*2)

typedef struct _mp3_info {
    int sample_rate;
    int channels;
    int audio_bytes;  // generated amount of audio per frame
} mp3_info_t;

typedef void* mp3_decoder_t;

extern mp3_decoder_t mp3_create(void);
extern int mp3_decode(mp3_decoder_t dec, void *buf, int bytes, signed short *out, mp3_info_t *info);
extern void mp3_done(mp3_decoder_t *dec);
#define mp3_free(dec) do { mp3_done(dec); dec = NULL; } while(0)


/////////////////////////////////////////////////////////
//
//  Author.......: Katarina Cvetkovic (katarinac)
//	Contact......: katarina.cvetkovic@rt-rk.com
//	Date.........: Apr 15, 2024
//
//	Declarations of MP3 decoder function for reading input wave file
/////////////////////////////////////////////////////////

#define __fg_call
#define __bg_call

void __fg_call mp3Decoder_prekickFunction(void* mp3Decoder_mifPtr);
void __fg_call mp3Decoder_postkickFunction();
void __fg_call mp3Decoder_brickFunction();
void __fg_call mp3Decoder_AFAPFunction();
void __bg_call mp3Decoder_BackgroundFunction();
void __fg_call mp3Decoder_premallocFunction();


#endif//__MINIMP3_H_INCLUDED__
