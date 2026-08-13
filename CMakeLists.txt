cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w CACHE STRING "Target board")
include(pico_sdk_import.cmake)

project(embedded_temperature_control C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(DASHBOARD_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui-v3/src/index.html
    ${CMAKE_CURRENT_LIST_DIR}/ui-v3/src/experience.css
    ${CMAKE_CURRENT_LIST_DIR}/ui-v3/src/experience.js
    ${CMAKE_CURRENT_LIST_DIR}/ui-v3/src/product-scene.js
    ${CMAKE_CURRENT_LIST_DIR}/ui-v3/src/thermal-overlay.js
    ${CMAKE_CURRENT_LIST_DIR}/ui-v3/src/preview.js
    ${CMAKE_CURRENT_LIST_DIR}/ui-v4/src/component-model.js
    ${CMAKE_CURRENT_LIST_DIR}/ui-v4/src/digital-twin.js
    ${CMAKE_CURRENT_LIST_DIR}/ui-v4/src/digital-twin.css
    ${CMAKE_CURRENT_LIST_DIR}/tools/build_ui.py
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_LIST_DIR}/include/web_assets.h
        ${CMAKE_CURRENT_LIST_DIR}/preview.html
        ${CMAKE_CURRENT_LIST_DIR}/ui/dist/dashboard.production.html
        ${CMAKE_CURRENT_LIST_DIR}/ui/dist/build-stats.json
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/tools/build_ui.py
    DEPENDS ${DASHBOARD_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating preview and embedded dashboard assets"
    VERBATIM
)

add_custom_target(dashboard_assets ALL DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/include/web_assets.h
    ${CMAKE_CURRENT_LIST_DIR}/preview.html
    ${CMAKE_CURRENT_LIST_DIR}/ui/dist/dashboard.production.html
    ${CMAKE_CURRENT_LIST_DIR}/ui/dist/build-stats.json
)

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
    src/status_led_logic.c
    src/status_leds.c
    src/safety.c
    src/webserver.c
)

add_dependencies(temperature_control dashboard_assets)

target_include_directories(temperature_control PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_options(temperature_control PRIVATE -Wall -Wextra)

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
