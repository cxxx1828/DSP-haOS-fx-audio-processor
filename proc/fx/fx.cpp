#include "fx.h"
#include "haos_api.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>



// Use HaOS API for buffer access
#define BLOCK_SIZE BRICK_SIZE
#define sampleBuffer HAOS::getIOChannelPointerTable()
#define NUM_CHANNELS 6

// Get sample rate from HaOS
#define SAMPLE_RATE HAOS::getInputStreamFS()

static FX_ControlPanel moduleControl;

#define NTAPS 31 
#define DBUFSIZE 640
#define MAX_DELAY_SAMPLES 24000  // for 450ms @ 48kHz + margin

// Gain values in linear scale
static const double GAIN_CH0_PRE = 0.81283;    // -1.8 dB
static const double GAIN_CH0_POST = 0.87096;   // -1.2 dB
static const double GAIN_CH1_PRE = 0.79433;    // -2.0 dB
static const double GAIN_CH1_POST = 0.77426;   // -2.2 dB

// Filter coefficients for CH1-CH5 (for cutoff 2k, 3k, 4k, 5k)
static double lpf2kHz_coeffs[NTAPS] =
{
    -0.00293477270045858870,
    -0.00288096430204124940,
    -0.00250016237760415690,
    -0.00164589844498685530,
    -0.00013456384194181242,
    0.00225884205157718140,
    0.00580400571775908230,
    0.01081700734517632700,
    0.01765966375633795700,
    0.02673557015223622000,
    0.03848174902364159100,
    0.05335467351374602300,
    0.07180932214316887400,
    0.09426986305243344200,
    0.12109058458285790000,
    0.13563016065619612000,
    0.12109058458285790000,
    0.09426986305243344200,
    0.07180932214316887400,
    0.05335467351374602300,
    0.03848174902364159100,
    0.02673557015223622000,
    0.01765966375633795700,
    0.01081700734517632700,
    0.00580400571775908230,
    0.00225884205157718140,
    -0.00013456384194181242,
    -0.00164589844498685530,
    -0.00250016237760415690,
    -0.00288096430204124940,
    -0.00293477270045858870
};

static double lpf3kHz_coeffs[NTAPS] =
{
    -0.00105106340607596220,
    -0.00170346888080965400,
    -0.00250624192866901340,
    -0.00339431573437104200,
    -0.00422212099590341550,
    -0.00472444069729934920,
    -0.00446861323138686780,
    -0.00280012215560185900,
    0.00121378071889749130,
    0.00883416031353021630,
    0.02168127068667904300,
    0.04172769649713474500,
    0.07122907943774888000,
    0.11256174105687924000,
    0.16793187751543467000,
    0.19938156160762566000,
    0.16793187751543467000,
    0.11256174105687924000,
    0.07122907943774888000,
    0.04172769649713474500,
    0.02168127068667904300,
    0.00883416031353021630,
    0.00121378071889749130,
    -0.00280012215560185900,
    -0.00446861323138686780,
    -0.00472444069729934920,
    -0.00422212099590341550,
    -0.00339431573437104200,
    -0.00250624192866901340,
    -0.00170346888080965400,
    -0.00105106340607596220
};

static double lpf4kHz_coeffs[NTAPS] =
{
    0.00023788185274493161,
    0.00014732315999688281,
    -0.00012610517776346129,
    -0.00068427605436556342,
    -0.00162403770285430670,
    -0.00298395274470884650,
    -0.00464731383860926030,
    -0.00618683792099326490,
    -0.00664136791959670480,
    -0.00422852529657953470,
    0.00397624012568426490,
    0.02232048518544594800,
    0.05667994398862042500,
    0.11417521732532962000,
    0.20220642969938624000,
    0.25475779063652526000,
    0.20220642969938624000,
    0.11417521732532962000,
    0.05667994398862042500,
    0.02232048518544594800,
    0.00397624012568426490,
    -0.00422852529657953470,
    -0.00664136791959670480,
    -0.00618683792099326490,
    -0.00464731383860926030,
    -0.00298395274470884650,
    -0.00162403770285430670,
    -0.00068427605436556342,
    -0.00012610517776346129,
    0.00014732315999688281,
    0.00023788185274493161
};

