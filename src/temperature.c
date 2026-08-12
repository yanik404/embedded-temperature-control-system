#include "temperature.h"

#include <math.h>

#include "config.h"
#include "hardware/adc.h"
#include "tla2024.h"

static float filtered_primary;
static float filtered_secondary;
static bool primary_initialized;
static bool secondary_initialized;

static float tmp36_from_voltage(float voltage) {
    return (voltage - TMP36_OFFSET_V) / TMP36_VOLTS_PER_C;
}

static bool plausible(float voltage, float temperature) {
    return isfinite(voltage) && isfinite(temperature) && voltage > 0.05f &&
           voltage < ADC_REFERENCE_V - 0.05f && temperature >= SENSOR_MIN_C &&
           temperature <= SENSOR_MAX_C;
}

static float filter(float sample, float *state, bool *initialized) {
    if (!*initialized) {
        *state = sample;
        *initialized = true;
    } else {
        *state += TEMPERATURE_FILTER_ALPHA * (sample - *state);
    }
    return *state;
}

void temperature_init(void) {
    adc_init();
    adc_gpio_init(PIN_TEMP_1_ADC);
    primary_initialized = false;
    secondary_initialized = false;
}

temperature_reading_t temperature_read(void) {
    temperature_reading_t result = {0};
    adc_select_input(TEMP_1_ADC_CHANNEL);
    const uint16_t raw = adc_read();
    const float primary_voltage = (float)raw * ADC_REFERENCE_V / 4095.0f;
    const float primary = tmp36_from_voltage(primary_voltage);
    result.primary_valid = raw > 5u && raw < 4090u && plausible(primary_voltage, primary);
    if (result.primary_valid) result.primary_c = filter(primary, &filtered_primary, &primary_initialized);

    float secondary_voltage = 0.0f;
    const float secondary = tla2024_read_voltage(TLA_CH_TEMP_2, &secondary_voltage)
                                ? tmp36_from_voltage(secondary_voltage) : 0.0f;
    result.secondary_valid = plausible(secondary_voltage, secondary);
    if (result.secondary_valid) {
        result.secondary_c = filter(secondary, &filtered_secondary, &secondary_initialized);
    }

    /* Both sensors are required; controlling the warmer point is conservative. */
    result.valid = result.primary_valid && result.secondary_valid &&
                   fabsf(result.primary_c - result.secondary_c) < 20.0f;
    result.process_c = result.primary_c > result.secondary_c ? result.primary_c : result.secondary_c;
    return result;
}

