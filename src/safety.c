#include "safety.h"

#include "config.h"

#include <math.h>

static bool thermal_run_active(system_state_t state) {
    return state == SYSTEM_HEATING || state == SYSTEM_COOLING || state == SYSTEM_HOLDING;
}

void safety_init(void) {
}

bool safety_can_start(const system_status_t *status) {
    return status != NULL && status->temperature_valid && status->current_valid &&
           status->cup_detected && status->power_5v_ok && status->error == ERROR_NONE;
}

error_code_t safety_check(const system_status_t *status, uint32_t thermal_run_elapsed_ms) {
    if (status == NULL || !status->temperature_valid) return ERROR_TEMP_SENSOR;
    if (status->temperature_c >= MAX_SAFE_TEMPERATURE_C) return ERROR_OVERTEMPERATURE;
    if (status->temperature_c <= MIN_SAFE_TEMPERATURE_C) return ERROR_UNDERTEMPERATURE;
    if (!status->current_valid) return ERROR_CURRENT_SENSOR;
    if (fabsf(status->peltier_1_current_a) > CURRENT_MAX_A ||
        fabsf(status->peltier_2_current_a) > CURRENT_MAX_A) {
        return ERROR_OVERCURRENT;
    }
    if (thermal_run_active(status->state) && !status->cup_detected) return ERROR_CUP_REMOVED;
    /* USB-only diagnostics are allowed. Power-good becomes mandatory at START
       and remains monitored while a 12 V load is intentionally active. */
    if (thermal_run_active(status->state) && !status->power_5v_ok) return ERROR_POWER_SUPPLY;
    const float fan_fault_threshold = status->thermal_output_mode == THERMAL_OUTPUT_COOLING
                                          ? FAN_FAULT_COOLING_POWER_PERCENT
                                          : FAN_FAULT_POWER_PERCENT;
    if (fabsf(status->peltier_power_percent) >= fan_fault_threshold &&
        thermal_run_elapsed_ms >= FAN_FAULT_GRACE_MS && status->fan_rpm < FAN_MIN_VALID_RPM) {
        return ERROR_FAN;
    }
    return ERROR_NONE;
}
