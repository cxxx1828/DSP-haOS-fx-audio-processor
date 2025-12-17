#ifndef FX_H
#define FX_H

#include <stdint.h>

typedef struct
{
    uint32_t on;                     // glavni enable

    // Konfiguracija za SVAKI od 6 kanala
    uint32_t channel_enable[6];      // enable za svaki kanal (CH0-CH5)
    uint32_t ch0_delay_select[6];    // delay za CH0: 0: 0ms, 1: 150ms, 2: 300ms, 3: 450ms
    uint32_t ch0_processing[6];      // CH0 processing: 0: samo gain, 1: kompletan
    uint32_t ch1_filter_select[6];   // filter za CH1: 0: 2kHz, 1: 3kHz, 2: 4kHz, 3: 5kHz

} FX_ControlPanel;

void FX_init(FX_ControlPanel* controlsInit);
void FX_processBlock();
FX_ControlPanel FX_parseArguments(int argc, char* argv[]);

#endif