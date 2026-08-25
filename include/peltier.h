#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PELTIER_CHANNEL_1 = 0,
    PELTIER_CHANNEL_2 = 1
} peltier_channel_t;

typedef enum {
    PELTIER_DIRECTION_HEAT = 0,
    PELTIER_DIRECTION_COOL
} peltier_direction_t;

void peltier_init(void);
void peltier_off(void);
/* Changing direction always forces both channels off and leaves output disabled. */
bool peltier_set_direction(peltier_direction_t direction);
peltier_direction_t peltier_get_direction(void);
void peltier_set_enabled(bool enabled);
void peltier_set_power(peltier_channel_t channel, uint8_t percent);
uint8_t peltier_get_power(peltier_channel_t channel);
