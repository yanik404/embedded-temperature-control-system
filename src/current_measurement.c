#include "current_measurement.h"

#include <math.h>

#include "config.h"
#include "tla2024.h"

void current_measurement_init(void) {
}

current_reading_t current_measurement_read(void) {
    current_reading_t reading = {0};
    float voltage_1 = 0.0f;
    float voltage_2 = 0.0f;
    const bool ok_1 = tla2024_read_voltage(TLA_CH_PEL1_CS, &voltage_1);
    const bool ok_2 = tla2024_read_voltage(TLA_CH_PEL2_CS, &voltage_2);
    reading.channel_1_a = (voltage_1 - CURRENT_ZERO_V) * CURRENT_AMPS_PER_VOLT;
    reading.channel_2_a = (voltage_2 - CURRENT_ZERO_V) * CURRENT_AMPS_PER_VOLT;
    reading.valid = ok_1 && ok_2 && isfinite(reading.channel_1_a) && isfinite(reading.channel_2_a) &&
                    reading.channel_1_a >= CURRENT_PLAUSIBLE_MIN_A &&
                    reading.channel_2_a >= CURRENT_PLAUSIBLE_MIN_A &&
                    reading.channel_1_a <= CURRENT_PLAUSIBLE_MAX_A &&
                    reading.channel_2_a <= CURRENT_PLAUSIBLE_MAX_A;
    reading.overcurrent = reading.channel_1_a > CURRENT_MAX_A || reading.channel_2_a > CURRENT_MAX_A;
    return reading;
}

