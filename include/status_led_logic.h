#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "system_types.h"

typedef struct {
    bool ready_on;
    bool heating_on;
    bool holding_on;
    bool heartbeat_on;
    bool fault_on;
    uint8_t ring_red;
    uint8_t ring_green;
    uint8_t ring_blue;
} status_led_output_t;

void status_led_logic_evaluate(const system_status_t *status, bool blink_on,
                               status_led_output_t *output);
