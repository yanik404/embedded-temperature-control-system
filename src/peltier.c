#include "peltier.h"

#include "config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include <stddef.h>

static bool output_enabled;
static uint8_t requested_power[2];

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
    const uint ina_pin = channel == PELTIER_CHANNEL_1 ? PIN_PEL1_INA : PIN_PEL2_INA;
    const uint inb_pin = channel == PELTIER_CHANNEL_1 ? PIN_PEL1_INB : PIN_PEL2_INB;
    const uint8_t power = output_enabled ? requested_power[channel] : 0u;

    if (power == 0u) {
        pwm_set_gpio_level(pwm_pin, 0u);
        gpio_put(ina_pin, 0);
        gpio_put(inb_pin, 0);
        return;
    }
    gpio_put(ina_pin, PELTIER_HEAT_INA_LEVEL);
    gpio_put(inb_pin, PELTIER_HEAT_INB_LEVEL);
    pwm_set_gpio_level(pwm_pin, (uint16_t)power * 10u);
}

void peltier_init(void) {
    output_enabled = false;
    requested_power[0] = 0u;
    requested_power[1] = 0u;

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

void peltier_set_enabled(bool enabled) {
    output_enabled = enabled;
    apply_channel(PELTIER_CHANNEL_1);
    apply_channel(PELTIER_CHANNEL_2);
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
