#include <stdio.h>
#include <assert.h>
#include <ba_dsp.h>
#include <stdio.h>
#include <ba_tcp_messages.h>

void test_dsp_init() {
    biQuadFilter_t bf = {0};
    biQuadFilter_init(&bf);

    float data[3][2] = {{0, 0}, {1, 1}, {2.5, 2.5}};
    for (int i = 0; i < 3; i++) {
        float result = biQuadFilter_process(&bf, data[i][0]);
        assert(result == data[i][1]);        
    }
}

void test_dsp_high_shelf() {
    biQuadFilter_t bf = {0};
    biQuadFilter_init(&bf);

    biQuadFilter_highShelf(&bf, 40, 1, 0);

    assert(bf.a0 == 2);
    assert(bf.a1 == -2);
    assert(bf.a2 == 1);
    assert(bf.b0 == 100);
    assert(bf.b1 == -200);
    assert(bf.b2 == 100);
}

void test_dsp_low_shelf() {
    biQuadFilter_t bf = {0};
    biQuadFilter_init(&bf);

    biQuadFilter_lowShelf(&bf, 40, 1, 0);

    assert(bf.a0 == 20);
    assert(bf.a1 == -2);
    assert(bf.a2 == 1);
    assert(bf.b0 == 1);
    assert(bf.b1 == -2);
    assert(bf.b2 == 1);
}

void test_dsp_process() {
    biQuadFilter_t bf = {0};
    biQuadFilter_init(&bf);

    biQuadFilter_highShelf(&bf, 40, 1, 0);

    float data[3][2] = {{0, 0}, {1, 100}, {2.5, 250}};
    for (int i = 0; i < 3; i++) {
        float result = biQuadFilter_process(&bf, data[i][0]);
        printf("%f\n", result);
        assert(result == data[i][1]);        
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    test_dsp_init();
    test_dsp_high_shelf();
    test_dsp_low_shelf();
    test_dsp_process();

    printf("All tests passed!\n");
    
    return 0;
}
