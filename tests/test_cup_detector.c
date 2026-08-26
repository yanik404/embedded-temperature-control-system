#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cup_detector.h"

static void test_active_low_with_asymmetric_debounce(void) {
    cup_detector_t detector;
    cup_detector_init(&detector, true, false, 0u);
    assert(!cup_detector_is_present(&detector));

    assert(!cup_detector_update(&detector, false, 10u, 100u, 50u));
    assert(!cup_detector_update(&detector, false, 109u, 100u, 50u));
    assert(cup_detector_update(&detector, false, 110u, 100u, 50u));
    assert(cup_detector_is_present(&detector));

    /* A short release and renewed contact are treated as switch bounce. */
    assert(!cup_detector_update(&detector, true, 200u, 100u, 50u));
    assert(!cup_detector_update(&detector, false, 225u, 100u, 50u));
    assert(cup_detector_is_present(&detector));

    assert(!cup_detector_update(&detector, true, 240u, 100u, 50u));
    assert(!cup_detector_update(&detector, true, 289u, 100u, 50u));
    assert(cup_detector_update(&detector, true, 290u, 100u, 50u));
    assert(!cup_detector_is_present(&detector));
}

static void test_active_high_and_time_wrap(void) {
    cup_detector_t detector;
    const uint32_t start = UINT32_MAX - 20u;
    cup_detector_init(&detector, false, true, start);
    assert(!cup_detector_update(&detector, true, start + 5u, 30u, 10u));
    assert(!cup_detector_update(&detector, true, 13u, 30u, 10u));
    assert(cup_detector_update(&detector, true, 14u, 30u, 10u));
    assert(cup_detector_is_present(&detector));
}

int main(void) {
    test_active_low_with_asymmetric_debounce();
    test_active_high_and_time_wrap();
    puts("cup detector tests passed");
    return 0;
}
