#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool active_level;
    bool candidate_present;
    bool stable_present;
    uint32_t candidate_since_ms;
} cup_detector_t;

void cup_detector_init(cup_detector_t *detector, bool raw_level,
                       bool active_level, uint32_t now_ms);
bool cup_detector_update(cup_detector_t *detector, bool raw_level,
                         uint32_t now_ms, uint32_t insert_debounce_ms,
                         uint32_t remove_debounce_ms);
bool cup_detector_is_present(const cup_detector_t *detector);
