#include "status_leds.h"

#include "config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/time.h"
#include "status_led_logic.h"
#include "ws2812.pio.h"

#include <stddef.h>
#include <stdio.h>

#define RGB_LED_COUNT 12u

static PIO rgb_pio = pio0;
static uint rgb_sm;
static bool previous_heartbeat_on;
static bool previous_error_active;
static uint8_t previous_ring_red;
static uint8_t previous_ring_green;
static uint8_t previous_ring_blue;

static void active_low_led_set(uint pin, bool on) {
    gpio_put(pin, on ? 0 : 1);
}

static void ring_color(uint8_t red, uint8_t green, uint8_t blue) {
    const uint32_t color = ((uint32_t)green << 24u) | ((uint32_t)red << 16u) | ((uint32_t)blue << 8u);
    for (uint i = 0; i < RGB_LED_COUNT; ++i) pio_sm_put_blocking(rgb_pio, rgb_sm, color);
}

void status_leds_init(void) {
    const uint leds[] = {PIN_OUT_LED1_N, PIN_OUT_LED2_N, PIN_OUT_LED3_N, PIN_HB_LED_N, PIN_FB_LED_N};
    for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); ++i) {
        gpio_init(leds[i]);
        gpio_put(leds[i], 1);
        gpio_set_dir(leds[i], GPIO_OUT);
    }
    const uint offset = pio_add_program(rgb_pio, &ws2812_program);
    rgb_sm = pio_claim_unused_sm(rgb_pio, true);
    ws2812_program_init(rgb_pio, rgb_sm, offset, PIN_RGB_DIN, 800000.0f);
    ring_color(0, 0, 0);
    previous_heartbeat_on = false;
    previous_error_active = false;
    previous_ring_red = 0u;
    previous_ring_green = 0u;
    previous_ring_blue = 0u;
}

void status_leds_update(const system_status_t *status) {
    if (status == NULL) return;
    const bool blink_on = (to_ms_since_boot(get_absolute_time()) / 450u) % 2u == 0u;
    status_led_output_t output;
    status_led_logic_evaluate(status, blink_on, &output);

    active_low_led_set(PIN_OUT_LED1_N, output.ready_on);
    active_low_led_set(PIN_OUT_LED2_N, output.heating_on);
    active_low_led_set(PIN_OUT_LED3_N, output.holding_on);
    active_low_led_set(PIN_HB_LED_N, output.heartbeat_on);
    active_low_led_set(PIN_FB_LED_N, output.fault_on);
    if (output.ring_red != previous_ring_red || output.ring_green != previous_ring_green ||
        output.ring_blue != previous_ring_blue) {
        ring_color(output.ring_red, output.ring_green, output.ring_blue);
        previous_ring_red = output.ring_red;
        previous_ring_green = output.ring_green;
        previous_ring_blue = output.ring_blue;
    }

    if (output.heartbeat_on != previous_heartbeat_on) {
        printf("[LED] Webserver %s -> HB_LED %s\n",
               output.heartbeat_on ? "bereit" : "nicht bereit",
               output.heartbeat_on ? "EIN" : "AUS");
        previous_heartbeat_on = output.heartbeat_on;
    }

    const bool error_active = status->state == SYSTEM_ERROR || status->error != ERROR_NONE;
    if (error_active != previous_error_active) {
        printf("[LED] Fehler %s -> FB_LED %s\n",
               error_active ? "aktiv" : "nicht aktiv",
               error_active ? "blinkt" : "AUS");
        previous_error_active = error_active;
    }
}
