#include "fx.h"
#include "haos_api.h"
#include <iostream>
#include <string.h>



static FX_ControlPanel fxMCV = {
    1,      // on = enabled

    // channel_enable[6]
    {1, 1, 1, 1, 1, 1},  // svi kanali enable-ovani

    // ch0_delay_select[6]
    {0, 1, 2, 3, 0, 1},  // 0ms, 150ms, 300ms, 450ms, 0ms, 150ms

    // ch0_processing[6]
    {0, 1, 1, 0, 1, 0},  // gain, full, full, gain, full, gain

    // ch1_filter_select[6]  
    {2, 0, 1, 2, 3, 0}   // 4kHz, 2kHz, 3kHz, 4kHz, 5kHz, 2kHz
};

// Globalna promenljiva za FX modul (originalni FX_ControlPanel)
static FX_ControlPanel moduleControlForHaOS;

// Konverziona funkcija: MCV -> FX_ControlPanel
static void convertMCVtoFXControlPanel()
{
    moduleControlForHaOS = fxMCV;
}

// Callback funkcije za FX modul
void __fg_call FX_preKick(void* mif);
void __fg_call FX_processBrick();
void __bg_call FX_background();

// MCT (Module Call Table) za FX modul
HAOS_Mct_t fxMCT = {
    FX_preKick,      // Pre-kick - inicijalizacija
    0,               // Post-kick
    0,               // Timer
    0,               // Frame
    FX_processBrick, // Brick - glavna processing funkcija
    0,               // AFAP
    FX_background,   // Background
    0,               // Post-malloc
    0                // Pre-malloc
};

// MIF (Module Interface) struktura
HAOS_Mif_t fxMIF = { &fxMCV, &fxMCT };

// Pre-kick callback - inicijalizacija modula
void __fg_call FX_preKick(void* mif)
{
    std::cout << ">> FX Module: Initializing..." << std::endl;

    // Direktno koristi fxMCV (koja je FX_ControlPanel)
    // Nema potrebe za konverzijom jer je MCV = FX_ControlPanel
    FX_init(&fxMCV);

    std::cout << ">> FX Module: Ready (MCV format)" << std::endl;
    std::cout << "   - On: " << (int)fxMCV.on << std::endl;

    // Prikaz konfiguracije kanala
    const char* filter_names[] = { "2kHz", "3kHz", "4kHz", "5kHz" };
    const char* delay_names[] = { "0ms", "150ms", "300ms", "450ms" };

    for (int i = 0; i < 6; i++) {
        std::cout << "   - Channel " << i << ": "
            << "Filter=" << filter_names[fxMCV.ch1_filter_select[i]]
            << ", Delay=" << delay_names[fxMCV.ch0_delay_select[i]]
            << ", Processing=" << (fxMCV.ch0_processing[i] ? "Full" : "Gain")
            << ", Enable=" << (fxMCV.channel_enable[i] ? "Yes" : "No")
            << std::endl;
    }


}
// Brick callback - glavna processing funkcija
void __fg_call FX_processBrick()
{
    //std::cout << "FX_processBrick: BYPASSED (return immediately)" << std::endl;
    //return;  // Bypass sve

    // Za mute možemo koristiti on=0 ili dodati poseban mute parametar
    if (fxMCV.on == 0) {
        // Ako je on=0, postavi sve kanale na 0
        for (int ch = 0; ch < 6; ch++) {
            if (fxMCV.channel_enable[ch]) {
                for (int i = 0; i < BRICK_SIZE; i++) {
                    HAOS::getIOChannelPointerTable()[ch][i] = 0.0;
                }
            }
        }
        return;
    }

    // Inače izvrši normalnu obradu
    FX_processBlock();

    // Postavi valid channel mask za enable-ovane kanale
    HAOS_ChannelMask_t mask = 0;
    for (int ch = 0; ch < 6; ch++) {
        if (fxMCV.channel_enable[ch]) {
            mask |= (1 << ch);
        }
    }
    HAOS::setValidChannelMask(mask);
}

// Background callback - može se koristiti za dinamičko update-ovanje kontrola
void __bg_call FX_background()
{
    // Ova funkcija može biti prazna ili se može koristiti za
    // dinamičko update-ovanje kontrola tokom runtime-a
    static int counter = 0;
    counter++;

    // Primer: Svakih 1000 poziva proveri da li treba ažurirati konfiguraciju
    if (counter % 1000 == 0) {
        // Ažuriraj FX_ControlPanel iz MCV (ako je MCV promenjen od strane hosta)
        convertMCVtoFXControlPanel();
    }
}



