#include <stdio.h>
#include <portaudio.h>

#include "ba_portaudio_helpers.h"


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