static double lpf5kHz_coeffs[NTAPS] =
{
    0.00008857651266302499,
    0.00018563654770944524,
    0.00029705790434555150,
    0.00035639479549170490,
    0.00022408277442446719,
    -0.00032666317820101296,
    -0.00157705390053453120,
    -0.00372405359560962780,
    -0.00655228341240485060,
    -0.00882463321284275050,
    -0.00733706812629941580,
    0.00429065592013054730,
    0.03675249953744918300,
    0.10535216347841343000,
    0.22811515202438210000,
    0.30535907186176536000,
    0.22811515202438210000,
    0.10535216347841343000,
    0.03675249953744918300,
    0.00429065592013054730,
    -0.00733706812629941580,
    -0.00882463321284275050,
    -0.00655228341240485060,
    -0.00372405359560962780,
    -0.00157705390053453120,
    -0.00032666317820101296,
    0.00022408277442446719,
    0.00035639479549170490,
    0.00029705790434555150,
    0.00018563654770944524,
    0.00008857651266302499
};

// Filter history buffers for all channels
static double filter_history[6][NTAPS];

// FIR implementation
static double fir(double input, double* coeffs, double* history, unsigned int ntaps)
{
    int i;
    double ret = 0;

    // Shift delay line
    for (i = ntaps - 2; i >= 0; i--)
    {
        history[i + 1] = history[i];
    }

    // Store input at the beginning of the delay line
    history[0] = input;

    // FIR calculation
    for (i = 0; i < ntaps; i++)
    {
        ret += coeffs[i] * history[i];
    }

    return ret;
}

// Delay state structure
typedef struct
{
    double* delayBuffer;
    double* bufferEndPointer;
    double* readPointer;
    double* writePointer;
    int delay;
    int bufferSize;
} DelayState;

// Delay buffers for each channel
static DelayState channel_delay_state[6];
static double channel_delay_buffer[6][MAX_DELAY_SAMPLES];

// Delay implementation
static void delayInit(DelayState* delayState, double* delayBuffer, int delayBufLen, int delay)
{
    printf("DEBUG delayInit: Starting, delayBufLen=%d, delay=%d\n", delayBufLen, delay);

    if (!delayBuffer) {
        printf("ERROR delayInit: delayBuffer is NULL!\n");
        return;
    }

    delayState->delayBuffer = delayBuffer;
    delayState->bufferEndPointer = delayState->delayBuffer + delayBufLen;
    delayState->writePointer = delayState->delayBuffer;
    delayState->delay = delay;
    delayState->bufferSize = delayBufLen;

    printf("DEBUG delayInit: Basic pointers set\n");

    // Set read pointer back by delay samples
    delayState->readPointer = delayState->writePointer - delay;

    // If readPointer is below buffer start, wrap around
    if (delayState->readPointer < delayState->delayBuffer) {
        printf("DEBUG delayInit: Wrapping read pointer\n");
        delayState->readPointer += delayBufLen;
    }

    printf("DEBUG delayInit: Read pointer = %p, Write pointer = %p\n",
        delayState->readPointer, delayState->writePointer);

    // Initialize buffer to 0
    printf("DEBUG delayInit: Initializing buffer to 0\n");
    for (int i = 0; i < delayBufLen; i++) {
        delayBuffer[i] = 0.0;
    }

    printf("DEBUG delayInit: Finished\n");
}

