#ifndef BA_AMP
#define BA_AMP

#include "ba_dsp.h"


#define BA_DIAL_MIN (1)
#define BA_DIAL_MAX (10)


typedef enum BA_AMP_VOICES {
    DEFAULT,
    TEST
} ampVoiceType_t;

typedef struct BA_AMP_VOICE {
    char* name;
    float bassCornerFreq;
    float trebleCornerFreq;
} ampVoice_t;


#define BA_AMP_VOICE_COUNT (2)


typedef struct BA_AMP_VALUES {
    int gain;           // 1-10, as per my amp
    int bass;           // 1-10, as per my amp
    int treble;         // 1-10, as per my amp
    int power;          // 1-10, as per my amp
    ampVoiceType_t voice;
    biQuadFilter_t bqf_bass;
    biQuadFilter_t bqf_treble;
} ampValues_t;


// const extern char* ampVoiceNames[];
extern const ampVoice_t voices[];


/**
Initializes `ampValues_t` struct (gain, bass, treble, power) to 1.
*/
void initAmpValues(ampValues_t* a);


/**
print `ampValues_t` struct (to be used if the program is being debugged
from the terminal)
*/
void printAmpValues(ampValues_t* a);

/**
Test that a value is between BA_DIAL_MIN & BA_DIAL_MAX
*/
int testAmpValue(int value);


/**
Convert some integer value to a voice value.
    `raw` -> the raw integer value
    returns an `ampVoice_t` compliant enum
*/
ampVoiceType_t itoavt(int raw);

void printAmpVoices();

/**
Sets the value of the `gain` property of the passed `ampValues_t` `a` to be
the value of `gainValue` or 0 if `gainValue` fails `testAmpValue` check.
*/
void setAmpGain(ampValues_t* a, int gainValue);

/**
Sets the value of the `bass` property of the passed `ampValues_t` `a` to be the
value of `bassValue` or 0 if `bassValue` fails `testAmpValue` check.
*/
void setAmpBass(ampValues_t* a, int bassValue);

/**
Sets the value of the `treble` property of the passed `ampValues_t` `a` to be the
value of `trebleValue` or 0 of `trebleValue` fails `testAmpValue` check.
*/
void setAmpTreble(ampValues_t* a, int trebleValue);

/**
Sets the value of the `power` property of the passed `ampValues_t` `a` to be the
value of `powerValue` or 0 if `powerValue` failas `testAmpValue` check.
*/
void setAmpPower(ampValues_t* a, int powerValue);

#endif

