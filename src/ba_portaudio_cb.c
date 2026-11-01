#include "ba_portaudio_cb.h"


// BestAmp PortAudio callback implementation.
int bestAmpCB(
    const void* input,
    void* output,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
)
{
    const float* in = (float*)input; // this has to be derived from the chosen buffer format
    float* out = (float*)output; // this has to be derived from the chosen buffer format
    // sysInfo_t* sysInfo = (sysInfo_t*)userData;
    appInfo_t* appInfo = (appInfo_t*)userData;
    // (void)userData;
    (void)timeInfo;
    (void)statusFlags;

    // [WO] todo copy input to output
    // where 2 is the number of channels
    float y;
    for (unsigned int i = 0; i < framesPerBuffer * appInfo->baPrefs.inChanneln; i++)
    {
        y = appInfo->ampValues.gainMult * *in;     // pre-amplification

        y = biQuadFilter_process(
            &appInfo->ampValues.bqf_bass, y);      // execute bass biQuad filter process
        y = biQuadFilter_process(
            &appInfo->ampValues.bqf_treble, y);    // execute treble biQuad filter process

        y = appInfo->ampValues.powerMult * y;      // power-amplification

        *out = y; // write the processed sample to the output buffer
        
        // advance pointers 
        out++;
        in++;
    }

    return paContinue;
}

