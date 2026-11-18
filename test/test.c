#include <stdio.h>
#include <assert.h>
#include <ba_dsp.h>
#include <stdio.h>

void test_dsp_init() {
    biQuadFilter_t bq = {0};
    biQuadFilter_init(&bq);

    float data[3][2] = {{0, 0}, {1, 1}, {2.5, 2.5}};
    for (int i = 0; i < 3; i++) {
        float result = biQuadFilter_process(&bq, data[i][0]);
        assert(result == data[i][1]);        
    }
}
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    test_dsp_init();

    printf("All tests passed\n");
    
    return 0;
}
