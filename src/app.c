#include "app.h"

#include <math.h>
#include <stdio.h>

#include "buttons.h"
#include "config.h"
#include "controller.h"
#include "current_measurement.h"
#include "display.h"
#include "fan.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"
#include "light_sensor.h"
#include "peltier.h"
#include "pico/stdlib.h"
#include "safety.h"
#include "status_leds.h"
#include "temperature.h"
#include "tla2024.h"
#include "webserver.h"

static system_status_t status;
static pi_controller_t controller;
static volatile bool start_requested;
static volatile bool stop_requested;
static volatile bool setpoint_requested;
static volatile float requested_setpoint;
static uint32_t heating_started_ms;
static uint32_t fan_run_on_until_ms;

static uint32_t milliseconds(void) {
    return to_ms_since_boot(get_absolute_time());
}

const char *system_state_name(system_state_t state) {
    switch (state) {
        case SYSTEM_OFF: return "AUS";
        case SYSTEM_READY: return "BEREIT";
        case SYSTEM_HEATING: return "AUFHEIZEN";
        case SYSTEM_HOLDING: return "HALTEN";
        case SYSTEM_ERROR: return "FEHLER";
        default: return "UNBEKANNT";
    }
}

const char *error_name(error_code_t error) {
    switch (error) {
        case ERROR_NONE: return "KEIN FEHLER";
        case ERROR_TEMP_SENSOR: return "TEMPERATURSENSOR";
        case ERROR_OVERTEMPERATURE: return "UEBERTEMPERATUR";
        case ERROR_FAN: return "LUEFTER";
        case ERROR_OVERCURRENT: return "UEBERSTROM";
        case ERROR_CURRENT_SENSOR: return "STROMSENSOR";
        case ERROR_CUP_REMOVED: return "BECHER ENTFERNT";
        case ERROR_POWER_SUPPLY: return "5V VERSORGUNG";
        default: return "UNBEKANNT";
    }
}

static void request_start(void) { start_requested = true; }
static void request_stop(void) { stop_requested = true; }
static void request_setpoint(float value) {
    requested_setpoint = value;
    setpoint_requested = true;
}

static float clamp_setpoint(float value) {
    if (value < SETPOINT_MIN_C) return SETPOINT_MIN_C;
    if (value > SETPOINT_MAX_C) return SETPOINT_MAX_C;
    return value;
}

static void all_heating_off(void) {
    peltier_off();
    status.peltier_power_percent = 0.0f;
    controller_reset(&controller);
}

static void enter_error(error_code_t error) {
    all_heating_off();
    status.error = error;
    status.state = SYSTEM_ERROR;
}

static void process_commands(void) {
    if (setpoint_requested) {
        status.setpoint_c = clamp_setpoint(requested_setpoint);
        setpoint_requested = false;
    }
    if (stop_requested) {
        stop_requested = false;
        all_heating_off();
        status.error = ERROR_NONE;
        status.state = safety_can_start(&status) ? SYSTEM_READY : SYSTEM_OFF;
        fan_run_on_until_ms = milliseconds() + FAN_RUN_ON_MS;
    }
    if (start_requested) {
        start_requested = false;
        if ((status.state == SYSTEM_READY || status.state == SYSTEM_OFF) && safety_can_start(&status)) {
            controller_reset(&controller);
            heating_started_ms = milliseconds();
            peltier_set_enabled(true);
            status.state = SYSTEM_HEATING;
        }
    }
}

static void process_buttons(void) {
    buttons_update();
    if (button_was_pressed(BUTTON_UP)) request_setpoint(status.setpoint_c + 0.5f);
    if (button_was_pressed(BUTTON_DOWN)) request_setpoint(status.setpoint_c - 0.5f);
    if (button_was_pressed(BUTTON_OK)) {
        if (status.state == SYSTEM_HEATING || status.state == SYSTEM_HOLDING) request_stop();
        else if (status.state == SYSTEM_ERROR) request_stop();
        else request_start();
    }
    if (button_was_pressed(BUTTON_MODE) &&
        (status.state == SYSTEM_OFF || status.state == SYSTEM_READY)) {
        if (status.state == SYSTEM_READY) {
            all_heating_off();
            status.state = SYSTEM_OFF;
        } else if (safety_can_start(&status)) {
            status.state = SYSTEM_READY;
        }
    }
}

static void sample_sensors(void) {
    const temperature_reading_t temperatures = temperature_read();
    const current_reading_t currents = current_measurement_read();
    status.temperature_c = temperatures.process_c;
    status.temperature_2_c = temperatures.secondary_c;
    status.temperature_valid = temperatures.valid;
    status.peltier_1_current_a = currents.channel_1_a;
    status.peltier_2_current_a = currents.channel_2_a;
    status.current_valid = currents.valid;
    status.cup_detected = buttons_cup_detected();
    status.power_5v_ok = gpio_get(PIN_PG_5V0);
    float light;
    if (light_sensor_read(&light)) status.light_level = light;
    status.night_mode = status.light_level < LIGHT_DARK_THRESHOLD;
}

