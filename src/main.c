#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "ba_amp.h"
#include "ba_dsp.h"
#include "ba_prefs.h"
#include "ba_portaudio_helpers.h"
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
Container for application state.
    `hostApiInfo`
        should be a pointer to `PaHostApiInfo` of the target host API
    `inDevInfo`
        should be a pointer to `PaDeviceInfo` of the target input device
    `outDevInfo`
        should be a pointer to `PaDeviceInfo` of the target output device
*/
typedef struct BA_APP_INFO {
    ampValues_t* ampValues;
    hostPrefs_t* hostPrefs;
    devPrefs_t* devPrefs;
    PaHostApiInfo* hostApiInfo;
    PaDeviceInfo* inDevInfo;
    PaDeviceInfo* outDevInfo;
} appInfo_t;


/**
BestAmp PortAudio callback definition.
*/
int bestAmpCB(
    const void* input,
    void* output,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
);


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
    const float* in = (float*)input;        // this has to be derived from the chosen buffer format
    float* out = (float*)output;            // this has to be derived from the chosen buffer format
    // sysInfo_t* sysInfo = (sysInfo_t*)userData;
    appInfo_t* appInfo = (appInfo_t*)userData;
    // (void)userData;
    (void)timeInfo;
    (void)statusFlags;

    // [WO] todo copy input to output
    // where 2 is the number of channels
    for (unsigned int i = 0; i < framesPerBuffer * appInfo->devPrefs->inChanneln; i++)
    {
        *out++ = appInfo->ampValues->gain * *(in++);
    }

    return paContinue;
}


