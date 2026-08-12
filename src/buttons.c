#include "buttons.h"

#include "config.h"
#include "hardware/gpio.h"
#include "pico/time.h"

typedef struct {
    bool raw;
    bool stable;
    bool pressed_event;
    uint32_t changed_ms;
} debounce_t;

static debounce_t states[BUTTON_COUNT];
static const uint pins[BUTTON_COUNT] = {PIN_S_MODE, PIN_S_DOWN, PIN_S_OK, PIN_S_UP};

static uint32_t now_ms(void) {
    return to_ms_since_boot(get_absolute_time());
}

void buttons_init(void) {
    for (uint i = 0; i < BUTTON_COUNT; ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
        states[i].raw = !gpio_get(pins[i]);
        states[i].stable = states[i].raw;
        states[i].pressed_event = false;
        states[i].changed_ms = now_ms();
    }
    gpio_init(PIN_S_DETECT);
    gpio_set_dir(PIN_S_DETECT, GPIO_IN);
    gpio_pull_up(PIN_S_DETECT);
}

void buttons_update(void) {
    const uint32_t time = now_ms();
    for (uint i = 0; i < BUTTON_COUNT; ++i) {
        const bool pressed = !gpio_get(pins[i]);
        if (pressed != states[i].raw) {
            states[i].raw = pressed;
            states[i].changed_ms = time;
        } else if (pressed != states[i].stable && time - states[i].changed_ms >= BUTTON_DEBOUNCE_MS) {
            states[i].stable = pressed;
            if (pressed) states[i].pressed_event = true;
        }
    }
}

bool button_was_pressed(button_id_t button) {
    if (button >= BUTTON_COUNT) return false;
    const bool event = states[button].pressed_event;
    states[button].pressed_event = false;
    return event;
}

bool buttons_cup_detected(void) {
    /* S_DETECT is an active-low PCB switch/input. */
    return !gpio_get(PIN_S_DETECT);
}

