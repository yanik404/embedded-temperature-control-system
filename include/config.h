#pragma once

#include "hardware/i2c.h"
#include "secrets.h"

/* Fixed PCB pin assignment - do not change without a PCB revision. */
#define PIN_I2C_DATA       0u
#define PIN_I2C_CLOCK      1u
#define PIN_OUT_LED1_N     2u
#define PIN_OUT_LED2_N     3u
#define PIN_OUT_LED3_N     4u
#define PIN_HB_LED_N       5u
#define PIN_FB_LED_N       6u
#define PIN_PG_5V0         7u
#define PIN_RGB_DIN        8u
#define PIN_S_MODE         9u
#define PIN_S_DOWN         10u
#define PIN_S_OK           11u
#define PIN_S_UP           12u
#define PIN_S_DETECT       13u
#define PIN_FAN_PWM        14u
#define PIN_FAN_TACHO      15u
#define PIN_FAN_CTRL       16u
#define PIN_PEL1_INA       17u
#define PIN_PEL1_INB       18u
#define PIN_PEL1_PWM       19u
#define PIN_PEL1_SEL       20u
#define PIN_PEL2_INA       21u
#define PIN_PEL2_INB       22u
#define PIN_TEMP_1_ADC     26u
#define PIN_PEL2_PWM       27u
#define PIN_PEL2_SEL       28u

#define TEMP_1_ADC_CHANNEL 0u
#define I2C_PORT            i2c0
#define I2C_BAUD_HZ         400000u
#define TLA2024_ADDRESS     0x48u
#define SSD1306_ADDRESS     0x3Cu

/* WLAN client timing. Network availability never grants heating permission. */
#define WIFI_CONNECT_TIMEOUT_MS 15000u
#define WIFI_RETRY_DELAY_MS      5000u

/* Local dashboard control authorization. Safety checks remain independent. */
#define WEB_CONTROL_SESSION_MS   300000u

/* TLA2024 channels. */
#define TLA_CH_PEL1_CS      0u
#define TLA_CH_PEL2_CS      1u
#define TLA_CH_LIGHT        2u
#define TLA_CH_TEMP_2       3u

/* Central process limits. */
#define SETPOINT_MIN_C              20.0f
#define SETPOINT_MAX_C              60.0f
#define SETPOINT_DEFAULT_C          45.0f
#define MAX_SAFE_TEMPERATURE_C      65.0f
#define MIN_SAFE_TEMPERATURE_C      18.0f
#define SENSOR_MIN_C                -10.0f
#define SENSOR_MAX_C                90.0f
#define TMP36_OFFSET_V              0.500f
#define TMP36_VOLTS_PER_C           0.010f
#define ADC_REFERENCE_V             3.300f
#define TEMPERATURE_FILTER_ALPHA    0.18f

/* Controller parameters. Positive output heats, negative output cools. */
#define CONTROL_PERIOD_MS           250u
#define PI_KP                       8.0f
#define PI_KI                       0.12f
#define HOLDING_ENTER_BAND_C        0.4f
#define HOLDING_EXIT_BAND_C         1.0f
#define PELTIER_MAX_HEATING_PERCENT 100.0f
#define PELTIER_MAX_COOLING_PERCENT 20.0f
#define PELTIER_OUTPUT_DEADBAND_PERCENT 1.0f

#define PELTIER_PWM_HZ              20000u
#define FAN_PWM_HZ                  25000u
#define FAN_MIN_ACTIVE_PERCENT      35u
#define FAN_RUN_ON_MS               15000u
#define FAN_TACH_PULSES_PER_REV     2u
#define FAN_FAULT_POWER_PERCENT     40u
#define FAN_FAULT_COOLING_POWER_PERCENT 15u
#define FAN_FAULT_GRACE_MS          8000u
#define FAN_MIN_VALID_RPM           250u

/* Calibrate these constants against the assembled PCB. */
#define CURRENT_AMPS_PER_VOLT       4.0f
#define CURRENT_ZERO_V              0.0f
#define CURRENT_MAX_A               8.0f
#define CURRENT_PLAUSIBLE_MIN_A    -0.25f
#define CURRENT_PLAUSIBLE_MAX_A     12.0f

#define BUTTON_DEBOUNCE_MS          30u
#define LIGHT_DARK_THRESHOLD        0.18f
#define DISPLAY_PERIOD_MS           350u
#define STATUS_LED_PERIOD_MS         50u
#define SENSOR_PERIOD_MS            200u
#define WEB_STATUS_PERIOD_MS        500u

/*
 * VNH7070 full-bridge directions. The physical heat/cool assignment must be
 * confirmed once on the assembled PCB with a current-limited 12 V supply.
 * SEL0 selects the conducting high-side current-sense channel (HSA or HSB).
 */
#define PELTIER_HEAT_INA_LEVEL      1
#define PELTIER_HEAT_INB_LEVEL      0
#define PELTIER_HEAT_SEL_LEVEL      1
#define PELTIER_COOL_INA_LEVEL      0
#define PELTIER_COOL_INB_LEVEL      1
#define PELTIER_COOL_SEL_LEVEL      0
#define PELTIER_DIRECTION_DEADTIME_MS 10u
#define PELTIER_WAKE_DELAY_US       20u
