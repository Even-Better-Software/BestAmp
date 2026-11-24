#include <stdio.h>
#include <portaudio.h>

#include "ba_common.h"
#include "ba_portaudio_helpers.h"


/**
[WO] this is not to be considered part of the BestAmp source proper
    its a helper program that is used (mainly for dev purposes)
    it merely dumps hostapiinfo & deviceinfo struct values to the console
*/


int main()
{
    PaError e;

    // initalize portaudio
    e = Pa_Initialize();
    if (e != paNoError)
        goto error;

    // enumerate host api info
    PaHostApiInfo*  hinfo;
    PaDeviceInfo*   dinfo;
    PaDeviceInfo*   oinfo;
    PaDeviceInfo*   iinfo;

    int hostApiCount = Pa_GetHostApiCount();
    
    printf(BA_INFO "Available host APIs\n");
    printf(BA_INFO "%d\n", hostApiCount);
    printf("\n");

    for (int i = 0; i < hostApiCount; i++) {
        hinfo = (PaHostApiInfo*)Pa_GetHostApiInfo(i);
        if (hinfo == NULL) {
            printf(BA_ERROR "host w/ index %d is NULL, skipping.\n", i);
            continue;
        }
        printf(BA_INFO);
        printHostApiInfo(hinfo);
        // print host api info
        // enumerate host api device info
        for (int j = 0; j < hinfo->deviceCount; j++) {
            dinfo = (PaDeviceInfo*)Pa_GetDeviceInfo(j);
            if (dinfo == NULL) {
                printf(BA_ERROR "device w/ index %d is NULL, skipping.\n", j);
                continue;
            }
            printf(BA_INFO);
            printDeviceInfo(dinfo);
        }
        printf("\n");
    }
    
    printf(BA_INFO "default host API & devices\n");

    // show default hostapi
    hinfo = (PaHostApiInfo*)Pa_GetHostApiInfo(Pa_GetDefaultHostApi());
    if (hinfo == NULL) {
        printf(BA_ERROR "default host API is NULL, error");
        goto error;
    }
    printf(BA_INFO);
    printHostApiInfo(hinfo);

    // show default input device
    iinfo = (PaDeviceInfo*)Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    if (iinfo == NULL) {
        printf(BA_ERROR "default input device is NULL, error exit");
        goto error;
    }
    printf(BA_INFO);
    printDeviceInfo(iinfo);

    // show default output device
    oinfo = (PaDeviceInfo*)Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
    if (oinfo == NULL) {
        printf(BA_ERROR "default output device is NULL, error exit");
        goto error;
    }
    printf(BA_INFO);
    printDeviceInfo(oinfo);
    printf("\n");

    Pa_Terminate();

    return 0;

error:
    Pa_Terminate();

    return 1;
}

