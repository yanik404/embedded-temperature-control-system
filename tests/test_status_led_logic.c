#include <assert.h>
#include <stdio.h>

#include "status_led_logic.h"

static status_led_output_t evaluate(system_state_t state, error_code_t error,
                                    bool webserver_ready, bool night_mode, bool blink_on) {
    const system_status_t status = {
        .state = state,
        .error = error,
        .webserver_ready = webserver_ready,
        .night_mode = night_mode
    };
    status_led_output_t output;
    status_led_logic_evaluate(&status, blink_on, &output);
    return output;
}

int main(void) {
    status_led_output_t output = evaluate(SYSTEM_OFF, ERROR_NONE, false, false, true);
    assert(!output.ready_on && !output.heating_on && !output.holding_on);
    assert(!output.heartbeat_on && !output.fault_on);
    assert(output.ring_red == 0u && output.ring_green == 0u && output.ring_blue == 0u);

    output = evaluate(SYSTEM_READY, ERROR_NONE, true, false, true);
    assert(output.ready_on && !output.heating_on && !output.holding_on);
    assert(output.heartbeat_on && !output.fault_on);
    assert(output.ring_red == 24u && output.ring_green == 24u && output.ring_blue == 24u);

    output = evaluate(SYSTEM_HEATING, ERROR_NONE, false, false, true);
    assert(!output.ready_on && output.heating_on && !output.holding_on);
    assert(!output.heartbeat_on && !output.fault_on);
    assert(output.ring_red == 24u && output.ring_green == 8u && output.ring_blue == 0u);

    output = evaluate(SYSTEM_COOLING, ERROR_NONE, false, false, true);
    assert(!output.ready_on && !output.heating_on && !output.holding_on);
    assert(!output.heartbeat_on && !output.fault_on);
    assert(output.ring_red == 0u && output.ring_green == 6u && output.ring_blue == 24u);

    output = evaluate(SYSTEM_HOLDING, ERROR_NONE, false, false, true);
    assert(!output.ready_on && !output.heating_on && output.holding_on);
    assert(output.ring_red == 0u && output.ring_green == 24u && output.ring_blue == 0u);

    output = evaluate(SYSTEM_ERROR, ERROR_TEMP_SENSOR, false, false, true);
    assert(!output.ready_on && !output.heating_on && !output.holding_on);
    assert(output.fault_on);
    assert(output.ring_red == 24u && output.ring_green == 0u && output.ring_blue == 0u);

    output = evaluate(SYSTEM_ERROR, ERROR_TEMP_SENSOR, false, false, false);
    assert(!output.fault_on);
    assert(output.ring_red == 0u && output.ring_green == 0u && output.ring_blue == 0u);

    output = evaluate(SYSTEM_READY, ERROR_NONE, false, true, true);
    assert(!output.heartbeat_on);
    assert(output.ring_red == 5u && output.ring_green == 5u && output.ring_blue == 5u);

    puts("status LED logic tests passed");
    return 0;
}
