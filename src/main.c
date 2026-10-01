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


typedef struct BA_SYS_INFO {
    int apiIndex;
    int inputDeviceIndex;
    int outputDeviceIndex;
    int inputDeviceChannelIdx;
    int outputDeviceChannelIdx;
    int latency;
    double sampleRate;
    long sampleFormat;
} sysInfo_t;


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
PaStreamCallbackResult bestAmpCB(
    const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
);



void printAmpValues(ampValues_t* a)
{
    printf("AmpValues: { gain: %d, bass: %d, treble: %d, power: %d }\n",
        a->gain, a->bass, a->treble, a->power); 
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
PaStreamCallbackResult bestAmpCB(
    const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
)
{
    // [WO] todo, copy intput to output
    return paContinue;
}


/**
Program Entry
*/
int main(int argc, char** argv)
{
    PaStream* stream;
    PaError e;

    int hostApiIndex        = -1;
    int inputDeviceIndex    = -1;
    int outputDeviceIndex   = -1;
    int inputChannel        = -1; // channel idx of device (but what if multiple channels?), no count
    int outputChannel       = -1; // channel idx of device (but what if multiple channels?), no count
    int latency             = -1;
    double sampleRate       = -1;
    long sampleFormat       = paFloat32;


    // [WO] probably dont have to be so strict, can just resolve them to nothing
    // parse command line arguments
    // figure out how many there are
    // then assign values as follows
    // GAIN BASS TREBLE POWER ...
    // resolving to 0 if there is no value (so it will get the default, which is the min)
    // if (argc != BA_EXPECTED_ARGS)
    //     goto error;
    
    // [WO] cmd args will be used for nowhttps://github.com/Even-Better-Software/BestAmp/tree/main
    // initialize the `amp` instance of the `ampValues_t` struct from cmd args
    // will be passed as the user data to PortAudio callback
    ampValues_t amp = { 0 };

    if (argc > 1)
        setAmpGain(&amp, atoi(argv[1]));    // gain cmd arg
    if (argc > 2)
        setAmpBass(&amp, atoi(argv[2]));    // bass cmd arg
    if (argc > 3)
        setAmpTreble(&amp, atoi(argv[3]));  // treble cmd arg
    if (argc > 4)
        setAmpPower(&amp, atoi(argv[4]));   // power cmd arg
    if (argc > 5) 
        hostApiIndex = atoi(argv[5]);
    if (argc > 6)
        inputDeviceIndex = atoi(argv[6]);
    if (argc > 7)
        inputChannel = atoi(argv[7]);
    if (argc > 8)
        outputChannel = atoi(argv[8]);
    if (argc > 9)
        latency = atoi(argv[9]);
    if (argc > 10)
        sampleRate = atof(argv[10]);
    if (argc > 11)
        sampleFormat = atol(argv[11]);      // format values
    
    // show the values
    printAmpValues(&amp);
    printf("TEMP: { hostApiIndex: %d, inputDeviceIndex: %d, outputDeviceIndex: %d, inputChannel: %d, outputChannel: %d }\n",
        hostApiIndex, inputDeviceIndex, outputDeviceIndex, inputChannel, outputChannel);
    printf("\n");


    // initalize portaudio
    Pa_Initialize();
    if (e != paNoError)
        goto error;
   
 
    // [WO] maybe hidden by a switch (so that people can see what
    // APIs/Interfaces they can use w/ the program)?

    // enumerate host api info
    PaHostApiInfo*  hinfo;
    PaDeviceInfo*   dinfo;
    PaDeviceInfo*   oinfo;
    PaDeviceInfo*   iinfo;

    int hostApiCount = Pa_GetHostApiCount();
    
    printf("Available host APIs & devices\n");
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


    // reduce system values to defaults (as per pa query results)
    

    
    // [WO] info dump ends
    // then program continues (no more info dump)

    // Create Input/Output stream parameters from CLI stuff
    PaStreamParameters iStreamParams, oStreamParams;

    iStreamParams.device = inputDeviceIndex;
    iStreamParams.channelCount = inputChannel;
    iStreamParams.suggestedLatency = latency;
    iStreamParams.sampleFormat = sampleFormat;
    iStreamParams.hostApiSpecificStreamInfo = NULL;

    oStreamParams.device = outputDeviceIndex;
    oStreamParams.channelCount = outputChannel;
    oStreamParams.suggestedLatency = latency;
    oStreamParams.sampleFormat = sampleFormat;
    oStreamParams.hostApiSpecificStreamInfo = NULL;

    // Is format supported test 
    // Pa_IsFormatSupported
    e = Pa_IsFormatSupported(&iStreamParams, &oStreamParams, sampleRate);
    if (e != paNoError)
        goto error;

    // Pa_OpenStream(...)
    // Pa_CloseStream(...)
   


    // terminate portaudio
    Pa_Terminate();

    return 0;

error:
    Pa_Terminate();

    printf(BA_HELP);
    return e;
}
    
