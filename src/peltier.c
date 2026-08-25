#include "peltier.h"

#include "config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#include <stddef.h>

static bool output_enabled;
static uint8_t requested_power[2];
static peltier_direction_t active_direction;

static uint direction_ina_pin(peltier_channel_t channel) {
    return channel == PELTIER_CHANNEL_1 ? PIN_PEL1_INA : PIN_PEL2_INA;
}

static uint direction_inb_pin(peltier_channel_t channel) {
    return channel == PELTIER_CHANNEL_1 ? PIN_PEL1_INB : PIN_PEL2_INB;
}

static uint sense_select_pin(peltier_channel_t channel) {
    return channel == PELTIER_CHANNEL_1 ? PIN_PEL1_SEL : PIN_PEL2_SEL;
}

static void set_direction_inputs(peltier_channel_t channel, peltier_direction_t direction) {
    const bool heating = direction == PELTIER_DIRECTION_HEAT;
    gpio_put(direction_ina_pin(channel), heating ? PELTIER_HEAT_INA_LEVEL : PELTIER_COOL_INA_LEVEL);
    gpio_put(direction_inb_pin(channel), heating ? PELTIER_HEAT_INB_LEVEL : PELTIER_COOL_INB_LEVEL);
    gpio_put(sense_select_pin(channel), heating ? PELTIER_HEAT_SEL_LEVEL : PELTIER_COOL_SEL_LEVEL);
}

static void configure_pwm_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    const uint slice = pwm_gpio_to_slice_num(pin);
    pwm_config cfg = pwm_get_default_config();
    const float divider = (float)clock_get_hz(clk_sys) / (PELTIER_PWM_HZ * 1000.0f);
    pwm_config_set_clkdiv(&cfg, divider);
    pwm_config_set_wrap(&cfg, 999u);
    pwm_init(slice, &cfg, true);
    pwm_set_gpio_level(pin, 0u);
}

static void apply_channel(peltier_channel_t channel) {
    const uint pwm_pin = channel == PELTIER_CHANNEL_1 ? PIN_PEL1_PWM : PIN_PEL2_PWM;
    const uint8_t power = output_enabled ? requested_power[channel] : 0u;

    if (!output_enabled) {
        pwm_set_gpio_level(pwm_pin, 0u);
        gpio_put(direction_ina_pin(channel), 0);
        gpio_put(direction_inb_pin(channel), 0);
        gpio_put(sense_select_pin(channel), 0);
        return;
    }

    /* Keep the selected direction asserted at zero duty so a later PWM edge
       never wakes the bridge and drives it in the same instruction sequence. */
    set_direction_inputs(channel, active_direction);
    if (power == 0u) {
        pwm_set_gpio_level(pwm_pin, 0u);
        return;
    }
    pwm_set_gpio_level(pwm_pin, (uint16_t)power * 10u);
}

void peltier_init(void) {
    output_enabled = false;
    requested_power[0] = 0u;
    requested_power[1] = 0u;
    active_direction = PELTIER_DIRECTION_HEAT;

    const uint direction_pins[] = {PIN_PEL1_INA, PIN_PEL1_INB, PIN_PEL2_INA, PIN_PEL2_INB};
    for (size_t i = 0; i < sizeof(direction_pins) / sizeof(direction_pins[0]); ++i) {
        gpio_init(direction_pins[i]);
        gpio_put(direction_pins[i], 0);
        gpio_set_dir(direction_pins[i], GPIO_OUT);
    }
    gpio_init(PIN_PEL1_SEL);
    gpio_put(PIN_PEL1_SEL, 0);
    gpio_set_dir(PIN_PEL1_SEL, GPIO_OUT);
    gpio_init(PIN_PEL2_SEL);
    gpio_put(PIN_PEL2_SEL, 0);
    gpio_set_dir(PIN_PEL2_SEL, GPIO_OUT);

    configure_pwm_pin(PIN_PEL1_PWM);
    configure_pwm_pin(PIN_PEL2_PWM);
    peltier_off();
}

void peltier_off(void) {
    output_enabled = false;
    requested_power[0] = 0u;
    requested_power[1] = 0u;
    apply_channel(PELTIER_CHANNEL_1);
    apply_channel(PELTIER_CHANNEL_2);
}

bool peltier_set_direction(peltier_direction_t direction) {
    if (direction != PELTIER_DIRECTION_HEAT && direction != PELTIER_DIRECTION_COOL) return false;

    /* Break-before-make: both PWM signals and all direction inputs are forced
       low before the polarity changes. The caller must explicitly re-enable. */
    peltier_off();
    sleep_ms(PELTIER_DIRECTION_DEADTIME_MS);
    active_direction = direction;
    return true;
}

peltier_direction_t peltier_get_direction(void) {
    return active_direction;
}

void peltier_set_enabled(bool enabled) {
    output_enabled = enabled;
    apply_channel(PELTIER_CHANNEL_1);
    apply_channel(PELTIER_CHANNEL_2);
    if (enabled) sleep_us(PELTIER_WAKE_DELAY_US);
}

void peltier_set_power(peltier_channel_t channel, uint8_t percent) {
    if (channel > PELTIER_CHANNEL_2) return;
    requested_power[channel] = percent > 100u ? 100u : percent;
    apply_channel(channel);
}

uint8_t peltier_get_power(peltier_channel_t channel) {
    if (channel > PELTIER_CHANNEL_2 || !output_enabled) return 0u;
    return requested_power[channel];
}
