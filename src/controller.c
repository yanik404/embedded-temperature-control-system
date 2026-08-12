#include "controller.h"

static float clampf(float value, float minimum, float maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

void controller_init(pi_controller_t *controller, float kp, float ki) {
    controller->kp = kp;
    controller->ki = ki;
    controller->integral = 0.0f;
    controller->output_min = 0.0f;
    controller->output_max = 100.0f;
}

void controller_reset(pi_controller_t *controller) {
    controller->integral = 0.0f;
}

float controller_update(pi_controller_t *controller, float setpoint, float measured, float dt_seconds) {
    const float error = setpoint - measured;
    const float proportional = controller->kp * error;
    const float proposed_integral = controller->integral + controller->ki * error * dt_seconds;
    const float proposed_output = proportional + proposed_integral;

    /* Conditional integration prevents windup at either output rail. */
    if ((proposed_output < controller->output_max || error < 0.0f) &&
        (proposed_output > controller->output_min || error > 0.0f)) {
        controller->integral = proposed_integral;
    }

    return clampf(proportional + controller->integral,
                  controller->output_min, controller->output_max);
}

