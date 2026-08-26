#include "buttons.h"

#include <stdio.h>

#include "config.h"
#include "cup_detector.h"
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
static cup_detector_t cup_detector;
static bool cup_raw_level;

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
#if CUP_DETECT_ACTIVE_LEVEL
    gpio_pull_down(PIN_S_DETECT);
#else
    gpio_pull_up(PIN_S_DETECT);
#endif
    cup_raw_level = gpio_get(PIN_S_DETECT);
    cup_detector_init(&cup_detector, cup_raw_level, CUP_DETECT_ACTIVE_LEVEL != 0u, now_ms());
    printf("[DETECT] Initialisierung: GP13 raw=%u, aktiv=%u, Becherpruefung laeuft\n",
           cup_raw_level ? 1u : 0u, CUP_DETECT_ACTIVE_LEVEL);
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
    const bool raw_level = gpio_get(PIN_S_DETECT);
    if (raw_level != cup_raw_level) {
        cup_raw_level = raw_level;
        printf("[DETECT] GP13 raw=%u -> %s Kandidat\n", raw_level ? 1u : 0u,
               raw_level == (CUP_DETECT_ACTIVE_LEVEL != 0u) ? "Becher erkannt" : "Becher frei");
    }
    if (cup_detector_update(&cup_detector, raw_level, time, CUP_DETECT_INSERT_MS,
                            CUP_DETECT_REMOVE_MS)) {
        printf("[DETECT] Stabil: %s (GP13 raw=%u)\n",
               cup_detector_is_present(&cup_detector) ? "Becher erkannt" : "Becher entfernt",
               raw_level ? 1u : 0u);
    }
}

bool button_was_pressed(button_id_t button) {
    if (button >= BUTTON_COUNT) return false;
    const bool event = states[button].pressed_event;
    states[button].pressed_event = false;
    return event;
}

bool buttons_cup_detected(void) {
    return cup_detector_is_present(&cup_detector);
}

bool buttons_cup_raw_level(void) {
    return cup_raw_level;
}
