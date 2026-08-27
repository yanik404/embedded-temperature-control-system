#pragma once

#include <stdbool.h>

#include "system_types.h"

void status_leds_init(void);
void status_leds_start_test(void);
void status_leds_update(const system_status_t *status);
