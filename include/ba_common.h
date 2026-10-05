#ifndef BA_COMMON
#define BA_COMMON

#include <portaudio.h>

#include "ba_amp.h"
#include "ba_prefs.h"


#define true (1)
#define false (0)

#define BA_INFO    "\e[32m[INFO]\e[0m "
#define BA_ERROR   "\e[31m[ERROR]\e[0m "

/**
Container for application state.
    `baPrefs`
        should be a pointer to `ampValues_t`
    `ampValues`
        should be a pointer to `baPrefs_t`
*/
typedef struct BA_APP_INFO {
    ampValues_t ampValues;     // [WO] why pointer?
    baPrefs_t baPrefs;         // [WO] why pointer?
    PaStream* stream;
    int streamOpen;
    int streamRunning;
} appInfo_t;

#endif

