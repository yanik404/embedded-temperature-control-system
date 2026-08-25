from pathlib import Path


config = Path("include/config.h").read_text(encoding="utf-8")
peltier = Path("src/peltier.c").read_text(encoding="utf-8")
app = Path("src/app.c").read_text(encoding="utf-8")
safety = Path("src/safety.c").read_text(encoding="utf-8")
webserver = Path("src/webserver.c").read_text(encoding="utf-8")

for token in (
    "PELTIER_MAX_COOLING_PERCENT 20.0f",
    "PELTIER_DIRECTION_DEADTIME_MS 10u",
    "PELTIER_WAKE_DELAY_US       20u",
    "FAN_FAULT_COOLING_POWER_PERCENT 15u",
    "PELTIER_HEAT_SEL_LEVEL      1",
    "PELTIER_COOL_SEL_LEVEL      0",
):
    assert token in config

# Direction changes must always pass through the safe all-off path. Power is
# re-enabled explicitly by app.c only after the driver completed that sequence.
direction_function = peltier[peltier.index("bool peltier_set_direction"):peltier.index("peltier_direction_t peltier_get_direction")]
assert direction_function.index("peltier_off();") < direction_function.index("sleep_ms(PELTIER_DIRECTION_DEADTIME_MS)")
assert "output_enabled = enabled" in peltier and "sleep_us(PELTIER_WAKE_DELAY_US)" in peltier
assert "pwm_set_gpio_level(pwm_pin, 0u);" in peltier

for token in (
    "SYSTEM_COOLING",
    "-PELTIER_MAX_COOLING_PERCENT",
    "fabsf(applied_output)",
    "PELTIER_DIRECTION_COOL",
    "THERMAL_OUTPUT_COOLING",
):
    assert token in app

assert "MIN_SAFE_TEMPERATURE_C" in safety
assert "fabsf(status->peltier_power_percent)" in safety
assert "thermal_mode" in webserver and "thermal_output_name" in webserver
assert "max_cooling_power" in webserver and "PELTIER_MAX_COOLING_PERCENT" in webserver

print("bidirectional Peltier safety contract checks passed")
