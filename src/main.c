#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "ba_common.h"
#include "ba_amp.h"
#include "ba_dsp.h"
#include "ba_portaudio_helpers.h"
#include "ba_prefs.h"
#include "ba_tcp.h"


#define BA_NAME "BestAmp.exe"
#define BA_HELP BA_NAME \
" program useage:\n" \
"\t" BA_NAME " <GAIN> <BASS> <TREBLE> <POWER>\n" \
"\t\twhere GAIN,BASS,TREBLE,& POWER are numbers (integers) " \
"that are in the range 1-10, if a value is outside this range " \
"it will be set to 1 (the minimum)\n"

#define BA_EXPECTED_ARGS (5)


/**
Program Entry
*/
int main(int argc, char** argv)
{
    PaError e;

    ampValues_t amp = { 0 };
    baPrefs_t prefs = { 0 };
    
    // [WO] this is no effectively deprecated, although these values
    // technically server as inital values
    // app info state container
    appInfo_t appInfo = { 0 };
    appInfo.ampValues = amp;
    appInfo.baPrefs = prefs;

    initAmpValues(&amp); // BestAmp defaults (1,1,1,1)
    initBaPrefs(&prefs);

    // parse CLI arguments
    if (argc > 1)
        setAmpGain(&amp, atoi(argv[1]));    // gain cmd arg
    if (argc > 2)
        setAmpBass(&amp, atoi(argv[2]));    // bass cmd arg
    if (argc > 3)
        setAmpTreble(&amp, atoi(argv[3]));  // treble cmd arg
    if (argc > 4)
        setAmpPower(&amp, atoi(argv[4]));   // power cmd arg

    if (argc > 5) 
        prefs.hostApiIdx = atoi(argv[5]);   // host api index
    if (argc > 6)
        prefs.inDevIdx = atoi(argv[6]);     // host input device index
    if (argc > 7)
        prefs.outDevIdx = atoi(argv[7]);    // host output device index
    if (argc > 8)
        prefs.inChanneln = atoi(argv[8]);    // device input channel number
    if (argc > 9)
        prefs.outChanneln = atoi(argv[9]);   // device output channel number
    if (argc > 10)
        prefs.sampleRate = atof(argv[10]);       // device sample rate
    if (argc > 11)
        prefs.sampleFormat = paFloat32;          // [WO] for now its always this
        // prefs.sampleFormat = atol(argv[11]);  // device sample format
    if (argc > 12)
        prefs.framesPerBuffer = atoi(argv[12]);  // device frames per buffer
   
 
    // show the values
    printf(BA_INFO);
    printAmpValues(&amp);
    printf(BA_INFO);
    printBaPrefs(&prefs);
    printf("\n");

    // initalize portaudio
    e = Pa_Initialize();
    if (e != paNoError)
        goto error;

    // coalesce to system defaults (mostly)
    e = coalesceBaPrefsToPaDefaults(&prefs);
    if (e < 0) {
        printf("\e[31merror coalescing user preferences\e[0m: %s\n", Pa_GetErrorText(e));
        goto error;
    }

    printf(BA_INFO);
    printBaPrefs(&prefs);
    printf(BA_INFO);
    printHostApiInfo(prefs.hostApi);
    printf(BA_INFO);
    printDeviceInfo(prefs.inDev);
    printf(BA_INFO);
    printDeviceInfo(prefs.outDev);

    // [WO] TCP connection loop begins here
    // Logic -> request to update system settings
    // resetup the entire thing (re-construct the portaudio stream)
    // request to update amp settings
    // only have to re-calculate amp biquad coeffecients, stream can
    // possibly remain running

    // Create Input/Output stream parameters
    /*
    PaStreamParameters iStreamParams, oStreamParams;

    iStreamParams.device = prefs.inDevIdx;
    iStreamParams.channelCount = prefs.inChanneln;
    iStreamParams.suggestedLatency = prefs.inDev->defaultLowInputLatency;
    iStreamParams.sampleFormat = prefs.sampleFormat;
    iStreamParams.hostApiSpecificStreamInfo = NULL;

    oStreamParams.device = prefs.outDevIdx;
    oStreamParams.channelCount = prefs.outChanneln;
    oStreamParams.suggestedLatency = prefs.outDev->defaultLowOutputLatency;
    oStreamParams.sampleFormat = prefs.sampleFormat;
    oStreamParams.hostApiSpecificStreamInfo = NULL;

    // Is format supported test 
    // Pa_IsFormatSupported
    e = Pa_IsFormatSupported(&iStreamParams, &oStreamParams, prefs.sampleRate);
    if (e != paNoError) {
        printf("\e[31mFormat supported error\e[0m: %s\n", Pa_GetErrorText(e));
        goto error;
    }

    // [WO] whenever we are receiving input from the tcp connection
    // if the message specifies a new value for bass or treble
    // the biQuadFilter will beed to have its coeffecients recalculated
    // meaning that one of these functions will need to be invoked
    // for now, calculate biquad coeffecients here
    biQuadFilter_lowShelf(&amp.bqf_bass, (float)amp.bass, prefs.sampleRate);
    biQuadFilter_highShelf(&amp.bqf_treble, (float)amp.treble, prefs.sampleRate);


    e = Pa_OpenStream(  &appInfo->stream,
                        &iStreamParams,
                        &oStreamParams,
                        prefs.sampleRate,
                        prefs.framesPerBuffer,
                        0,
                        bestAmpCB,
                        &appInfo );
    if (e != paNoError) {
        printf("\e[31mOpen stream error\e[0m: %s\n", Pa_GetErrorText(e));
        goto error;
    }
    e = Pa_StartStream(appInfo->stream);
    if (e != paNoError) {
        printf("\e[31mStart stream error\e[0m: %s\n", Pa_GetErrorText(e));
        goto error;
    }
    */

    // [WO] I know this is bad but...
    // getchar();
    initWinsock(&appInfo);
    
    /*
    e = Pa_CloseStream(stream);
    if (e != paNoError) {
        printf("Close stream error: \e[31m%s\e[0m\n", Pa_GetErrorText(e));
        goto error;
    }
    */
   
    // terminate portaudio
    Pa_Terminate();

    // tcp test (remove later)
    // int test_res = test();
    // printf("tcp test result: %d\n", test_res);

    return 0;

error:
    Pa_Terminate();

    printf(BA_HELP);
    return e;
}
    
