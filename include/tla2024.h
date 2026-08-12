#pragma once

#include <stdbool.h>
#include <stdint.h>

bool tla2024_init(void);
bool tla2024_read_voltage(uint8_t channel, float *voltage);
bool tla2024_is_available(void);
