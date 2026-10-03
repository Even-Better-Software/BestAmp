#include <math.h>

#include "ba_dsp.h"


void biQuadFilter_init(biQuadFilter_t* bf)
{
    bf->x1 = bf->x2 = bf->y1 = bf->y2 = 0.0f;
    bf->b0 = 1.0f;
    bf->a1 = bf->a2 = bf->b1 = bf->b2 = 0.0f;
}


float biQuadFilter_process(biQuadFilter_t* bf, float x)
{
    float y = (bf->b0*x) + (bf->b1 * bf->x1) + (bf->b2 * bf->x2)
            - (bf->a1 * bf->y1) - (bf->a2 * bf->y2);
    bf->x2 = bf->x1;
    bf->x1 = x;
    bf->y2 = bf->y1;
    bf->y1 = y;
    return y;
}

/**
[WO]  so I misunderstood what they were doing w/ a0 term, I still need
to calculate it & divide all the coeffecients by it in this initial
calculation, but by dividing it here I can treat it as 1 (therefore omit it)
during the process function.

but its late & I dont want to work on this anymore.
*/

void biQuadFilter_highShelf(biQuadFilter_t* bf, float dbGain, float sampleFreq)
{
    float A, w0, a, sinw0, cosw0;
    A       = powf(10, dbGain/40);
    w0      = TWOPI*(BA_TREB_CENTER_FREQ/sampleFreq);
    sinw0   = sinf(w0);
    cosw0   = cosf(w0);
    a       = (sinw0/2)*sqrtf(2); // I simplified this for S = 1
    
    // TODO
    bf->a0 = ;

    bf->b0 = ( A*((A+1)+(A-1)*cosw0+2*sqrtf(A)*a) );
    bf->b1 = ( -2*A*((A-1)+(A+1)*cosw0) );
    bf->b2 = ( A*((A+1)+(A-1)*cosw0-2*sqrtf(A)*a) );
    bf->a1 = ( 2*((A-1)-(A+1)*cosw0) );
    bf->a2 = ( (A+1)-(A-1)*cosw0-2*sqrtf(A)*a );
}


void biQuadFilter_lowShelf(biQuadFilter_t* bf, float dbGain, float sampleFreq)
{
    float A, w0, a, sinw0, cosw0;
    A       = powf(10, dbGain/40);
    w0      = TWOPI*(BA_BASS_CENTER_FREQ/sampleFreq);
    sinw0   = sinf(w0);
    cosw0   = cosf(w0);
    a       = (sinw0/2)*sqrtf(2); // I simplified this for S = 1
    
    // TODO
    bf->a0 = ;

    bf->b0 = A*((A+1)-(A-1)*cosw0+2*sqrtf(A)*a);
    bf->b1 = 2*A*((A-1)-(A+1)*cosw0);
    bf->b2 = A*((A+1)-(A-1)*cosw0-2*sqrtf(A)*a);
    bf->a1 = -2*((A-1)+(A+1)*cosw0);
    bf->a2 = (A+1)+(A-1)*cosw0-2*sqrtf(A)*a;
}

