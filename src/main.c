#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>


#define true    (1)
#define false   (0)


#define BA_NAME "BestAmp.exe"
#define BA_HELP BA_NAME \
" program useage:\n" \
"\t" BA_NAME " <GAIN> <BASS> <TREBLE> <POWER>\n" \
"\t\twhere GAIN,BASS,TREBLE,& POWER are numbers (integers) " \
"that are in the range 1-10, if a value is outside this range " \
"it will be set to 1 (the minimum)\n"


#define BA_EXPECTED_ARGS (5)
#define BA_DIAL_MIN (1)
#define BA_DIAL_MAX (10)



typedef struct BA_AMP_VALUES {
    int gain;           // 1-10, as per my amp
    int bass;           // 1-10, as per my amp
    int treble;         // 1-10, as per my amp
    int power;          // 1-10, as per my amp
} ampValues_t;


typedef struct BA_HOST_PREFS {
    int hostApiIdx;
    int inDevIdx;
    int outDevIdx;
} hostPrefs_t;


typedef struct BA_DEVICE_PREFS {
    int inChanneln; 
    int outChanneln;
    double sampleRate;
    int framesPerBuffer;
    long sampleFormat;  // I would almost like to be able to cast to the thing at this
} devPrefs_t;           // determine if you can store 'types' in variables (seems like a far fetched thing)


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
    devPrefs_t* defPrefs;
    PaHostApiInfo* hostApiInfo;
    PaDeviceInfo* inDevInfo;
    PaDeviceInfo* outDevInfo;
} appInfo_t;


/**
print `ampValues_t` struct (to be used if the program is being debugged
from the terminal)
*/
void printAmpValues(ampValues_t* a);

/**
Test that a value is between BA_DIAL_MIN & BA_DIAL_MAX
*/
int testAmpValue(int value);

/**
Sets the value of the `gain` property of the passed `ampValues_t` `a` to be
the value of `gainValue` or 0 if `gainValue` fails `testAmpValue` check.
*/
void setAmpGain(ampValues_t* a, int gainValue);

/**
Sets the value of the `bass` property of the passed `ampValues_t` `a` to be the
value of `bassValue` or 0 if `bassValue` fails `testAmpValue` check.
*/
void setAmpBass(ampValues_t* a, int bassValue);

/**
Sets the value of the `treble` property of the passed `ampValues_t` `a` to be the
value of `trebleValue` or 0 of `trebleValue` fails `testAmpValue` check.
*/
void setAmpTreble(ampValues_t* a, int trebleValue);

/**
Sets the value of the `power` property of the passed `ampValues_t` `a` to be the
value of `powerValue` or 0 if `powerValue` failas `testAmpValue` check.
*/
void setAmpPower(ampValues_t* a, int powerValue);


void printHostPrefs(hostPrefs_t* s);

/**
Set all host preferences to sentinel of `-1`
    This will tell the program to use the portaudio defaults for preferences
*/
void initHostPrefs(hostPrefs_t* s);

// [WO] at-least right now, these aren't really doing anything
// void setHostApiIdx(hostPrefs_t* s, int hostApiIndex);
// void setHostInDevIdx(hostPrefs_t* s, int inDevIdx);
// void setHostOutDevIdx(hostPrefs_t* s, int outDevIdx);


void printDevPrefs(devPrefs_t* d);

/**
Set all device preferences to sentinel of `-1`.
    This will tell the program to use the portaudio defaults for preferences
*/
void initDevPrefs(devPrefs_t* d);

// [WO] at-least right now, these aren't really doing anything
// void setDevInChanneln(devPrefs_t* d, int inChanneln);
// void setDevOutChanneln(devPrefs_t* d, int outChanneln);
// void setDevSampleRate(devPrefs_t* d, double sampleRate);
// void setDevFramesPerBuffer(devPrefs_t* d, int framesPerBuffer);


/**
dump struct fields to stdout.
*/
void printHostApiInfo(PaHostApiInfo* h);

/**
dump struct fields to stdout.
*/
void printDeviceInfo(PaDeviceInfo* d);


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



void printAmpValues(ampValues_t* a)
{
    printf("AmpValues: { gain: %d, bass: %d, treble: %d, power: %d }\n",
        a->gain, a->bass, a->treble, a->power); 
}

void initAmpValues(ampValues_t* a)
{
    a->gain = 1;
    a->bass = 1;
    a->treble = 1;
    a->power = 1;
}

int testAmpValue(int value)
{
    if (value < BA_DIAL_MIN || value > BA_DIAL_MAX)
        return false;
    return true;
}

void setAmpGain(ampValues_t* a, int gainValue)
{
    if (testAmpValue(gainValue))
        a->gain = gainValue;
    else
        a->gain = BA_DIAL_MIN;
}

void setAmpBass(ampValues_t* a, int bassValue)
{
    if (testAmpValue(bassValue))
        a->bass = bassValue;
    else
        a->bass = BA_DIAL_MIN;
}

void setAmpTreble(ampValues_t* a, int trebleValue)
{
    if (testAmpValue(trebleValue))
        a->treble = trebleValue;
    else
        a->treble = BA_DIAL_MIN;
}

