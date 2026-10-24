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
"\t" BA_NAME


/**
Program Entry
*/
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    PaError e;

    appInfo_t appInfo = { 0 };

    ampValues_t amp = { 0 };
    baPrefs_t prefs = { 0 };

    initAmpValues(&amp); // BestAmp defaults (1,1,1,1)
    initBaPrefs(&prefs);

    appInfo.ampValues = amp;
    appInfo.baPrefs = prefs;

    // [WO] testing show all the ampVoices
    printAmpVoices();

    // initalize portaudio
    e = Pa_Initialize();
    if (e != paNoError)
        goto error;

    // coalesce to system defaults (mostly)
    // failsafe initial state value
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

    // TCP LOOP BEGINS
    // [WO] add some return & then check that
    int sockExitStatus = initWinsock(&appInfo);
    if (sockExitStatus != 0)
        goto error;
    
    // terminate portaudio
    Pa_Terminate();

    return 0;

error:
    // still need to terminate portaudio
    Pa_Terminate();

    printf(BA_HELP);
    return e;
}
    
