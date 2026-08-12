#pragma once

#include <stdbool.h>

typedef struct {
    float primary_c;
    float secondary_c;
    float process_c;
    bool primary_valid;
    bool secondary_valid;
    bool valid;
} temperature_reading_t;

void temperature_init(void);
temperature_reading_t temperature_read(void);

