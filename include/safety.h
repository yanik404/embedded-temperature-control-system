#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "system_types.h"

void safety_init(void);
error_code_t safety_check(const system_status_t *status, uint32_t thermal_run_elapsed_ms);
bool safety_can_start(const system_status_t *status);
