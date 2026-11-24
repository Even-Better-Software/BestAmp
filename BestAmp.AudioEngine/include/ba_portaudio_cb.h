#ifndef BA_PORTAUDIO_CB
#define BA_PORTAUDIO_CB


#include <portaudio.h>

#include "ba_app.h"
#include "ba_common.h"
#include "ba_prefs.h"
#include "ba_app.h"
#include "ba_dsp.h"


int bestAmpCB(
    const void* input,
    void* output,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
);


#endif

