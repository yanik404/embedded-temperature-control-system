#include "safety.h"

#include "config.h"

void safety_init(void) {
}

bool safety_can_start(const system_status_t *status) {
    return status != NULL && status->temperature_valid && status->current_valid &&
           status->cup_detected && status->power_5v_ok && status->error == ERROR_NONE;
}

error_code_t safety_check(const system_status_t *status, uint32_t heating_elapsed_ms) {
    if (status == NULL || !status->temperature_valid) return ERROR_TEMP_SENSOR;
    if (status->temperature_c >= MAX_SAFE_TEMPERATURE_C) return ERROR_OVERTEMPERATURE;
    if (!status->current_valid) return ERROR_CURRENT_SENSOR;
    if (status->peltier_1_current_a > CURRENT_MAX_A || status->peltier_2_current_a > CURRENT_MAX_A) {
        return ERROR_OVERCURRENT;
    }
    if ((status->state == SYSTEM_HEATING || status->state == SYSTEM_HOLDING) &&
        !status->cup_detected) return ERROR_CUP_REMOVED;
    if (!status->power_5v_ok) return ERROR_POWER_SUPPLY;
    if (status->peltier_power_percent >= FAN_FAULT_POWER_PERCENT &&
        heating_elapsed_ms >= FAN_FAULT_GRACE_MS && status->fan_rpm < FAN_MIN_VALID_RPM) {
        return ERROR_FAN;
    }
    return ERROR_NONE;
}
