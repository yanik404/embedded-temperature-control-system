#pragma once

#include <stdbool.h>

typedef struct {
    float channel_1_a;
    float channel_2_a;
    bool channel_1_valid;
    bool channel_2_valid;
    bool valid;
    bool overcurrent;
} current_reading_t;

void current_measurement_init(void);
current_reading_t current_measurement_read(void);
