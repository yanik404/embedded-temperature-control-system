#include "tla2024.h"

#include "config.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define TLA_REG_CONVERSION 0x00u
#define TLA_REG_CONFIG     0x01u

static bool available;

bool tla2024_init(void) {
    const uint8_t pointer = TLA_REG_CONFIG;
    uint8_t value[2];
    available = i2c_write_blocking(I2C_PORT, TLA2024_ADDRESS, &pointer, 1, true) == 1 &&
                i2c_read_blocking(I2C_PORT, TLA2024_ADDRESS, value, 2, false) == 2;
    return available;
}

bool tla2024_read_voltage(uint8_t channel, float *voltage) {
    if (channel > 3u || voltage == NULL) return false;

    /* Single-shot, AINx-GND, +/-4.096 V, 1600 SPS, comparator disabled. */
    const uint16_t config = (uint16_t)(0x8000u | ((0x4u + channel) << 12u) |
                                      (0x1u << 9u) | (0x1u << 8u) |
                                      (0x4u << 5u) | 0x0003u);
    const uint8_t command[] = {TLA_REG_CONFIG, (uint8_t)(config >> 8u), (uint8_t)config};
    if (i2c_write_blocking(I2C_PORT, TLA2024_ADDRESS, command, sizeof(command), false) !=
        (int)sizeof(command)) {
        available = false;
        return false;
    }

    sleep_us(900u);
    const uint8_t pointer = TLA_REG_CONVERSION;
    uint8_t raw_bytes[2];
    if (i2c_write_blocking(I2C_PORT, TLA2024_ADDRESS, &pointer, 1, true) != 1 ||
        i2c_read_blocking(I2C_PORT, TLA2024_ADDRESS, raw_bytes, 2, false) != 2) {
        available = false;
        return false;
    }

    const int16_t raw = (int16_t)((raw_bytes[0] << 8u) | raw_bytes[1]) >> 4u;
    available = true;
    *voltage = (float)raw * (4.096f / 2048.0f);
    return *voltage >= -0.01f && *voltage <= 3.6f;
}

bool tla2024_is_available(void) {
    return available;
}
