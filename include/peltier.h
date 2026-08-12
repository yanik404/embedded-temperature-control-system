#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PELTIER_CHANNEL_1 = 0,
    PELTIER_CHANNEL_2 = 1
} peltier_channel_t;

void peltier_init(void);
void peltier_off(void);
void peltier_set_enabled(bool enabled);
void peltier_set_power(peltier_channel_t channel, uint8_t percent);
uint8_t peltier_get_power(peltier_channel_t channel);

