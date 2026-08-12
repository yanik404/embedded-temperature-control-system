#pragma once

#include <stdbool.h>

#include "system_types.h"

void status_leds_init(void);
void status_leds_update(system_state_t state, bool night_mode);

