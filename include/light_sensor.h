#pragma once

#include <stdbool.h>

void light_sensor_init(void);
bool light_sensor_read(float *normalized_level);

