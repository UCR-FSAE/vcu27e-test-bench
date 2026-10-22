#include <unity.h>

void setUp(void) {
    setupSerialBlocking()
    setupPins();
    setupCANBlocking();
    resetVCU();
}

void tearDown(void) {
    endSerialBlocking();
}

void test_rtd_sequence() {
    
}

void main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_rtd_sequence);
    
    UNITY_END();
}
