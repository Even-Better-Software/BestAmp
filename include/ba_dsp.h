#ifndef BA_DSP
#define BA_DSP

/**
Sources:
https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

provides an explanation for derived functions


impl notes

where a0 = 1
& b0 = 1

if you are comparing the equations in the cookbook to the implementations
in `ba_dsp.c` you will notice that the value of 

continue w/ derivations based off these coeffecient normalizations
*/

#define BA_CENTER_FREQ (200)            // apparently a common value for this
#define TWOPI (2*3.14158265359)         // probably enough precision but idk


typedef struct BA_BIQUAD_FILTER {
    float x1, x2, y1, y2;               // temp, n - 1, n - 2
    float a0, a1, a2, b0, b1, b2;
} biQuadFilter_t;


/**
Initialize a biquad filter.
    all values to zero except b1
*/
void biQuadFilter_init(biQuadFilter_t* bf);

/**
Produce y[n], given x[n] using current values of the coeffecients
for the specified biQuad.

    Set x1 -> x2
    & set current x as x1

    Set y1 -> y2
    & set current y as x1
    
    Note: in the implementation a0 as a denominator is omitted because thats
    just division by 1
*/
float biQuadFilter_process(biQuadFilter_t* bf, float x);

/**
To be called when amp bass dial is modified.
    re-computes highShelf biQuadFilter when `dbGain` has been modified by the user
*/
void biQuadFilter_highShelf(biQuadFilter_t* bf,
                            float dbGain, float sampleFreq);

/**
To be called when amp treble dial is modified.
    re-computes lowShelf biQuadFilter when `dbGain` has been modified by the user
*/
void biQuadFilter_lowShelf( biQuadFilter_t* bf,
                            float dbGain, float sampleFreq);


#endif

