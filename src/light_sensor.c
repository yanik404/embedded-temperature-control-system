#include "light_sensor.h"

#include "config.h"
#include "tla2024.h"

void light_sensor_init(void) {
}

bool light_sensor_read(float *normalized_level) {
    float voltage;
    if (normalized_level == NULL || !tla2024_read_voltage(TLA_CH_LIGHT, &voltage)) return false;
    float level = voltage / ADC_REFERENCE_V;
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    *normalized_level = level;
    return true;
}
