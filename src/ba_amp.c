#include <math.h>
#include <stdio.h>
#include "ba_common.h"
#include "ba_amp.h"
#include "ba_dsp.h"


const ampVoice_t voices[] = {
    {  
        .name               = "default",
        .bassCornerFreq     = 220.0f,
        .trebleCornerFreq   = 4800.0f,
        .maxGain_db         = 25.0f,
        .minGain_db         = 0.0f,
        .maxPower_db        = 0.0f,
        .minPower_db        = -60.0f,
        .bassRange_db       = 12.0f,
        .trebRange_db       = 12.0f
    },
    {
        .name               = "test",
        .bassCornerFreq     = 200.0f,
        .trebleCornerFreq   = 5000.0f,
        .maxGain_db         = 25.0f,
        .minGain_db         = 0.0f,
        .maxPower_db        = 0.0f,
        .minPower_db        = -60.0f,
        .bassRange_db       = 12.0f,
        .trebRange_db       = 12.0f
    }
};


void printAmpValues(ampValues_t* a)
{
    printf("AmpValues: { gain: %d, bass: %d, treble: %d, power: %d, voice: %d }\n",
        a->gain, a->bass, a->treble, a->power, a->voice); 
}


void initAmpValues(ampValues_t* a)
{
    a->gain = 1;
    a->bass = 1;
    a->treble = 1;
    a->power = 1;
    a->voice = DEFAULT;

    a->gainMult   = 1.0f;
    a->bassMult   = 1.0f;
    a->trebleMult = 1.0f;
    a->powerMult  = 1.0f;

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


void setAmpVoiceType(ampValues_t* a, int rawVoice)
{
    ampVoiceType_t voice = itoavt(rawVoice);
    a->voice = voice;
}


void setAmpGain(ampValues_t* a, int gainValue)
{
    if (testAmpValue(gainValue)) {
        a->gain = gainValue;
    }
    else {
        a->gain = BA_DIAL_MIN;
    }
    ampVoice_t v = getAmpVoice(a->voice);

    /*
    float n     = ( 1.0f * gainValue ) / BA_DIAL_MAX;
    a->gainDb   = v.minGain_db + ( n * ( v.maxGain_db - v.minGain_db ));
    */
    a->gainDb = BA_ITODB(gainValue, v.maxGain_db, v.minGain_db);
    a->gainMult = BA_VTOM(a->gainDb);
}


void setAmpBass(ampValues_t* a, int bassValue)
{
    if (testAmpValue(bassValue)) {
        a->bass = bassValue;
    }
    else {
        a->bass = BA_DIAL_MIN;
    }
    ampVoice_t v = getAmpVoice(a->voice);

    a->bassDb = BA_ITODB_RANGE(bassValue, v.bassRange_db);
    a->bassMult = BA_VTOM(a->bassDb);
}


void setAmpTreble(ampValues_t* a, int trebleValue)
{
    if (testAmpValue(trebleValue)) {
        a->treble = trebleValue;
    }
    else {
        a->treble = BA_DIAL_MIN;
    }
    ampVoice_t v = getAmpVoice(a->voice);

    a->trebleDb = BA_ITODB_RANGE(trebleValue, v.trebRange_db);
    a->trebleMult = BA_VTOM(a->trebleDb);
}


void setAmpPower(ampValues_t* a, int powerValue)
{
    if (testAmpValue(powerValue))
        a->power = powerValue;
    else {
        a->power = BA_DIAL_MIN;
    }
    ampVoice_t v = getAmpVoice(a->voice);

    a->powerDb = BA_ITODB(powerValue, v.maxPower_db, v.minPower_db);
    a->powerMult = BA_VTOM(a->powerDb);
}