static double applyDelay(double input, DelayState* delayState)
{

    if (!delayState) {
        printf("ERROR applyDelay: delayState is NULL!\n");
        return input;
    }

    if (!delayState->delayBuffer || !delayState->writePointer || !delayState->readPointer) {
        printf("ERROR applyDelay: Invalid delay state pointers!\n");
        printf("  delayBuffer: %p\n", delayState->delayBuffer);
        printf("  writePointer: %p\n", delayState->writePointer);
        printf("  readPointer: %p\n", delayState->readPointer);
        return input;
    }

    if (!delayState->delayBuffer) return input;

    // Write to buffer
    *delayState->writePointer = input;
    delayState->writePointer++;

    // Wrap write pointer
    if (delayState->writePointer >= delayState->bufferEndPointer) {
        delayState->writePointer = delayState->delayBuffer;
    }

    // Read from buffer
    double ret = *delayState->readPointer;
    delayState->readPointer++;

    // Wrap read pointer
    if (delayState->readPointer >= delayState->bufferEndPointer) {
        delayState->readPointer = delayState->delayBuffer;
    }

    return ret;
}

// Add limiter function
static double add(double input0, double input1)
{
    double ret = input0 + input1;
    if (ret >= 1.0) ret = 0.99999999;
    if (ret < -1.0) ret = -1.0;
    return ret;
}

// Function to get coefficients based on selector
static double* getFilterCoeffs(int filter_select)
{
    switch (filter_select) {
    case 0: return lpf2kHz_coeffs;
    case 1: return lpf3kHz_coeffs;
    case 2: return lpf4kHz_coeffs;
    case 3: return lpf5kHz_coeffs;
    default: return lpf4kHz_coeffs;
    }
}

// Initialize delays for all channels
static void initCH0Delays()
{
    printf("DEBUG initCH0Delays: Starting\n");

    int sample_rate = SAMPLE_RATE;
    if (sample_rate == 0) sample_rate = 48000; // default

    printf("DEBUG initCH0Delays: Sample rate = %d\n", sample_rate);

    const int delay_samples_table[4] = {
        0,                      // 0ms
        (int)(sample_rate * 0.150),   // 150ms
        (int)(sample_rate * 0.300),   // 300ms
        (int)(sample_rate * 0.450)    // 450ms
    };

    printf("DEBUG initCH0Delays: Delay table = [%d, %d, %d, %d]\n",
        delay_samples_table[0], delay_samples_table[1],
        delay_samples_table[2], delay_samples_table[3]);

    for (int ch = 0; ch < 6; ch++) {
        printf("DEBUG initCH0Delays: Processing channel %d\n", ch);

        // Check if channel is enabled
        if (!moduleControl.channel_enable[ch]) {
            printf("DEBUG initCH0Delays: Channel %d disabled, skipping\n", ch);
            continue;
        }

        // Get delay value from table
        int delay_select = moduleControl.ch0_delay_select[ch];
        if (delay_select < 0 || delay_select > 3) {
            delay_select = 0; // default to 0ms if invalid
        }

        printf("DEBUG initCH0Delays: Channel %d delay_select = %d\n", ch, delay_select);

        int delay_samples = delay_samples_table[delay_select];

        printf("DEBUG initCH0Delays: Channel %d delay_samples = %d\n", ch, delay_samples);

        //dodato
        if (delay_samples >= MAX_DELAY_SAMPLES) {
            delay_samples = MAX_DELAY_SAMPLES - 1;
            printf("Warning: Delay for channel %d clamped to %d samples\n", ch, delay_samples);
        }

        printf("DEBUG initCH0Delays: Initializing delay for channel %d\n", ch);

        // Initialize delay for this channel
        delayInit(&channel_delay_state[ch], channel_delay_buffer[ch],
            MAX_DELAY_SAMPLES, delay_samples);

        printf("DEBUG initCH0Delays: Channel %d delay initialized\n", ch);
    }

    printf("DEBUG initCH0Delays: Finished\n");
}
// FX implementation
void FX_init(FX_ControlPanel* controlsInit)
{
    printf("DEBUG FX_init: Starting initialization\n");

    // Copy controls
    memcpy(&moduleControl, controlsInit, sizeof(FX_ControlPanel));

    printf("DEBUG FX_init: Controls copied, initializing delays...\n");

    // Initialize delays for all channels
    initCH0Delays();

    printf("DEBUG FX_init: Delays initialized, resetting filter history...\n");

    // Reset filter history
    memset(filter_history, 0, sizeof(filter_history));

    printf("DEBUG FX_init: Initialization complete\n");
}