void setAmpPower(ampValues_t* a, int powerValue)
{
    if (testAmpValue(powerValue))
        a->power = powerValue;
    else
        a->power = BA_DIAL_MIN;
}


void printHostPrefs(hostPrefs_t* h)
{
    printf(
        "HostPrefs: {"          \
            "hostIdx: %d, "     \
            "inDevIdx: %d, "    \
            "outDevIdx: %d, "   \
        "}\n",
        h->hostApiIdx,
        h->inDevIdx,
        h->outDevIdx
    );
}

void initHostPrefs(hostPrefs_t* h)
{
    h->hostApiIdx = -1;
    h->inDevIdx = -1;
    h->outDevIdx = -1; 
}


void printDevPrefs(devPrefs_t* d)
{
    printf(
        "DevPrefs: {"                   \
            "inChanneln: %d, "          \
            "outChanneln: %d, "         \
            "sampleRate: %f, "          \
            "sampleFormat: %ld, "       \
            "framesPerBuffer: %d, "     \
        "}\n",
        d->inChanneln,
        d->outChanneln,
        d->sampleRate,
        d->sampleFormat,
        d->framesPerBuffer
    );
}

void initDevPrefs(devPrefs_t* d)
{
    d->inChanneln = -1;
    d->outChanneln = -1;
    d->sampleRate = -1;
    d->sampleFormat = -1;
    d->framesPerBuffer = -1;
}


void printHostApiInfo(PaHostApiInfo* h)
{
    printf(
        "HostApiInfo: { "               \
            "structVersion: %d, "       \
            "type: %d, "                \
            "name: %s, "                \
            "deviceCount: %d, "         \
            "defaultInputDevice: %d, "  \
            "defaultOutputDevice: %d, " \
        "}\n",
        h->structVersion,
        h->type,
        h->name,
        h->deviceCount,
        h->defaultInputDevice,
        h->defaultOutputDevice
    );
}

void printDeviceInfo(PaDeviceInfo* d)
{
    printf(
        "DeviceInfo: { "                        \
            "structVersion: %d, "               \
            "name: %s, "                        \
            "maxInputChannels: %d, "            \
            "maxOutputChannels: %d, "           \
            "defaultLowInputLatency: %f, "      \
            "defaultLowOutpuLatency: %f, "      \
            "defaultHighInputLatency: %f, "     \
            "defaultHighOutputLatency: %f, "    \
            "defaultSampleRate: %f, "           \
        "}\n",
        d->structVersion,
        d->name,
        d->maxInputChannels,
        d->maxOutputChannels,
        d->defaultLowInputLatency,
        d->defaultLowOutputLatency,
        d->defaultHighInputLatency,
        d->defaultHighOutputLatency,
        d->defaultSampleRate
    );
}


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
    const float* in = (float*)input;
    float* out = (float*)output;
    // sysInfo_t* sysInfo = (sysInfo_t*)userData;

    // [WO] todo copy input to output
    // where 2 is the number of channels
    for (int i = 0; i < framesPerBuffer * 2; i++)
    {
        *out++ = 2 * *(in++);
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

    // int hostApiIndex        = -1;
    // int inputDeviceIndex    = -1;
    // int outputDeviceIndex   = -1;
    // int inputChannel        = -1; // channel idx of device (but what if multiple channels?), no count
    // int outputChannel       = -1; // channel idx of device (but what if multiple channels?), no count
    // int latency             = -1;
    // double sampleRate       = -1; // ex. 44100
    // int framesPerBuffer     = -1; // ex. 512
    // long sampleFormat       = paFloat32;


    ampValues_t amp = { 0 };
    initAmpValues(&amp); // BestAmp defaults (1,1,1,1)

    hostPrefs_t hostPrefs = { 0 };
    initHostPrefs(&hostPrefs); // no value sentinels (query from portaudio)

    devPrefs_t devPrefs = { 0 };
    initDevPrefs(&devPrefs); // no value sentinels (query from portaudio)

    appInfo_t appInfo = { 0 };

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
        devPrefs.sampleFormat = atol(argv[11]);     // device sample format
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
    if (devPrefs.sampleFormat < 0)
        devPrefs.sampleFormat = paInt32;
    if (devPrefs.sampleRate < 0)
        devPrefs.sampleRate = (appInfo.inDevInfo->defaultSampleRate > appInfo.outDevInfo->defaultSampleRate)
            ? appInfo.outDevInfo->defaultSampleRate
            : appInfo.inDevInfo->defaultSampleRate;     // whichever is smaller but they should be the same
    
    // Create Input/Output stream parameters from CLI stuff
    PaStreamParameters iStreamParams, oStreamParams;

    iStreamParams.device = hostPrefs.inDevIdx;
    iStreamParams.channelCount = appInfo.inDevInfo->maxInputChannels;
    iStreamParams.suggestedLatency = appInfo.inDevInfo->defaultLowInputLatency;
    iStreamParams.sampleFormat = devPrefs.sampleFormat;
    iStreamParams.hostApiSpecificStreamInfo = NULL;

    oStreamParams.device = hostPrefs.outDevIdx;
    oStreamParams.channelCount = appInfo.outDevInfo->maxOutputChannels;
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

    return 0;

error:
    Pa_Terminate();

    printf(BA_HELP);
    return e;
}
    
