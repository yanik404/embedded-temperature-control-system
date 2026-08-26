#include "cup_detector.h"

#include <stddef.h>

void cup_detector_init(cup_detector_t *detector, bool raw_level,
                       bool active_level, uint32_t now_ms) {
    if (detector == NULL) return;
    detector->active_level = active_level;
    detector->candidate_present = raw_level == active_level;
    /* A cup is never trusted immediately at boot. It must first remain stable
       for the configured insertion interval. */
    detector->stable_present = false;
    detector->candidate_since_ms = now_ms;
}

bool cup_detector_update(cup_detector_t *detector, bool raw_level,
                         uint32_t now_ms, uint32_t insert_debounce_ms,
                         uint32_t remove_debounce_ms) {
    if (detector == NULL) return false;
    const bool candidate_present = raw_level == detector->active_level;
    if (candidate_present != detector->candidate_present) {
        detector->candidate_present = candidate_present;
        detector->candidate_since_ms = now_ms;
        return false;
    }
    if (candidate_present == detector->stable_present) return false;
    const uint32_t debounce_ms = candidate_present ? insert_debounce_ms : remove_debounce_ms;
    if (now_ms - detector->candidate_since_ms < debounce_ms) return false;
    detector->stable_present = candidate_present;
    return true;
}

bool cup_detector_is_present(const cup_detector_t *detector) {
    return detector != NULL && detector->stable_present;
}
