// minimp3 example player application for Win32
// this file is public domain -- do with it whatever you want!
//#define MAIN_PROGRAM

#include "libc.h"
#include "minimp3.h"


#include <iostream>
#include "haos_api.h"
#include "haos.h"
#include "bitripper_sim.h"

#define BUFFER_COUNT 1
#define MP3_FRAME_SIZE 1152
#define PCM16_SCALE (32768.0) // 2^15
#define MP3_INPUT_CHUNK_SIZE 768

static char bs_buffer[MP3_MAX_SAMPLES_PER_FRAME * BUFFER_COUNT];
static signed short sample_buffer[MP3_MAX_SAMPLES_PER_FRAME * BUFFER_COUNT];


// Definicija Ping-Pong bafera
// (alocirati kontinualni prostor za dva kanala)
static HAOS_PcmSample_t PingPongsample_buffer[MP3_FRAME_SIZE * 2 * 2];

// Definicija Ping-Pong Write pokazivača
// (inicijalizovati na početak svakog kanala u postkick delu)
static HAOS_PcmSample_t* PingPongSample_buffer_WrPtr[2];

// Definicija Ping-Pong Read pokazivača
// (inicijalizovati na iste početne adrese kao i Write pointeri)
static HAOS_PcmSample_t* PingPongSample_buffer_RdPtr[2];

// Definicija Ping-Pong End pokazivača
// (postaviti na kraj Ping-Pong bloka kako bi se
//  omogućilo kružno adresiranje tokom upisa/čitanja)
static HAOS_PcmSample_t* PingPongSample_buffer_EndPtr[2];

static int PCMAvailable = 0;

static mp3_info_t info;
static mp3_decoder_t mp3;

struct
{
	unsigned int mp3Enable;
	unsigned int srcActiveChannels[NUMBER_OF_IO_CHANNELS];
} mp3Decoder_mcv =
{
	//default values for mp3DecoderMCV
0x00000001,				// enable mp3 decoder
0,						// input wave channel 0,
NO_SOURCE,				// input wave channel 1,
1,				        // input wave channel 2,
NO_SOURCE,				// input wave channel 3,
NO_SOURCE,				// input wave channel 4,
NO_SOURCE,				// input wave channel 5,
NO_SOURCE,				// input wave channel 6,
NO_SOURCE,				// input wave channel 7,
NO_SOURCE,				// input wave channel 8,
NO_SOURCE,				// input wave channel 9,
NO_SOURCE,				// input wave channel 10,
NO_SOURCE,				// input wave channel 11,
NO_SOURCE,				// input wave channel 12,
NO_SOURCE,				// input wave channel 13,
NO_SOURCE,				// input wave channel 14,
NO_SOURCE,				// input wave channel 15
NO_SOURCE,				// input wave channel 16,
NO_SOURCE,				// input wave channel 17,
NO_SOURCE,				// input wave channel 18,
NO_SOURCE,				// input wave channel 19,
NO_SOURCE,				// input wave channel 20,
NO_SOURCE,				// input wave channel 21,
NO_SOURCE,				// input wave channel 22,
NO_SOURCE,				// input wave channel 23,
NO_SOURCE,				// input wave channel 24,
NO_SOURCE,				// input wave channel 25,
NO_SOURCE,				// input wave channel 26,
NO_SOURCE,				// input wave channel 27,
NO_SOURCE,				// input wave channel 28,
NO_SOURCE,				// input wave channel 29,
NO_SOURCE,				// input wave channel 30,
NO_SOURCE,				// input wave channel 31,
};



HAOS_Mct_t mp3Decoder_mct =
{
	mp3Decoder_prekickFunction, // Pre-kick
	mp3Decoder_postkickFunction,// Post-kick
	0,	// Timer
	0,	// Frame
	0, // Brick
	mp3Decoder_AFAPFunction,	// AFAP
	mp3Decoder_BackgroundFunction,	// Background
	0,	// Pre-malloc
	0	// Post-malloc 
};

HAOS_Mif_t mp3Decoder_mif = { &mp3Decoder_mcv, &mp3Decoder_mct };

HAOS_Odt_t mp3Decoder_odt =
{
	{&mp3Decoder_mif, 0x10},
	{0, 0}
};

HAOS_OdtEntry_t* mp3Decoder_odtPtr = mp3Decoder_odt;

// Definicija frameData strukture
static HAOS_FrameData_t mp3Decoder_frameData;

// definicija HAOS_CopyToIOPtrs_t strukture za MP3 dekoder
static HAOS_CopyToIOPtrs_t mp3Decoder_copyToIOPtrs;


void __fg_call mp3Decoder_prekickFunction(void* PcmDecoder_mifPtr)
{
	std::cout << "start prekick MP3" << std::endl;

	std::cout << "end prekick MP3" << std::endl;
}

