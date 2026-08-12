#include "status_leds.h"

#include "config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/time.h"
#include "ws2812.pio.h"

#include <stddef.h>

#define RGB_LED_COUNT 12u

static PIO rgb_pio = pio0;
static uint rgb_sm;

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
}

void status_leds_update(system_state_t state, bool night_mode) {
    const bool blink = (to_ms_since_boot(get_absolute_time()) / 400u) % 2u;
    uint8_t scale = night_mode ? 5u : 24u;
    uint8_t red = 0u, green = 0u, blue = 0u;
    gpio_put(PIN_OUT_LED1_N, 1);
    gpio_put(PIN_OUT_LED2_N, 1);
    gpio_put(PIN_OUT_LED3_N, 1);
    if (state == SYSTEM_READY) { blue = scale; gpio_put(PIN_OUT_LED1_N, 0); }
    if (state == SYSTEM_HEATING) { red = scale; green = scale / 4u; gpio_put(PIN_OUT_LED2_N, 0); }
    if (state == SYSTEM_HOLDING) { green = scale; gpio_put(PIN_OUT_LED3_N, 0); }
    if (state == SYSTEM_ERROR && blink) { red = scale; gpio_put(PIN_OUT_LED1_N, 0); gpio_put(PIN_OUT_LED3_N, 0); }
    ring_color(red, green, blue);
}
