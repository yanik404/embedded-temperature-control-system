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
static volatile int cup_manual_requested = -1;
static volatile bool rgb_test_requested;
static volatile bool peltier_test_requested;
static volatile bool workshop_override_requested;
static volatile uint8_t requested_test_channel;
static volatile bool requested_test_cooling;
static volatile uint8_t requested_test_profile;
static volatile bool presentation_demo_requested;
static volatile bool requested_demo_cooling;
static volatile bool setpoint_requested;
static volatile float requested_setpoint;
static volatile bool limits_requested;
static volatile float requested_max_heating;
static volatile float requested_max_cooling;
static volatile float requested_max_temperature;
static volatile float requested_min_temperature;
static uint32_t thermal_run_started_ms;
static uint32_t fan_run_on_until_ms;
static bool manual_off;
static bool peltier_test_active;
static uint32_t peltier_test_due_ms;
static uint32_t workshop_override_due_ms;
static bool presentation_demo_output_active;
static uint32_t presentation_demo_output_due_ms;

static uint32_t milliseconds(void) {
    return to_ms_since_boot(get_absolute_time());
}

const char *system_state_name(system_state_t state) {
    switch (state) {
        case SYSTEM_OFF: return "AUS";
        case SYSTEM_READY: return "BEREIT";
        case SYSTEM_HEATING: return "AUFHEIZEN";
        case SYSTEM_COOLING: return "KUEHLEN";
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
        case ERROR_UNDERTEMPERATURE: return "UNTERTEMPERATUR";
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
static void request_cup_manual(float enabled) { cup_manual_requested = enabled > 0.0f ? 1 : 0; }
static void request_rgb_test(void) { rgb_test_requested = true; }
static void request_workshop_override(void) { workshop_override_requested = true; }
static void request_peltier_test(uint8_t channel, bool cooling, uint8_t profile) {
    requested_test_channel = channel;
    requested_test_cooling = cooling;
    requested_test_profile = profile;
    peltier_test_requested = true;
}
static void request_presentation_demo(bool cooling) {
    requested_demo_cooling = cooling;
    presentation_demo_requested = true;
}
static void request_setpoint(float value) {
    requested_setpoint = value;
    setpoint_requested = true;
}

static float clamp_setpoint(float value) {
    if (value < SETPOINT_MIN_C) return SETPOINT_MIN_C;
    if (value > SETPOINT_MAX_C) return SETPOINT_MAX_C;
    return value;
}

static bool thermal_run_active(system_state_t state) {
    return state == SYSTEM_HEATING || state == SYSTEM_COOLING || state == SYSTEM_HOLDING;
}
static void request_limits(float heating_percent, float cooling_percent, float max_temperature_c, float min_temperature_c) {
    requested_max_heating = heating_percent;
    requested_max_cooling = cooling_percent;
    requested_max_temperature = max_temperature_c;
    requested_min_temperature = min_temperature_c;
    limits_requested = true;
}

static bool workshop_override_active(uint32_t now) {
    return status.workshop_override_active && (int32_t)(workshop_override_due_ms - now) > 0;
}

static bool peltier_test_can_start(uint32_t now) {
    return status.temperature_valid && status.current_valid && status.error == ERROR_NONE &&
           status.cup_detected && (status.power_5v_ok || workshop_override_active(now));
}

static bool control_can_start(uint32_t now) {
    system_status_t effective = status;
    if (workshop_override_active(now)) {
        effective.power_5v_ok = true;
    }
    return safety_can_start(&effective);
}

static void all_thermal_output_off(void) {
    peltier_test_active = false;
    presentation_demo_output_active = false;
    status.peltier_test_active = false;
    status.peltier_test_remaining_ms = 0u;
    peltier_off();
    status.peltier_power_percent = 0.0f;
    status.thermal_output_mode = THERMAL_OUTPUT_OFF;
    status.controller_proportional_percent = 0.0f;
    status.controller_integral_percent = 0.0f;
    status.controller_output_limited = false;
    status.controller_anti_windup_active = false;
    controller_reset(&controller);
}

static void enter_error(error_code_t error) {
    all_thermal_output_off();
    status.error = error;
    status.cup_manual = false;
    status.cup_detected = buttons_cup_detected();
    status.state = SYSTEM_ERROR;
}

static void process_commands(void) {
    if (cup_manual_requested >= 0) {
        status.cup_manual = cup_manual_requested == 1;
        cup_manual_requested = -1;
        status.cup_detected = buttons_cup_detected() || status.cup_manual;
        if (!status.cup_manual) stop_requested = true;
    }
    if (limits_requested) {
        limits_requested = false;
        if (requested_max_heating >= 10.0f && requested_max_heating <= PELTIER_MAX_HEATING_PERCENT &&
            requested_max_cooling >= 5.0f && requested_max_cooling <= PELTIER_MAX_COOLING_PERCENT &&
            requested_max_temperature >= 40.0f && requested_max_temperature <= MAX_SAFE_TEMPERATURE_C &&
            requested_min_temperature >= MIN_SAFE_TEMPERATURE_C && requested_min_temperature <= 30.0f &&
            requested_min_temperature < requested_max_temperature) {
            status.max_heating_percent = requested_max_heating;
            status.max_cooling_percent = requested_max_cooling;
            status.max_temperature_c = requested_max_temperature;
            status.min_temperature_c = requested_min_temperature;
            controller_set_output_limits(&controller, -status.max_cooling_percent,
                                         status.max_heating_percent);
        }
    }
    if (rgb_test_requested) {
        rgb_test_requested = false;
        status_leds_start_test();
        printf("[LED] RGB-Ring-Test gestartet\n");
    }
    if (workshop_override_requested) {
        workshop_override_requested = false;
        if (!thermal_run_active(status.state)) {
            workshop_override_due_ms = milliseconds() + 300000u;
            status.workshop_override_active = true;
            status.workshop_override_remaining_ms = 300000u;
            if (control_can_start(milliseconds())) status.state = SYSTEM_READY;
            printf("[PELTIER] Power-Good-Override fuer 5 min aktiviert; Leistung max. 10 %%\n");
        }
    }
    if (peltier_test_requested) {
        peltier_test_requested = false;
        if (!thermal_run_active(status.state) && peltier_test_can_start(milliseconds())) {
            static const uint8_t powers[] = {10u, 15u, 20u};
            static const uint32_t durations[] = {5000u, 15000u, 30000u};
            const bool overridden = workshop_override_active(milliseconds()) &&
                                    !status.power_5v_ok;
            const uint8_t profile = overridden || requested_test_profile > 2u
                                        ? 0u : requested_test_profile;
            const peltier_channel_t channel = requested_test_channel == 2u
                                                  ? PELTIER_CHANNEL_2 : PELTIER_CHANNEL_1;
            const peltier_direction_t direction = requested_test_cooling
                                                       ? PELTIER_DIRECTION_COOL
                                                       : PELTIER_DIRECTION_HEAT;
            all_thermal_output_off();
            (void)peltier_set_direction(direction);
            peltier_set_power(channel, powers[profile]);
            peltier_set_enabled(true);
            peltier_test_active = true;
            peltier_test_due_ms = milliseconds() + durations[profile];
            thermal_run_started_ms = milliseconds();
            status.peltier_test_active = true;
            status.peltier_test_channel = (uint8_t)channel + 1u;
            status.peltier_test_remaining_ms = durations[profile];
            status.peltier_power_percent = requested_test_cooling
                                               ? -(float)powers[profile] : (float)powers[profile];
            status.thermal_output_mode = requested_test_cooling
                                             ? THERMAL_OUTPUT_COOLING : THERMAL_OUTPUT_HEATING;
            status.state = requested_test_cooling ? SYSTEM_COOLING : SYSTEM_HEATING;
            printf("[PELTIER] Test Kanal %u: %s, %u %%, %u s\n",
                   (unsigned)status.peltier_test_channel,
                   requested_test_cooling ? "KUEHLEN" : "HEIZEN",
                   (unsigned)powers[profile], (unsigned)(durations[profile] / 1000u));
        }
    }
    if (presentation_demo_requested) {
        presentation_demo_requested = false;
        all_thermal_output_off();
        if (safety_can_start(&status)) {
            const peltier_direction_t direction = requested_demo_cooling
                                                       ? PELTIER_DIRECTION_COOL
                                                       : PELTIER_DIRECTION_HEAT;
            (void)peltier_set_direction(direction);
            peltier_set_power(PELTIER_CHANNEL_1, 5u);
            peltier_set_power(PELTIER_CHANNEL_2, 5u);
            peltier_set_enabled(true);
            presentation_demo_output_active = true;
            presentation_demo_output_due_ms = milliseconds() + 20000u;
            thermal_run_started_ms = milliseconds();
            status.peltier_power_percent = requested_demo_cooling ? -5.0f : 5.0f;
            status.thermal_output_mode = requested_demo_cooling
                                             ? THERMAL_OUTPUT_COOLING : THERMAL_OUTPUT_HEATING;
            status.state = requested_demo_cooling ? SYSTEM_COOLING : SYSTEM_HEATING;
            printf("[PELTIER] Praesentationsdemo: %s mit 5 %% fuer max. 20 s\n",
                   requested_demo_cooling ? "KUEHLEN" : "HEIZEN");
        } else {
            printf("[PELTIER] Praesentationsdemo ohne reale Leistung: Sicherheitsfreigabe fehlt\n");
        }
    }
    if (setpoint_requested) {
        status.setpoint_c = clamp_setpoint(requested_setpoint);
        const float new_error = status.setpoint_c - status.temperature_c;
        const bool output_opposes_new_target =
            (status.thermal_output_mode == THERMAL_OUTPUT_HEATING && new_error < 0.0f) ||
            (status.thermal_output_mode == THERMAL_OUTPUT_COOLING && new_error > 0.0f);
        if (thermal_run_active(status.state) && output_opposes_new_target) {
            /* A setpoint crossing must never keep driving in the old direction
               while the integrator unwinds. The next control cycle starts at 0. */
            peltier_set_power(PELTIER_CHANNEL_1, 0u);
            peltier_set_power(PELTIER_CHANNEL_2, 0u);
            controller_reset(&controller);
            status.peltier_power_percent = 0.0f;
            status.thermal_output_mode = THERMAL_OUTPUT_OFF;
            printf("[PELTIER] Sollwert kreuzt Istwert -> Ausgang sicher auf 0 %%\n");
        }
        setpoint_requested = false;
    }
    if (stop_requested) {
        stop_requested = false;
        status.cup_manual = false;
        status.cup_detected = buttons_cup_detected();
        all_thermal_output_off();
        status.error = ERROR_NONE;
        status.state = control_can_start(milliseconds()) ? SYSTEM_READY : SYSTEM_OFF;
        fan_run_on_until_ms = milliseconds() + FAN_RUN_ON_MS;
    }
    if (start_requested) {
        start_requested = false;
        if ((status.state == SYSTEM_READY || status.state == SYSTEM_OFF) && control_can_start(milliseconds())) {
            manual_off = false;
            controller_reset(&controller);
            thermal_run_started_ms = milliseconds();
            status.control_error_c = status.setpoint_c - status.temperature_c;
            const bool cooling = status.control_error_c < 0.0f;
            (void)peltier_set_direction(cooling ? PELTIER_DIRECTION_COOL : PELTIER_DIRECTION_HEAT);
            peltier_set_enabled(true);
            status.state = fabsf(status.control_error_c) <= HOLDING_ENTER_BAND_C
                               ? SYSTEM_HOLDING
                               : (cooling ? SYSTEM_COOLING : SYSTEM_HEATING);
            printf("[PELTIER] Regelbetrieb gestartet: %s, Kuehlgrenze %.0f %%\n",
                   cooling ? "KUEHLEN" : "HEIZEN", PELTIER_MAX_COOLING_PERCENT);
        }
    }
}

static void process_buttons(void) {
    buttons_update();
    if (button_was_pressed(BUTTON_UP)) request_setpoint(status.setpoint_c + 0.5f);
    if (button_was_pressed(BUTTON_DOWN)) request_setpoint(status.setpoint_c - 0.5f);
    if (button_was_pressed(BUTTON_OK)) {
        if (thermal_run_active(status.state)) request_stop();
        else if (status.state == SYSTEM_ERROR) request_stop();
        else request_start();
    }
    if (button_was_pressed(BUTTON_MODE) &&
        (status.state == SYSTEM_OFF || status.state == SYSTEM_READY)) {
        if (status.state == SYSTEM_READY) {
            manual_off = true;
            all_thermal_output_off();
            status.state = SYSTEM_OFF;
        } else if (safety_can_start(&status)) {
            manual_off = false;
            status.state = SYSTEM_READY;
        }
    }
}

static void sample_sensors(void) {
    const temperature_reading_t temperatures = temperature_read();
    const current_reading_t currents = current_measurement_read();
    status.temperature_1_c = temperatures.primary_c;
    status.temperature_c = temperatures.process_c;
    status.temperature_2_c = temperatures.secondary_c;
    status.temperature_1_valid = temperatures.primary_valid;
    status.temperature_2_valid = temperatures.secondary_valid;
    status.temperature_valid = temperatures.valid;
    status.peltier_1_current_a = currents.channel_1_a;
    status.peltier_2_current_a = currents.channel_2_a;
    status.current_1_valid = currents.channel_1_valid;
    status.current_2_valid = currents.channel_2_valid;
    status.current_valid = currents.valid;
    /* GP7 is tied to 3.3 V on the assembled board and is not a usable PG signal. */
    status.power_5v_ok = true;
    float light;
    status.light_sensor_available = light_sensor_read(&light);
    if (status.light_sensor_available) status.light_level = light;
    status.tla2024_available = tla2024_is_available();
    status.night_mode = status.light_sensor_available && status.light_level < LIGHT_DARK_THRESHOLD;
}

static void update_control(uint32_t now) {
    status.control_error_c = status.setpoint_c - status.temperature_c;
    const uint32_t elapsed = thermal_run_active(status.state)
                                 ? now - thermal_run_started_ms : 0u;
    if (status.workshop_override_active) {
        if (!workshop_override_active(now)) {
            status.workshop_override_active = false;
            status.workshop_override_remaining_ms = 0u;
            if (thermal_run_active(status.state) && !status.power_5v_ok) {
                all_thermal_output_off();
                status.state = SYSTEM_OFF;
                fan_run_on_until_ms = now + FAN_RUN_ON_MS;
                return;
            }
        } else {
            status.workshop_override_remaining_ms = workshop_override_due_ms - now;
        }
    }
    system_status_t safety_status = status;
    if (thermal_run_active(status.state) && workshop_override_active(now)) {
        safety_status.power_5v_ok = true;
    }
    const error_code_t fault = safety_check(&safety_status, elapsed);
    if (fault != ERROR_NONE) {
        /* Every safety fault is latched, including faults detected before START. */
        if (status.state != SYSTEM_ERROR) enter_error(fault);
        return;
    }
    if (status.state == SYSTEM_ERROR) return; /* Latched until OK/STOP. */
    if (presentation_demo_output_active) {
        if ((int32_t)(now - presentation_demo_output_due_ms) >= 0) {
            all_thermal_output_off();
            status.state = control_can_start(now) ? SYSTEM_READY : SYSTEM_OFF;
            fan_run_on_until_ms = now + FAN_RUN_ON_MS;
        }
        return;
    }
    if (peltier_test_active) {
        if ((int32_t)(now - peltier_test_due_ms) >= 0) {
            all_thermal_output_off();
            status.state = safety_can_start(&status) ? SYSTEM_READY : SYSTEM_OFF;
            fan_run_on_until_ms = now + FAN_RUN_ON_MS;
        } else {
            status.peltier_test_remaining_ms = peltier_test_due_ms - now;
        }
        return;
    }
    status.error = ERROR_NONE;
    if (status.state == SYSTEM_READY && !control_can_start(now)) {
        status.state = SYSTEM_OFF;
    }
    if (status.state == SYSTEM_OFF && !manual_off && control_can_start(now)) {
        status.state = SYSTEM_READY;
    }
    if (!thermal_run_active(status.state)) return;

    const float output = controller_update(&controller, status.setpoint_c, status.temperature_c,
                                           CONTROL_PERIOD_MS / 1000.0f);
    status.controller_proportional_percent = controller.kp * status.control_error_c;
    status.controller_integral_percent = controller.integral;
    const float unlimited_output = status.controller_proportional_percent +
                                   status.controller_integral_percent;
    status.controller_output_limited = unlimited_output <= controller.output_min ||
                                       unlimited_output >= controller.output_max;
    status.controller_anti_windup_active =
        (unlimited_output >= controller.output_max && status.control_error_c > 0.0f) ||
        (unlimited_output <= controller.output_min && status.control_error_c < 0.0f);
    float applied_output = fabsf(output) < PELTIER_OUTPUT_DEADBAND_PERCENT ? 0.0f : output;
    if (workshop_override_active(now)) {
        if (applied_output > 10.0f) applied_output = 10.0f;
        if (applied_output < -10.0f) applied_output = -10.0f;
    }
    thermal_output_mode_t mode = THERMAL_OUTPUT_OFF;
    if (applied_output > 0.0f) mode = THERMAL_OUTPUT_HEATING;
    if (applied_output < 0.0f) mode = THERMAL_OUTPUT_COOLING;

    if (mode != THERMAL_OUTPUT_OFF) {
        const peltier_direction_t direction = mode == THERMAL_OUTPUT_COOLING
                                                  ? PELTIER_DIRECTION_COOL
                                                  : PELTIER_DIRECTION_HEAT;
        if (peltier_get_direction() != direction) {
            /* peltier_set_direction performs PWM-off, break-before-make and
               leaves the bridge disabled until the explicit enable below. */
            (void)peltier_set_direction(direction);
            peltier_set_enabled(true);
            printf("[PELTIER] Sichere Richtungsumschaltung -> %s\n",
                   direction == PELTIER_DIRECTION_COOL ? "KUEHLEN" : "HEIZEN");
        }
    }

    status.peltier_power_percent = applied_output;
    status.thermal_output_mode = mode;
    const uint8_t percent = (uint8_t)(fabsf(applied_output) + 0.5f);
    peltier_set_power(PELTIER_CHANNEL_1, percent);
    peltier_set_power(PELTIER_CHANNEL_2, percent);

    if ((status.state == SYSTEM_HEATING || status.state == SYSTEM_COOLING) &&
        fabsf(status.control_error_c) <= HOLDING_ENTER_BAND_C) {
        status.state = SYSTEM_HOLDING;
    } else if (status.control_error_c > HOLDING_EXIT_BAND_C) {
        status.state = SYSTEM_HEATING;
    } else if (status.control_error_c < -HOLDING_EXIT_BAND_C) {
        status.state = SYSTEM_COOLING;
    }
}

static void update_fan(uint32_t now) {
    fan_update();
    if (thermal_run_active(status.state)) {
        const float output_magnitude = fabsf(status.peltier_power_percent);
        uint8_t desired = FAN_MIN_ACTIVE_PERCENT +
                          (uint8_t)((100u - FAN_MIN_ACTIVE_PERCENT) * output_magnitude / 100.0f);
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
    status.max_temperature_c = MAX_SAFE_TEMPERATURE_C;
    status.min_temperature_c = MIN_SAFE_TEMPERATURE_C;
    /* Daily-use default after every reboot; the web UI may raise it up to 100 %. */
    status.max_heating_percent = PELTIER_DEFAULT_HEATING_PERCENT;
    status.max_cooling_percent = PELTIER_MAX_COOLING_PERCENT;
    manual_off = false;
    peltier_test_active = false;
    presentation_demo_output_active = false;
    controller_init(&controller, PI_KP, PI_KI);
    controller_set_output_limits(&controller, -status.max_cooling_percent,
                                 status.max_heating_percent);
    safety_init();

    gpio_init(PIN_PG_5V0);
    gpio_set_dir(PIN_PG_5V0, GPIO_IN);
    i2c_init(I2C_PORT, I2C_BAUD_HZ);
    gpio_set_function(PIN_I2C_DATA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_DATA);
    gpio_pull_up(PIN_I2C_CLOCK);
    status.tla2024_available = tla2024_init();
    temperature_init();
    current_measurement_init();
    light_sensor_init();
    buttons_init();
    status.display_initialized = display_init();
    status_leds_init();
    status.status_leds_initialized = true;

    const webserver_config_t web_config = {
        .status = &status,
        .start = request_start,
        .stop = request_stop,
        .set_cup_manual = request_cup_manual,
        .rgb_test = request_rgb_test,
        .workshop_override = request_workshop_override,
        .peltier_test = request_peltier_test,
        .presentation_demo = request_presentation_demo,
        .set_setpoint = request_setpoint,
        .set_limits = request_limits
    };
    (void)webserver_init(&web_config);
    watchdog_enable(3000u, true);
}

void app_run(void) {
    uint32_t sensor_due = 0u, control_due = 0u, display_due = 0u;
    uint32_t status_led_due = 0u, web_due = 0u;
    while (true) {
        const uint32_t now = milliseconds();
        process_buttons();
        status.cup_detected = buttons_cup_detected();
        status.cup_detected = status.cup_detected || status.cup_manual;
        status.cup_switch_raw = buttons_cup_raw_level();
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
        }
        if ((int32_t)(now - web_due) >= 0) {
            web_due = now + WEB_STATUS_PERIOD_MS;
            webserver_update();
            status.wifi_connected = webserver_is_connected();
            status.webserver_ready = webserver_is_ready();
            (void)webserver_get_ip(status.wifi_ip, sizeof(status.wifi_ip));
        }
        if ((int32_t)(now - status_led_due) >= 0) {
            status_led_due = now + STATUS_LED_PERIOD_MS;
            status_leds_update(&status);
        }
        watchdog_update();
        sleep_ms(1u);
    }
}
