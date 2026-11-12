#ifndef BA_PREFS
#define BA_PREFS


#include <stdint.h>
#include <stdio.h>
#include <portaudio.h>

#include "ba_common.h"

typedef struct BA_PREFS {
    int32_t hostApiIdx;
    int32_t inDevIdx;
    int32_t outDevIdx;
    int32_t inChanneln;
    int32_t outChanneln;
    int32_t framesPerBuffer;
    int32_t sampleFormat;
    double sampleRate;
    PaHostApiInfo* hostApi;
    PaDeviceInfo* outDev;
    PaDeviceInfo* inDev;
} baPrefs_t;


/**
Prints `baPrefs_t` struct values to STDOUT.
*/
void printBaPrefs(baPrefs_t* prefs);

/**
A string representation of the `baPrefs_t` type.
    To be used in other more robust logging or maybe messages.
*/
// char* baPrefsStr(baPrefs_t* prefs);

/**
Initializes ba prefs with all fields set to -1 (or types equivalent)
    so that main processing can coalesce unset preferences to defaults
*/
void initBaPrefs(baPrefs_t* prefs);

/**
Initializes ba prefs struct to have system defaults.
    MUST be called after portaudio has been initalized.

    will write portaudio errors to stdout
    
    returns -1 if underlying calls to portaudio fail
*/
int initBaPrefsToPaDefaults(baPrefs_t* prefs);

/**
Initializes fields that hold the no value sentinel of `-1` to portaudio defaults.
    MUST be called after portaudio has been initialized.

    Surfaces the PaError value, so perform the same check that you would
    if you were invoking a portaudio function.

    ie. if (initBaPrefsToPaDefaults(&b) != paNoError) { // handle error }.
*/
int coalesceBaPrefsToPaDefaults(baPrefs_t* prefs);

#endif

