#include "ba_prefs.h"


void printBaPrefs(baPrefs_t* prefs)
{
    LINFO(
        "Prefs: {"                      \
            "hostIdx: %d, "             \
            "inDevIdx: %d, "            \
            "outDevIdx: %d, "           \
            "inChanneln: %d, "          \
            "outChanneln: %d, "         \
            "framesPerBuffer: %d, "     \
            "sampleFormat: %d, "       \
            "sampleRate: %lf, "         \
        "}",
        prefs->hostApiIdx,
        prefs->inDevIdx,
        prefs->outDevIdx,
        prefs->inChanneln,
        prefs->outChanneln,
        prefs->framesPerBuffer,
        prefs->sampleFormat,
        prefs->sampleRate
    );
}


void initBaPrefs(baPrefs_t* prefs)
{
    prefs->hostApiIdx = -1;
    prefs->inDevIdx = -1;
    prefs->outDevIdx = -1;
    prefs->inChanneln = -1;
    prefs->outChanneln = -1;
    prefs->framesPerBuffer = -1;
    prefs->sampleFormat = paFloat32;
    prefs->sampleRate = -1.0;
    prefs->hostApi = NULL;
    prefs->inDev = prefs->outDev = NULL;
}


int initBaPrefsToPaDefaults(baPrefs_t* prefs)
{
    PaHostApiIndex defaultHostApiIdx;
    PaHostApiInfo* defaultHostApi;

    PaDeviceIndex defaultiDevIdx, defaultoDevIdx;
    PaDeviceInfo* defaultiDev;
    PaDeviceInfo* defaultoDev;

    defaultHostApiIdx = Pa_GetDefaultHostApi();
    if (defaultHostApiIdx < 0) {
        LERROR("error getting the default host API: %s",
            Pa_GetErrorText(defaultHostApiIdx));
        return -1;
    }
    prefs->hostApiIdx = defaultHostApiIdx;
    defaultHostApi = (PaHostApiInfo*)Pa_GetHostApiInfo(prefs->hostApiIdx);
    if (defaultHostApi == NULL) {
        LERROR("null host info for host api idx %d", prefs->hostApiIdx);
        return -1;
    }
    prefs->hostApi = defaultHostApi;

    defaultiDevIdx = Pa_GetDefaultInputDevice();
    if (defaultiDevIdx < 0) {
        LERROR("error getting the default input device: %s",
            Pa_GetErrorText(defaultiDevIdx));
        return -1;
    }
    prefs->inDevIdx = defaultiDevIdx;
    defaultiDev = (PaDeviceInfo*)Pa_GetDeviceInfo(defaultiDevIdx);
    if (defaultiDev == NULL) {
        LERROR("null device info for device idx %d", defaultiDevIdx);
        return -1;
    }
    prefs->inDev = defaultiDev;

    defaultoDevIdx = Pa_GetDefaultOutputDevice();
    if (defaultoDevIdx < 0) {
        LERROR("error getting the default output device: %s",
            Pa_GetErrorText(defaultoDevIdx));
        return -1;
    }
    prefs->outDevIdx = defaultoDevIdx;
    defaultoDev = (PaDeviceInfo*)Pa_GetDeviceInfo(defaultoDevIdx);
    if (defaultoDev == NULL) {
        LERROR("null device info for device idx %d", defaultoDevIdx);
        return -1;
    }
    prefs->outDev = defaultoDev;
    
    prefs->inChanneln = defaultiDev->maxInputChannels;
    prefs->outChanneln = defaultoDev->maxOutputChannels;
    prefs->sampleRate = (
        defaultiDev->defaultSampleRate < defaultoDev->defaultSampleRate
    )
        ? defaultiDev->defaultSampleRate
        : defaultoDev->defaultSampleRate;

    // [WO] TODO
    prefs->sampleFormat = paFloat32;
    prefs->framesPerBuffer = 512;

    return 0;
}

int coalesceBaPrefsToPaDefaults(baPrefs_t* prefs)
{
    PaHostApiIndex hostApiIdx;
    PaHostApiInfo* hostApi;

    PaDeviceIndex iDevIdx, oDevIdx;
    PaDeviceInfo* iDev;
    PaDeviceInfo* oDev;

    if (prefs->hostApiIdx < 0) {
        hostApiIdx = Pa_GetDefaultHostApi();
        if (hostApiIdx < 0) {
            LERROR("error getting the default host API: %s",
                Pa_GetErrorText(hostApiIdx));
            return -1;
        }
        prefs->hostApiIdx = hostApiIdx;
    }
    hostApi = (PaHostApiInfo*)Pa_GetHostApiInfo(prefs->hostApiIdx);
    if (hostApi == NULL) {
        LERROR("null host info for host api idx %d", prefs->hostApiIdx);
        return -1;
    }
    prefs->hostApi = hostApi;

    if (prefs->inDevIdx < 0) {
        iDevIdx = Pa_GetDefaultInputDevice();
        if (iDevIdx < 0) {
            LERROR("error getting the default input device: %s",
                Pa_GetErrorText(iDevIdx));
            return -1;
        }
        prefs->inDevIdx = iDevIdx;
    }
    iDev = (PaDeviceInfo*)Pa_GetDeviceInfo(prefs->inDevIdx);
    if (iDev == NULL) {
        LERROR("null device info for device idx %d", prefs->inDevIdx);
        return -1;
    }
    prefs->inDev = iDev;

    if (prefs->outDevIdx < 0) {
        oDevIdx = Pa_GetDefaultOutputDevice();
        if (oDevIdx < 0) {
            LERROR("error getting the default output device: %s",
                Pa_GetErrorText(oDevIdx));
            return -1;
        }
        prefs->outDevIdx = oDevIdx;
    }
    oDev = (PaDeviceInfo*)Pa_GetDeviceInfo(prefs->outDevIdx);
    if (oDev == NULL) {
        LERROR("null device info for device idx %d", prefs->outDevIdx);
        return -1;
    }
    prefs->outDev = oDev;
    
    if (prefs->inChanneln < 0)
        prefs->inChanneln = iDev->maxInputChannels;
    if (prefs->outChanneln < 0)
        prefs->outChanneln = oDev->maxOutputChannels;
    if (prefs->sampleRate < 0)
        prefs->sampleRate = (
            iDev->defaultSampleRate < oDev->defaultSampleRate
        )
        ? iDev->defaultSampleRate
        : oDev->defaultSampleRate;

    // [WO] TODO
    prefs->sampleFormat = paFloat32;
    prefs->framesPerBuffer = 512;

    return 0;
}