/**
Program Entry
*/
int main(int argc, char** argv)
{
    PaStream* stream;
    PaError e;

    ampValues_t amp = { 0 };
    hostPrefs_t hostPrefs = { 0 };
    devPrefs_t devPrefs = { 0 };
    
    // app info state container
    appInfo_t appInfo = { 0 };
    appInfo.ampValues = &amp;
    appInfo.devPrefs = &devPrefs;
    appInfo.hostPrefs = &hostPrefs;

    initAmpValues(&amp); // BestAmp defaults (1,1,1,1)
    initHostPrefs(&hostPrefs);
    initDevPrefs(&devPrefs);


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
        hostPrefs.hostApiIdx = atoi(argv[5]);   // host api index
    if (argc > 6)
        hostPrefs.inDevIdx = atoi(argv[6]);     // host input device index
    if (argc > 7)
        hostPrefs.outDevIdx = atoi(argv[7]);    // host output device index

    if (argc > 8)
        devPrefs.inChanneln = atoi(argv[8]);    // device input channel number
    if (argc > 9)
        devPrefs.outChanneln = atoi(argv[9]);   // device output channel number
    if (argc > 10)
        devPrefs.sampleRate = atof(argv[10]);       // device sample rate
    if (argc > 11)
        devPrefs.sampleFormat = paFloat32;          // [WO] for now its always this
        // devPrefs.sampleFormat = atol(argv[11]);  // device sample format
    if (argc > 12)
        devPrefs.framesPerBuffer = atoi(argv[12]);  // device frames per buffer
    
    // show the values
    printAmpValues(&amp);
    printHostPrefs(&hostPrefs);
    printDevPrefs(&devPrefs);
    printf("\n");

    // initalize portaudio
    e = Pa_Initialize();
    if (e != paNoError)
        goto error;
   
    
    // [WO] THIS DOES NOT HAVE TO BE PART OF THIS PROGRAM AT ALL
    // [WO] maybe hidden by a switch (so that people can see what
    // APIs/Interfaces they can use w/ the program)?

    // enumerate host api info
    PaHostApiInfo*  hinfo;
    PaDeviceInfo*   dinfo;
    PaDeviceInfo*   oinfo;
    PaDeviceInfo*   iinfo;

    int hostApiCount = Pa_GetHostApiCount();
    
    printf("Available host APIs\n");
    printf("%d\n", hostApiCount);
    printf("\n");

    for (int i = 0; i < hostApiCount; i++) {
        hinfo = (PaHostApiInfo*)Pa_GetHostApiInfo(i);
        printHostApiInfo(hinfo);
        // print host api info
        // enumerate host api device info
        for (int j = 0; j < hinfo->deviceCount; j++) {
            dinfo = (PaDeviceInfo*)Pa_GetDeviceInfo(j);
            printDeviceInfo(dinfo);
        }
        printf("\n");
    }
    
    printf("default host API & devices\n");
    // show default hostapi
    hinfo = (PaHostApiInfo*)Pa_GetHostApiInfo(Pa_GetDefaultHostApi());
    printHostApiInfo(hinfo);
    // show default input device
    iinfo = (PaDeviceInfo*)Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    printDeviceInfo(iinfo);
    // show default output device
    oinfo = (PaDeviceInfo*)Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
    printDeviceInfo(oinfo);
    printf("\n");
    // [WO] END OF THE PART THAT IS IGNORABLE


    // coalesce to system defaults (mostly)
    if (hostPrefs.hostApiIdx < 0)
        hostPrefs.hostApiIdx = Pa_GetDefaultHostApi();
    appInfo.hostApiInfo = (PaHostApiInfo*)Pa_GetHostApiInfo(hostPrefs.hostApiIdx);
    if (appInfo.hostApiInfo == NULL) {
        printf("\e[31merror getting host api info\e[0m\n");
        goto error;
    }
    if (hostPrefs.inDevIdx < 0)
        hostPrefs.inDevIdx = Pa_GetDefaultInputDevice();
    appInfo.inDevInfo = (PaDeviceInfo*)Pa_GetDeviceInfo(hostPrefs.inDevIdx);
    if (appInfo.inDevInfo == NULL) {
        printf("\e[31merror getting in device inf\e[0m\n");
        goto error;
    }
    if (hostPrefs.outDevIdx < 0)
        hostPrefs.outDevIdx = Pa_GetDefaultOutputDevice();
    appInfo.outDevInfo = (PaDeviceInfo*)Pa_GetDeviceInfo(hostPrefs.outDevIdx);
    if (appInfo.outDevInfo == NULL) {
        printf("\e[31merror getting out device info\e[0m\n");
        goto error;
    }
    if (devPrefs.inChanneln < 0)
        devPrefs.inChanneln = appInfo.inDevInfo->maxInputChannels;
    if (devPrefs.outChanneln < 0)
        devPrefs.outChanneln = appInfo.outDevInfo->maxOutputChannels;
    if (devPrefs.sampleFormat < 0)
        devPrefs.sampleFormat = paFloat32;
        //devPrefs.sampleFormat = paInt32;
    if (devPrefs.framesPerBuffer < 0)
        devPrefs.framesPerBuffer = 512;
    if (devPrefs.sampleRate < 0)
        devPrefs.sampleRate = (appInfo.inDevInfo->defaultSampleRate > appInfo.outDevInfo->defaultSampleRate)
            ? appInfo.outDevInfo->defaultSampleRate
            : appInfo.inDevInfo->defaultSampleRate;     // whichever is smaller but they should be the same
 
    printHostPrefs(&hostPrefs);
    printDevPrefs(&devPrefs);
    
    // Create Input/Output stream parameters from CLI stuff
    PaStreamParameters iStreamParams, oStreamParams;

    iStreamParams.device = hostPrefs.inDevIdx;
    iStreamParams.channelCount = devPrefs.inChanneln;
    iStreamParams.suggestedLatency = appInfo.inDevInfo->defaultLowInputLatency;
    iStreamParams.sampleFormat = devPrefs.sampleFormat;
    iStreamParams.hostApiSpecificStreamInfo = NULL;

    oStreamParams.device = hostPrefs.outDevIdx;
    oStreamParams.channelCount = devPrefs.outChanneln;
    oStreamParams.suggestedLatency = appInfo.outDevInfo->defaultLowOutputLatency;
    oStreamParams.sampleFormat = devPrefs.sampleFormat;
    oStreamParams.hostApiSpecificStreamInfo = NULL;

    // Is format supported test 
    // Pa_IsFormatSupported
    e = Pa_IsFormatSupported(&iStreamParams, &oStreamParams, devPrefs.sampleRate);
    if (e != paNoError) {
        printf("Format supported error: \e[31m%s\e[0m\n", Pa_GetErrorText(e));
        goto error;
    }

    e = Pa_OpenStream(  &stream,
                        &iStreamParams,
                        &oStreamParams,
                        devPrefs.sampleRate,
                        devPrefs.framesPerBuffer,
                        0,
                        bestAmpCB,
                        &appInfo );
    if (e != paNoError) {
        printf("Open stream error: \e[31m%s\e[0m\n", Pa_GetErrorText(e));
        goto error;
    }
    e = Pa_StartStream(stream);
    if (e != paNoError) {
        printf("Start stream error: \e[31m%s\e[0m\n", Pa_GetErrorText(e));
        goto error;
    }

    // [WO] I know this is bad but...
    getchar();

    e = Pa_CloseStream(stream);
    if (e != paNoError) {
        printf("Close stream error: \e[31m%s\e[0m\n", Pa_GetErrorText(e));
        goto error;
    }
   
    // terminate portaudio
    Pa_Terminate();

    // tcp test (remove later)
    int test_res = test();
    printf("tcp test result: %d\n", test_res);

    return 0;

error:
    Pa_Terminate();

    printf(BA_HELP);
    return e;
}
    
