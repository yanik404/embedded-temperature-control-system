#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define WEB_AUTH_TOKEN_LENGTH 16u
#define WEB_AUTH_SESSION_COUNT 4u

typedef struct {
    char token[WEB_AUTH_TOKEN_LENGTH + 1u];
    uint32_t expires_ms;
    bool active;
} web_auth_session_t;

typedef struct {
    web_auth_session_t sessions[WEB_AUTH_SESSION_COUNT];
} web_auth_t;

void web_auth_init(web_auth_t *auth);
bool web_auth_unlock(web_auth_t *auth, const char *pin, const char *expected_pin,
                     uint32_t now_ms, uint32_t timeout_ms, uint64_t entropy,
                     char *token, size_t token_size);
bool web_auth_validate(web_auth_t *auth, const char *token, uint32_t now_ms);
uint32_t web_auth_remaining_ms(web_auth_t *auth, const char *token, uint32_t now_ms);
bool web_auth_has_active(web_auth_t *auth, uint32_t now_ms);
