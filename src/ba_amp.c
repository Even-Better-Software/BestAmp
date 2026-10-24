#include <stdio.h>
#include "ba_common.h"
#include "ba_amp.h"
#include "ba_dsp.h"


const ampVoice_t voices[] = {
    {  
        .name               = "default",
        .bassCornerFreq     = 4800.0f,
        .trebleCornerFreq   = 220.0f
    },
    {
        .name               = "test",
        .bassCornerFreq     = 5000.0f,
        .trebleCornerFreq   = 200.0f
    }
};


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
    a->voice = DEFAULT;
    biQuadFilter_init(&a->bqf_bass);
    biQuadFilter_init(&a->bqf_treble);
}


int testAmpValue(int value)
{
    if (value < BA_DIAL_MIN || value > BA_DIAL_MAX)
        return false;
    return true;
}


ampVoiceType_t itoavt(int raw) {
    switch (raw) {
        case (TEST): {
            return TEST;
        }
        default: {
            return DEFAULT;
        }
    }
}


void printAmpVoice(ampVoice_t* v)
{
    fprintf(stderr, "AmpVoice: {" \
        "name: %s, " \
        "bassCornerFreq: %f, " \
        "trebleCornerFreq: %f, "
    "}\n", v->name, v->bassCornerFreq, v->trebleCornerFreq);
}


void printAmpVoices()
{
    for (int i = 0; i < BA_AMP_VOICE_COUNT; i++) {
        ampVoice_t v = voices[i];
        fprintf(stderr, BA_INFO "%d ", i);
        printAmpVoice(&v);
    }
}


ampVoice_t getAmpVoice(ampVoiceType_t voiceTypeIdx)
{
    if (voiceTypeIdx <= 0 || voiceTypeIdx >= BA_AMP_VOICE_COUNT)
        return voices[DEFAULT];
    return voices[voiceTypeIdx];
}


void setAmpGain(ampValues_t* a, int gainValue)
{
    if (testAmpValue(gainValue)) {
        a->gain = gainValue;
    }
    else
        a->gain = BA_DIAL_MIN;
}


void setAmpBass(ampValues_t* a, int bassValue)
{
    if (testAmpValue(bassValue)) {
        a->bass = bassValue;
    }
    else
        a->bass = BA_DIAL_MIN;
}


void setAmpTreble(ampValues_t* a, int trebleValue)
{
    if (testAmpValue(trebleValue)) {
        a->treble = trebleValue;
    }
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

