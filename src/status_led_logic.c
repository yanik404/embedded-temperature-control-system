#include "status_led_logic.h"

#include <string.h>

#define RGB_DAY_BRIGHTNESS 24u
#define RGB_NIGHT_BRIGHTNESS 5u

void status_led_logic_evaluate(const system_status_t *status, bool blink_on,
                               status_led_output_t *output) {
    if (output == NULL) return;
    memset(output, 0, sizeof(*output));
    if (status == NULL) return;

    output->ready_on = status->state == SYSTEM_READY;
    output->heating_on = status->state == SYSTEM_HEATING;
    output->holding_on = status->state == SYSTEM_HOLDING;
    output->heartbeat_on = status->webserver_ready;

    const bool error_active = status->state == SYSTEM_ERROR || status->error != ERROR_NONE;
    output->fault_on = error_active && blink_on;

    const uint8_t brightness = status->night_mode ? RGB_NIGHT_BRIGHTNESS : RGB_DAY_BRIGHTNESS;
    switch (status->state) {
        case SYSTEM_READY:
            output->ring_blue = brightness;
            break;
        case SYSTEM_HEATING:
            output->ring_red = brightness;
            output->ring_green = brightness / 3u;
            break;
        case SYSTEM_COOLING:
            output->ring_blue = brightness;
            output->ring_green = brightness / 4u;
            break;
        case SYSTEM_HOLDING:
            output->ring_green = brightness;
            break;
        case SYSTEM_ERROR:
            if (blink_on) output->ring_red = brightness;
            break;
        case SYSTEM_OFF:
        default:
            break;
    }
}
