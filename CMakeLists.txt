cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w CACHE STRING "Target board")
include(pico_sdk_import.cmake)

project(embedded_temperature_control C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(temperature_control
    src/main.c
    src/app.c
    src/controller.c
    src/peltier.c
    src/temperature.c
    src/tla2024.c
    src/current_measurement.c
    src/fan.c
    src/buttons.c
    src/light_sensor.c
    src/display.c
    src/status_leds.c
    src/safety.c
    src/webserver.c
)

target_include_directories(temperature_control PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_options(temperature_control PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(temperature_control
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
    hardware_i2c
    hardware_pwm
    hardware_watchdog
    hardware_pio
)

pico_generate_pio_header(temperature_control
    ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio
)

pico_enable_stdio_usb(temperature_control 1)
pico_enable_stdio_uart(temperature_control 0)
pico_add_extra_outputs(temperature_control)
