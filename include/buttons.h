#pragma once

#include <stdbool.h>

typedef enum {
    BUTTON_MODE = 0,
    BUTTON_DOWN,
    BUTTON_OK,
    BUTTON_UP,
    BUTTON_COUNT
} button_id_t;

void buttons_init(void);
void buttons_update(void);
bool button_was_pressed(button_id_t button);
bool buttons_cup_detected(void);
bool buttons_cup_raw_level(void);