void FX_processBlock()
{
    if (!moduleControl.on) return;

    // Get actual number of input channels
    int input_channels = HAOS::getInputStreamChCnt();  // This should be 2

    // DEBUG: Add this to see what's happening
    static int debug_counter = 0;
    if (debug_counter++ < 5) {
        printf("FX_processBlock: input_channels = %d\n", input_channels);
        printf("FX_processBlock: sampleBuffer pointer = %p\n", sampleBuffer);
        if (sampleBuffer) {
            printf("FX_processBlock: sampleBuffer[0] = %p\n", sampleBuffer[0]);
            printf("FX_processBlock: sampleBuffer[1] = %p\n", sampleBuffer[1]);
        }
    }

    for (int32_t i = 0; i < BLOCK_SIZE; i++) {
        // Process all 6 channels
        for (int ch = 0; ch < 6; ch++) {
            if (!moduleControl.channel_enable[ch]) continue;

            // FIXED: ALWAYS use input channels 0 and 1 for ALL FX channels
            // Only check if input channels actually exist
            double ch0_input = 0.0;
            double ch1_input = 0.0;

            if (input_channels >= 1) {
                ch0_input = sampleBuffer[0][i];  // Always from input channel 0
            }
            if (input_channels >= 2) {
                ch1_input = sampleBuffer[1][i];  // Always from input channel 1
            }

            // Process CH0 for this channel
            double processed_ch0;
            if (moduleControl.ch0_processing[ch]) {
                // Complete processing: -1.8dB -> delay -> -1.2dB
                processed_ch0 = ch0_input * GAIN_CH0_PRE;
                processed_ch0 = applyDelay(processed_ch0, &channel_delay_state[ch]);
                processed_ch0 = processed_ch0 * GAIN_CH0_POST;
            }
            else {
                // Gain only: -1.8dB
                processed_ch0 = ch0_input * GAIN_CH0_PRE;
            }

            // Process CH1 for this channel: -2.0dB -> filter -> -2.2dB
            double processed_ch1 = ch1_input * GAIN_CH1_PRE;

            // Apply filter to CH1 (if there's input)
            if (input_channels >= 2) {
                int filter_select = moduleControl.ch1_filter_select[ch];
                double* coeffs = getFilterCoeffs(filter_select);
                processed_ch1 = fir(processed_ch1, coeffs, filter_history[ch], NTAPS);
            }

            processed_ch1 = processed_ch1 * GAIN_CH1_POST;

            // SUM: processed CH1 + processed CH0
            sampleBuffer[ch][i] = add(processed_ch1, processed_ch0);

            // DEBUG: Check for NaN/inf
            if (processed_ch0 != processed_ch0 || processed_ch1 != processed_ch1) {  // NaN check
                printf("ERROR: NaN detected at ch=%d, i=%d\n", ch, i);
                return;
            }
        }
    }
}

// Function for parsing command line arguments (for standalone mode)
FX_ControlPanel FX_parseArguments(int argc, char* argv[])
{
    FX_ControlPanel config;

    // Default values
    config.on = 1;

    // Default all channels enabled
    for (int i = 0; i < 6; i++) {
        config.channel_enable[i] = 1;
        config.ch0_delay_select[i] = 0;        // 0ms delay for all
        config.ch0_processing[i] = 1;          // complete processing for all
        config.ch1_filter_select[i] = 2;       // 4kHz for all
    }

    return config;
}

void check_memory_sizes()
{
    std::cout << "Size of channel_delay_buffer: " << sizeof(channel_delay_buffer) << " bytes" << std::endl;
    std::cout << "Size of filter_history: " << sizeof(filter_history) << " bytes" << std::endl;
    std::cout << "Size of lpf coeffs: " << sizeof(lpf2kHz_coeffs) * 4 << " bytes" << std::endl;
    std::cout << "TOTAL STATIC MEMORY: " <<
        sizeof(channel_delay_buffer) +
        sizeof(filter_history) +
        (sizeof(lpf2kHz_coeffs) * 4) << " bytes" << std::endl;
}