static void update_control(uint32_t now) {
    status.control_error_c = status.setpoint_c - status.temperature_c;
    const uint32_t elapsed = (status.state == SYSTEM_HEATING || status.state == SYSTEM_HOLDING)
                                 ? now - heating_started_ms : 0u;
    const error_code_t fault = safety_check(&status, elapsed);
    if (fault != ERROR_NONE) {
        /* Every safety fault is latched, including faults detected before START. */
        if (status.state != SYSTEM_ERROR) enter_error(fault);
        return;
    }
    if (status.state == SYSTEM_ERROR) return; /* Latched until OK/STOP. */
    status.error = ERROR_NONE;
    if (status.state == SYSTEM_OFF && status.temperature_valid && status.current_valid) {
        status.state = SYSTEM_READY;
    }
    if (status.state != SYSTEM_HEATING && status.state != SYSTEM_HOLDING) return;

    const float output = controller_update(&controller, status.setpoint_c, status.temperature_c,
                                           CONTROL_PERIOD_MS / 1000.0f);
    status.peltier_power_percent = output;
    const uint8_t percent = (uint8_t)(output + 0.5f);
    peltier_set_power(PELTIER_CHANNEL_1, percent);
    peltier_set_power(PELTIER_CHANNEL_2, percent);

    if (status.state == SYSTEM_HEATING && fabsf(status.control_error_c) <= HOLDING_ENTER_BAND_C) {
        status.state = SYSTEM_HOLDING;
    } else if (status.state == SYSTEM_HOLDING && status.control_error_c > HOLDING_EXIT_BAND_C) {
        status.state = SYSTEM_HEATING;
    }
}

static void update_fan(uint32_t now) {
    fan_update();
    if (status.state == SYSTEM_HEATING || status.state == SYSTEM_HOLDING) {
        uint8_t desired = FAN_MIN_ACTIVE_PERCENT +
                          (uint8_t)((100u - FAN_MIN_ACTIVE_PERCENT) * status.peltier_power_percent / 100.0f);
        fan_set_speed(desired);
        fan_run_on_until_ms = now + FAN_RUN_ON_MS;
    } else if ((int32_t)(fan_run_on_until_ms - now) > 0) {
        fan_set_speed(FAN_MIN_ACTIVE_PERCENT);
    } else {
        fan_set_speed(0u);
    }
    status.fan_percent = fan_get_speed();
    status.fan_rpm = fan_get_rpm();
}

void app_init(void) {
    /* Safety-critical outputs are initialized first and remain disabled. */
    peltier_init();
    fan_init();
    stdio_init_all();

    status = (system_status_t){0};
    status.state = SYSTEM_OFF;
    status.error = ERROR_NONE;
    status.setpoint_c = SETPOINT_DEFAULT_C;
    controller_init(&controller, PI_KP, PI_KI);
    safety_init();

    gpio_init(PIN_PG_5V0);
    gpio_set_dir(PIN_PG_5V0, GPIO_IN);
    i2c_init(I2C_PORT, I2C_BAUD_HZ);
    gpio_set_function(PIN_I2C_DATA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_DATA);
    gpio_pull_up(PIN_I2C_CLOCK);
    (void)tla2024_init();
    temperature_init();
    current_measurement_init();
    light_sensor_init();
    buttons_init();
    (void)display_init();
    status_leds_init();

    const webserver_config_t web_config = {
        .status = &status,
        .start = request_start,
        .stop = request_stop,
        .set_setpoint = request_setpoint
    };
    (void)webserver_init(&web_config);
    watchdog_enable(3000u, true);
}

void app_run(void) {
    uint32_t sensor_due = 0u, control_due = 0u, display_due = 0u, web_due = 0u;
    while (true) {
        const uint32_t now = milliseconds();
        process_buttons();
        process_commands();
        if ((int32_t)(now - sensor_due) >= 0) {
            sensor_due = now + SENSOR_PERIOD_MS;
            sample_sensors();
        }
        update_fan(now);
        if ((int32_t)(now - control_due) >= 0) {
            control_due = now + CONTROL_PERIOD_MS;
            update_control(now);
        }
        if ((int32_t)(now - display_due) >= 0) {
            display_due = now + DISPLAY_PERIOD_MS;
            display_set_dimmed(status.night_mode);
            display_update(&status);
            status_leds_update(status.state, status.night_mode);
        }
        if ((int32_t)(now - web_due) >= 0) {
            web_due = now + WEB_STATUS_PERIOD_MS;
            status.wifi_connected = webserver_is_connected();
        }
        watchdog_update();
        sleep_ms(1u);
    }
}
