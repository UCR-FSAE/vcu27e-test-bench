#include <unity.h>

void setUp(void) {
    setupSerialBlocking()
    setupPins();
}

void tearDown(void) {
    
}

void test_rtd_sequence() {
    
}

void main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_rtd_sequence);
    
    UNITY_END();
}
