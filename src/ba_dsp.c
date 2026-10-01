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

