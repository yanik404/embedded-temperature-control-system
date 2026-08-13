#include "web_auth.h"

#include <stdio.h>
#include <string.h>

static bool deadline_reached(uint32_t now, uint32_t deadline) {
    return (int32_t)(now - deadline) >= 0;
}

static bool constant_time_equal(const char *left, const char *right) {
    if (left == NULL || right == NULL) return false;
    const size_t left_length = strlen(left);
    const size_t right_length = strlen(right);
    const size_t count = left_length > right_length ? left_length : right_length;
    unsigned difference = (unsigned)(left_length ^ right_length);
    for (size_t i = 0; i < count; ++i) {
        const unsigned a = i < left_length ? (unsigned char)left[i] : 0u;
        const unsigned b = i < right_length ? (unsigned char)right[i] : 0u;
        difference |= a ^ b;
    }
    return difference == 0u;
}

static void expire_sessions(web_auth_t *auth, uint32_t now_ms) {
    for (size_t i = 0; i < WEB_AUTH_SESSION_COUNT; ++i) {
        web_auth_session_t *session = &auth->sessions[i];
        if (session->active && deadline_reached(now_ms, session->expires_ms)) {
            memset(session, 0, sizeof(*session));
        }
    }
}

void web_auth_init(web_auth_t *auth) {
    if (auth != NULL) memset(auth, 0, sizeof(*auth));
}

bool web_auth_unlock(web_auth_t *auth, const char *pin, const char *expected_pin,
                     uint32_t now_ms, uint32_t timeout_ms, uint64_t entropy,
                     char *token, size_t token_size) {
    if (auth == NULL || token == NULL || token_size < WEB_AUTH_TOKEN_LENGTH + 1u ||
        timeout_ms == 0u || !constant_time_equal(pin, expected_pin)) return false;

    expire_sessions(auth, now_ms);
    size_t selected = 0u;
    for (size_t i = 0; i < WEB_AUTH_SESSION_COUNT; ++i) {
        if (!auth->sessions[i].active) {
            selected = i;
            break;
        }
        if ((int32_t)(auth->sessions[i].expires_ms - auth->sessions[selected].expires_ms) < 0) {
            selected = i;
        }
    }

    /* Mix the hardware entropy with time and slot so even repeated entropy does
       not produce the same token during the same boot. */
    uint64_t mixed = entropy ^ ((uint64_t)now_ms << 32u) ^
                     (0x9E3779B97F4A7C15ull * (uint64_t)(selected + 1u));
    mixed ^= mixed >> 30u;
    mixed *= 0xBF58476D1CE4E5B9ull;
    mixed ^= mixed >> 27u;
    mixed *= 0x94D049BB133111EBull;
    mixed ^= mixed >> 31u;
    (void)snprintf(auth->sessions[selected].token,
                   sizeof(auth->sessions[selected].token), "%08lx%08lx",
                   (unsigned long)(mixed >> 32u), (unsigned long)mixed);
    auth->sessions[selected].expires_ms = now_ms + timeout_ms;
    auth->sessions[selected].active = true;
    memcpy(token, auth->sessions[selected].token, WEB_AUTH_TOKEN_LENGTH + 1u);
    return true;
}

bool web_auth_validate(web_auth_t *auth, const char *token, uint32_t now_ms) {
    if (auth == NULL || token == NULL) return false;
    expire_sessions(auth, now_ms);
    for (size_t i = 0; i < WEB_AUTH_SESSION_COUNT; ++i) {
        if (auth->sessions[i].active && constant_time_equal(auth->sessions[i].token, token)) {
            return true;
        }
    }
    return false;
}

uint32_t web_auth_remaining_ms(web_auth_t *auth, const char *token, uint32_t now_ms) {
    if (auth == NULL || token == NULL) return 0u;
    expire_sessions(auth, now_ms);
    for (size_t i = 0; i < WEB_AUTH_SESSION_COUNT; ++i) {
        if (auth->sessions[i].active && constant_time_equal(auth->sessions[i].token, token)) {
            return auth->sessions[i].expires_ms - now_ms;
        }
    }
    return 0u;
}

bool web_auth_has_active(web_auth_t *auth, uint32_t now_ms) {
    if (auth == NULL) return false;
    expire_sessions(auth, now_ms);
    for (size_t i = 0; i < WEB_AUTH_SESSION_COUNT; ++i) {
        if (auth->sessions[i].active) return true;
    }
    return false;
}
