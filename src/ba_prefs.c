#include <stdio.h>

#include "ba_prefs.h"


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

