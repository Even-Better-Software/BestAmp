#ifndef BA_APP
#define BA_APP

#include "ba_amp.h"
#include "ba_prefs.h"

/**
Container for application state.
    `baPrefs`
        should be a pointer to `ampValues_t`
    `ampValues`
        should be a pointer to `baPrefs_t`
*/
typedef struct BA_APP_INFO {
    ampValues_t ampValues;
    baPrefs_t baPrefs;
    PaStream* stream;
    int streamOpen;
    int streamRunning;
} appInfo_t;

#endif

