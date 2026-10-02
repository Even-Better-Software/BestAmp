#ifndef BA_PREFS
#define BA_PREFS

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


void printHostPrefs(hostPrefs_t* s);

/**
Set all host preferences to sentinel of `-1`
    This will tell the program to use the portaudio defaults for preferences
*/
void initHostPrefs(hostPrefs_t* s);

// [WO] at-least right now, these aren't really doing anything
// void setHostApiIdx(hostPrefs_t* s, int hostApiIndex);
// void setHostInDevIdx(hostPrefs_t* s, int inDevIdx);
// void setHostOutDevIdx(hostPrefs_t* s, int outDevIdx);


void printDevPrefs(devPrefs_t* d);

/**
Set all device preferences to sentinel of `-1`.
    This will tell the program to use the portaudio defaults for preferences
*/
void initDevPrefs(devPrefs_t* d);

// [WO] at-least right now, these aren't really doing anything
// void setDevInChanneln(devPrefs_t* d, int inChanneln);
// void setDevOutChanneln(devPrefs_t* d, int outChanneln);
// void setDevSampleRate(devPrefs_t* d, double sampleRate);
// void setDevFramesPerBuffer(devPrefs_t* d, int framesPerBuffer);

#endif
