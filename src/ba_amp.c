#include <stdio.h>
#include "ba_common.h"
#include "ba_amp.h"

void printAmpValues(ampValues_t* a)
{
    printf("AmpValues: { gain: %d, bass: %d, treble: %d, power: %d }\n",
        a->gain, a->bass, a->treble, a->power); 
}

void initAmpValues(ampValues_t* a)
{
    a->gain = 1;
    a->bass = 1;
    a->treble = 1;
    a->power = 1;
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

