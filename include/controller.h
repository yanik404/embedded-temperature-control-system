#pragma once

typedef struct {
    float kp;
    float ki;
    float integral;
    float output_min;
    float output_max;
} pi_controller_t;

void controller_init(pi_controller_t *controller, float kp, float ki);
void controller_set_output_limits(pi_controller_t *controller, float output_min, float output_max);
void controller_reset(pi_controller_t *controller);
float controller_update(pi_controller_t *controller, float setpoint, float measured, float dt_seconds);
