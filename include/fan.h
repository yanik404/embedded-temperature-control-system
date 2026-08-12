#pragma once

#include <stdint.h>

void fan_init(void);
void fan_set_speed(uint8_t percent);
uint8_t fan_get_speed(void);
uint16_t fan_get_rpm(void);
void fan_update(void);

