#ifndef BA_PREFS
#define BA_PREFS


#include <portaudio.h>

#define BA_PA_ERROR_LOG "%s:%d \e[31mPortAudio error\e[0m: %s\n"
#define BA_NULL_HOST_ERROR_LOG "%s:%d \e[31mNull host\e[0m: index=%d\n"
#define BA_NULL_DEVICE_ERROR_LOG "%s:%d \e[31mNull device\e[0m: index=%d\n"


/*
typedef struct BA_HOST_PREFS {
    int hostApiIdx;
    int inDevIdx;
    int outDevIdx;
} hostPrefs_t;


typedef struct BA_DEVICE_PREFS {
    int inChanneln; 
    int outChanneln;
    double sampleRate;
    int framesPerBuffer;
    long sampleFormat;  // I would almost like to be able to cast to the thing at this
} devPrefs_t;           // determine if you can store 'types' in variables (seems like a far fetched thing)
*/

typedef struct BA_PREFS {
    int hostApiIdx;
    int inDevIdx;
    int outDevIdx;
    int inChanneln;
    int outChanneln;
    float sampleRate;
    int framesPerBuffer;
    long sampleFormat;  // I would almost like to be able to cast to the thing at this
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


// void printHostPrefs(hostPrefs_t* s);

/**
Set all host preferences to sentinel of `-1`
    This will tell the program to use the portaudio defaults for preferences
*/
// void initHostPrefs(hostPrefs_t* s);

// [WO] at-least right now, these aren't really doing anything
// void setHostApiIdx(hostPrefs_t* s, int hostApiIndex);
// void setHostInDevIdx(hostPrefs_t* s, int inDevIdx);
// void setHostOutDevIdx(hostPrefs_t* s, int outDevIdx);


// void printDevPrefs(devPrefs_t* d);

/**
Set all device preferences to sentinel of `-1`.
    This will tell the program to use the portaudio defaults for preferences
*/
// void initDevPrefs(devPrefs_t* d);

// [WO] at-least right now, these aren't really doing anything
// void setDevInChanneln(devPrefs_t* d, int inChanneln);
// void setDevOutChanneln(devPrefs_t* d, int outChanneln);
// void setDevSampleRate(devPrefs_t* d, double sampleRate);
// void setDevFramesPerBuffer(devPrefs_t* d, int framesPerBuffer);

#endif
