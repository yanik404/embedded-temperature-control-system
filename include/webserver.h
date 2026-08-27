#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "system_types.h"

typedef void (*web_action_callback_t)(void);
typedef void (*web_setpoint_callback_t)(float value);

typedef struct {
    const system_status_t *status;
    web_action_callback_t start;
    web_action_callback_t stop;
    web_action_callback_t rgb_test;
    web_setpoint_callback_t set_setpoint;
} webserver_config_t;

bool webserver_init(const webserver_config_t *config);
void webserver_deinit(void);
void webserver_update(void);
bool webserver_is_connected(void);
bool webserver_is_ready(void);
bool webserver_get_ip(char *buffer, size_t buffer_size);
