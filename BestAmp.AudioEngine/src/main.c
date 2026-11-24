#include <portaudio.h>

#include "ba_app.h"
#include "ba_common.h"
#include "ba_tcp.h"


#define BA_NAME "BestAmp.exe"
#define BA_HELP BA_NAME \
" program useage:\n" \
"\t" BA_NAME



int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    LINFO("starting BestAmp audio engine");

    PaError e;

    appInfo_t appInfo = { 0 };

    ampValues_t amp = { 0 };
    baPrefs_t prefs = { 0 };

    initAmpValues(&amp); // BestAmp defaults (1,1,1,1)
    initBaPrefs(&prefs);

    appInfo.ampValues = amp;
    appInfo.baPrefs = prefs;

#ifdef BA_DEBUG
    LINFO("voices %d", getAmpVoiceCount());
#endif

    // initalize portaudio
    e = Pa_Initialize();
    if (e != paNoError) {
        LERROR("error initializing portaudio");
        goto error;
    }

    // coalesce to system defaults
    // failsafe initial state value
    e = coalesceBaPrefsToPaDefaults(&prefs);
    if (e < 0) {
        LERROR("error determining user preferences system defaults");
        goto error;
    }

#ifdef BA_DEBUG
    printf(BA_INFO);
    printBaPrefs(&prefs);
    printf(BA_INFO);
    printHostApiInfo(prefs.hostApi);
    printf(BA_INFO);
    printDeviceInfo(prefs.inDev);
    printf(BA_INFO);
    printDeviceInfo(prefs.outDev);
#endif

    int sockExitStatus = initWinsock(&appInfo);
    if (sockExitStatus != 0) {
        LERROR("error exiting BestAmp audio engine event loop");
        goto error;
    }
    
    // terminate portaudio
    Pa_Terminate();

    LINFO("exiting BestAmp audio engine normally");

    return 0;

error:
    // still need to terminate portaudio
    Pa_Terminate();
    LERROR("exiting BestAmp audio engine w/ error code %d", e);
    return e;
}
    
