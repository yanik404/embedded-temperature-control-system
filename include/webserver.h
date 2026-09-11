#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "system_types.h"

typedef void (*web_action_callback_t)(void);
typedef void (*web_setpoint_callback_t)(float value);
typedef void (*web_limits_callback_t)(float heating_percent, float cooling_percent, float max_temperature_c, float min_temperature_c);
typedef void (*web_peltier_test_callback_t)(uint8_t channel, bool cooling, uint8_t profile);
typedef void (*web_presentation_demo_callback_t)(bool cooling);

typedef struct {
    const system_status_t *status;
    web_action_callback_t start;
    web_action_callback_t stop;
    web_setpoint_callback_t set_cup_manual;
    web_action_callback_t rgb_test;
    web_action_callback_t workshop_override;
    web_peltier_test_callback_t peltier_test;
    web_presentation_demo_callback_t presentation_demo;
    web_setpoint_callback_t set_setpoint;
    web_limits_callback_t set_limits;
} webserver_config_t;

bool webserver_init(const webserver_config_t *config);
void webserver_deinit(void);
void webserver_update(void);
bool webserver_is_connected(void);
bool webserver_is_ready(void);
bool webserver_get_ip(char *buffer, size_t buffer_size);