void __fg_call mp3Decoder_postkickFunction()
{
	std::cout << "start postkick MP3" << std::endl;



	for (int ch = 0; ch < NUMBER_OF_IO_CHANNELS; ch++)
	{
		if (mp3Decoder_mcv.srcActiveChannels[ch] != NO_SOURCE)
		{
			// inicijalizacija kanalne maske prema aktivnim kanalima
			mp3Decoder_frameData.inputChannelMask |= (1 << ch);
			mp3Decoder_frameData.outputChannelMask |= (1 << ch);
		}
	}

	// Inicijalizovati frameData i postaviti odgovarajući pokazivač na nju
	mp3Decoder_frameData.sampleRate = 48000;
	mp3Decoder_frameData.decodeInfo = DECODE_INFO_MP3;
	mp3Decoder_copyToIOPtrs.frameData = &mp3Decoder_frameData;

	// Inicijalizovati PingPong pokazivače
	PingPongSample_buffer_WrPtr[0] = PingPongsample_buffer;
	PingPongSample_buffer_WrPtr[1] = PingPongsample_buffer + MP3_FRAME_SIZE * 2;
	PingPongSample_buffer_RdPtr[0] = PingPongsample_buffer;
	PingPongSample_buffer_RdPtr[1] = PingPongsample_buffer + MP3_FRAME_SIZE * 2;
	PingPongSample_buffer_EndPtr[0] = PingPongsample_buffer + MP3_FRAME_SIZE * 2;
	PingPongSample_buffer_EndPtr[1] = PingPongsample_buffer + MP3_FRAME_SIZE * 2 * 2;

	// pozvati inicijalizaciju mp3 dekodera	
	mp3 = mp3_create();

	std::cout << "end postkick MP3" << std::endl;
}

void __bg_call mp3Decoder_BackgroundFunction()
{
	// proveriti da li je mp3 dekoder omogućen, ako nije, ne radi ništa
	if (mp3Decoder_mcv.mp3Enable)
	{
		// pročitati kompresovane podatke iz FIFO+a preko BitRipper-a
		for (int sample = 0; sample < MP3_INPUT_CHUNK_SIZE / 4;)
		{
			int word = BitRipper::extractBits(32);
			bs_buffer[sample * 4 + 0] = (word >> 0) & 0xFF;
			bs_buffer[sample * 4 + 1] = (word >> 8) & 0xFF;
			bs_buffer[sample * 4 + 2] = (word >> 16) & 0xFF;
			bs_buffer[sample * 4 + 3] = (word >> 24) & 0xFF;
			sample++;
		}
		// pozvati dekodovanje mp3 frejma
		int byte_count = mp3_decode(mp3, bs_buffer, MP3_FRAME_SIZE * BUFFER_COUNT * 2, sample_buffer, &info);
		if (byte_count != 0)
			PCMAvailable = MP3_FRAME_SIZE;

		// kopirati interlivovane uzorke u Ping-Pong bafer (po kanalima) u odgovarajucem formatu
		for (int ch = 0; ch < 2; ch++)
		{
			int i = 0;
			HAOS_PcmSample_t* wr = PingPongSample_buffer_WrPtr[ch];
			for (int sample = ch; sample < MP3_FRAME_SIZE * 2 - 1; sample += 2)
			{
				wr[i++] = (HAOS_PcmSample_t)sample_buffer[sample] / PCM16_SCALE;
			}

			// azurirati pokazivače i vratiti ih na početak ako je bafer popunjen
			PingPongSample_buffer_WrPtr[ch] += MP3_FRAME_SIZE;

			if (PingPongSample_buffer_WrPtr[ch] >= PingPongSample_buffer_EndPtr[ch])
			{
				PingPongSample_buffer_WrPtr[ch] = PingPongsample_buffer + (MP3_FRAME_SIZE * 2 * ch);
			}
		}

	}

}


// ------------------------------------------------
// AFAP – slanje PCM-a u IO
// ------------------------------------------------
void __fg_call mp3Decoder_AFAPFunction()
{
	// proveriti da li ima dovoljno dostupnih PCM-ova za kopiranje
	if (PCMAvailable >= BRICK_SIZE)
	{

		int inputChannelMask = mp3Decoder_frameData.inputChannelMask;
		int j = 0;
		int ch = 0;

		{
			// priprema IO pokazivače po kanalima
			mp3Decoder_frameData.outputChannelMask = mp3Decoder_frameData.inputChannelMask;;
			while (inputChannelMask != 0)
			{
				if (inputChannelMask & 1)
				{
					// postaviti odgovarajuće IOBufferPtrs za kopiranje iz PingPong-a u IO bafere
					mp3Decoder_copyToIOPtrs.IOBufferPtrs[j] = PingPongSample_buffer_RdPtr[ch];
					// ažurirati PingPong pokazivače
					PingPongSample_buffer_RdPtr[ch] += BRICK_SIZE;
					ch++;
				}
				j++;
				inputChannelMask = inputChannelMask >> 1;
			}
			//vratiti pokazivače na pocetak ako su došli do kraja bafera
			if (PingPongSample_buffer_RdPtr[0] >= PingPongSample_buffer_EndPtr[0])
			{
				PingPongSample_buffer_RdPtr[0] = PingPongsample_buffer;
			}

			if (PingPongSample_buffer_RdPtr[1] >= PingPongSample_buffer_EndPtr[1])
			{
				PingPongSample_buffer_RdPtr[1] = PingPongsample_buffer + MP3_FRAME_SIZE * 2;
			}
		}

		// kopirati BRICK u IO
		HAOS::copyBrickToIO(&mp3Decoder_copyToIOPtrs);

		// ažurirati validnu kanalnu masku
		HAOS::setValidChannelMask(mp3Decoder_frameData.outputChannelMask);

		// smanjiti broj dostupnih PCM odbiraka
		PCMAvailable -= BRICK_SIZE;
	}

}