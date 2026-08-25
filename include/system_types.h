#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SYSTEM_OFF = 0,
    SYSTEM_READY,
    SYSTEM_HEATING,
    SYSTEM_COOLING,
    SYSTEM_HOLDING,
    SYSTEM_ERROR
} system_state_t;

typedef enum {
    ERROR_NONE = 0,
    ERROR_TEMP_SENSOR,
    ERROR_OVERTEMPERATURE,
    ERROR_UNDERTEMPERATURE,
    ERROR_FAN,
    ERROR_OVERCURRENT,
    ERROR_CURRENT_SENSOR,
    ERROR_CUP_REMOVED,
    ERROR_POWER_SUPPLY
} error_code_t;

typedef enum {
    THERMAL_OUTPUT_OFF = 0,
    THERMAL_OUTPUT_HEATING,
    THERMAL_OUTPUT_COOLING
} thermal_output_mode_t;

typedef struct {
    system_state_t state;
    error_code_t error;
    float temperature_1_c;
    float temperature_c;
    float temperature_2_c;
    float setpoint_c;
    float control_error_c;
    /* Signed actuator command: positive heats, negative cools. */
    float peltier_power_percent;
    thermal_output_mode_t thermal_output_mode;
    float controller_proportional_percent;
    float controller_integral_percent;
    float peltier_1_current_a;
    float peltier_2_current_a;
    uint16_t fan_rpm;
    uint8_t fan_percent;
    float light_level;
    bool temperature_1_valid;
    bool temperature_2_valid;
    bool temperature_valid;
    bool current_1_valid;
    bool current_2_valid;
    bool current_valid;
    bool tla2024_available;
    bool light_sensor_available;
    bool display_initialized;
    bool status_leds_initialized;
    bool cup_detected;
    bool power_5v_ok;
    bool controller_output_limited;
    bool controller_anti_windup_active;
    bool wifi_connected;
    bool webserver_ready;
    char wifi_ip[16];
    bool night_mode;
} system_status_t;

const char *system_state_name(system_state_t state);
const char *error_name(error_code_t error);
