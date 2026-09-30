#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <pa_asio.h>


#define true    (1)
#define false   (0)


#define BA_NAME "BestAmp.exe"
#define BA_HELP BA_NAME \
" program useage:\n" \
"\t" BA_NAME " <GAIN> <BASS> <TREBLE> <POWER>\n" \
"\t\twhere GAIN,BASS,TREBLE,& POWER are numbers (integers) " \
"that are in the range 1-10, if a value is outside this range " \
"it will be set to 1 (the minimum)\n"


#define BA_EXPECTED_ARGS (5)
#define BA_DIAL_MIN (1)
#define BA_DIAL_MAX (10)


// [WO] dont know that we really need such a construct
// struct CMD_ARGS {
// } typedef cmdArgs_t;


typedef struct BA_AMP_VALUES {
    int gain;           // 1-10, as per my amp
    int bass;           // 1-10, as per my amp
    int treble;         // 1-10, as per my amp
    int power;          // 1-10, as per my amp
} ampValues_t;


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




/**
Program Entry
*/
int main(int argc, char** argv)
{
    // parse command line arguments
    // figure out how many there are
    // then assign values as follows
    // GAIN BASS TREBLE POWER ...
    // resolving to 0 if there is no value (so it will get the default, which is the min)
    if (argc != BA_EXPECTED_ARGS)
        goto error;

    // initialize the `amp` instance of the `ampValues_t` struct
    // will be passed as the user data to PortAudio callback
    ampValues_t amp = { 0 };

    setAmpGain(&amp, atoi(argv[1]));    // gain cmd arg
    setAmpBass(&amp, atoi(argv[2]));    // bass cmd arg
    setAmpTreble(&amp, atoi(argv[3]));  // treble cmd arg
    setAmpPower(&amp, atoi(argv[4]));   // power cmd arg
    
    // show the values
    printAmpValues(&amp);

    return 0;

error:
    printf(BA_HELP);
    return 1;
}


void printAmpValues(ampValues_t* a)
{
    printf("AmpValues: { gain: %d, bass: %d, treble: %d, power: %d }\n",
        a->gain, a->bass, a->treble, a->power); 
}


int testAmpValue(int value)
{
    if (value < BA_DIAL_MIN || value > BA_DIAL_MAX)
        return false;
    return true;
}


void setAmpGain(ampValues_t* a, int gainValue)
{
    if (testAmpValue(gainValue))
        a->gain = gainValue;
    else
        a->gain = BA_DIAL_MIN;
}

void setAmpBass(ampValues_t* a, int bassValue)
{
    if (testAmpValue(bassValue))
        a->bass = bassValue;
    else
        a->bass = BA_DIAL_MIN;
}

void setAmpTreble(ampValues_t* a, int trebleValue)
{
    if (testAmpValue(trebleValue))
        a->treble = trebleValue;
    else
        a->treble = BA_DIAL_MIN;
}

void setAmpPower(ampValues_t* a, int powerValue)
{
    if (testAmpValue(powerValue))
        a->power = powerValue;
    else
        a->power = BA_DIAL_MIN;
}

