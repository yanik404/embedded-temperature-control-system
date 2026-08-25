#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "controller.h"

static void test_default_unipolar_output_limits(void) {
    pi_controller_t pi;
    controller_init(&pi, 8.0f, 0.12f);
    assert(controller_update(&pi, 40.0f, 50.0f, 0.25f) == 0.0f);
}

static void test_bidirectional_limits(void) {
    pi_controller_t pi;
    controller_init(&pi, 8.0f, 0.12f);
    controller_set_output_limits(&pi, -20.0f, 100.0f);
    assert(controller_update(&pi, 20.0f, 30.0f, 0.25f) == -20.0f);
    controller_reset(&pi);
    assert(controller_update(&pi, 60.0f, 20.0f, 0.25f) == 100.0f);
}

static void test_output_is_limited(void) {
    pi_controller_t pi;
    controller_init(&pi, 8.0f, 0.12f);
    assert(controller_update(&pi, 60.0f, 20.0f, 0.25f) == 100.0f);
}

static void test_anti_windup_recovers(void) {
    pi_controller_t pi;
    controller_init(&pi, 8.0f, 1.0f);
    for (int i = 0; i < 500; ++i) (void)controller_update(&pi, 60.0f, 20.0f, 0.25f);
    assert(pi.integral < 1.0f);
    assert(controller_update(&pi, 40.0f, 41.0f, 0.25f) == 0.0f);
}

static void test_cooling_anti_windup_recovers(void) {
    pi_controller_t pi;
    controller_init(&pi, 8.0f, 1.0f);
    controller_set_output_limits(&pi, -20.0f, 100.0f);
    for (int i = 0; i < 500; ++i) (void)controller_update(&pi, 20.0f, 40.0f, 0.25f);
    assert(pi.integral > -1.0f);
    assert(controller_update(&pi, 40.0f, 39.0f, 0.25f) > 0.0f);
}

static void test_integral_holds_small_error(void) {
    pi_controller_t pi;
    controller_init(&pi, 2.0f, 1.0f);
    float output = 0.0f;
    for (int i = 0; i < 20; ++i) output = controller_update(&pi, 45.0f, 44.5f, 0.25f);
    assert(output > 2.0f && output < 5.0f);
}

int main(void) {
    test_default_unipolar_output_limits();
    test_bidirectional_limits();
    test_output_is_limited();
    test_anti_windup_recovers();
    test_cooling_anti_windup_recovers();
    test_integral_holds_small_error();
    puts("controller tests passed");
    return 0;
}
