#include "fan.h"

#include "config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"

static volatile uint32_t tach_pulses;
static uint32_t last_sample_ms;
static uint16_t rpm;
static uint8_t speed_percent;

static void tach_callback(uint gpio, uint32_t events) {
    (void)gpio;
    (void)events;
    ++tach_pulses;
}

void fan_init(void) {
    speed_percent = 0u;
    rpm = 0u;
    tach_pulses = 0u;
    last_sample_ms = to_ms_since_boot(get_absolute_time());

    gpio_init(PIN_FAN_CTRL);
    gpio_put(PIN_FAN_CTRL, 0);
    gpio_set_dir(PIN_FAN_CTRL, GPIO_OUT);

    gpio_set_function(PIN_FAN_PWM, GPIO_FUNC_PWM);
    const uint slice = pwm_gpio_to_slice_num(PIN_FAN_PWM);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, (float)clock_get_hz(clk_sys) / (FAN_PWM_HZ * 1000.0f));
    pwm_config_set_wrap(&cfg, 999u);
    pwm_init(slice, &cfg, true);
    pwm_set_gpio_level(PIN_FAN_PWM, 0u);

    gpio_init(PIN_FAN_TACHO);
    gpio_set_dir(PIN_FAN_TACHO, GPIO_IN);
    gpio_pull_up(PIN_FAN_TACHO);
    gpio_set_irq_enabled_with_callback(PIN_FAN_TACHO, GPIO_IRQ_EDGE_FALL, true, tach_callback);
}

void fan_set_speed(uint8_t percent) {
    speed_percent = percent > 100u ? 100u : percent;
    if (speed_percent == 0u) {
        pwm_set_gpio_level(PIN_FAN_PWM, 0u);
        gpio_put(PIN_FAN_CTRL, 0);
    } else {
        gpio_put(PIN_FAN_CTRL, 1);
        pwm_set_gpio_level(PIN_FAN_PWM, (uint16_t)speed_percent * 10u);
    }
}

uint8_t fan_get_speed(void) {
    return speed_percent;
}

uint16_t fan_get_rpm(void) {
    return rpm;
}

void fan_update(void) {
    const uint32_t time = to_ms_since_boot(get_absolute_time());
    const uint32_t elapsed = time - last_sample_ms;
    if (elapsed < 1000u) return;
    const uint32_t pulses = tach_pulses;
    tach_pulses = 0u;
    last_sample_ms = time;
    const uint32_t calculated = pulses * 60000u / (elapsed * FAN_TACH_PULSES_PER_REV);
    rpm = calculated > 65535u ? 65535u : (uint16_t)calculated;
}

