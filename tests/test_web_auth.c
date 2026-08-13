#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "web_auth.h"

int main(void) {
    web_auth_t auth;
    char token[WEB_AUTH_TOKEN_LENGTH + 1u];
    web_auth_init(&auth);

    assert(!web_auth_unlock(&auth, "0000", "1234", 1000u, 300000u,
                            0x123456789abcdef0ull, token, sizeof(token)));
    assert(!web_auth_has_active(&auth, 1000u));
    assert(web_auth_unlock(&auth, "1234", "1234", 1000u, 300000u,
                           0x123456789abcdef0ull, token, sizeof(token)));
    assert(strlen(token) == WEB_AUTH_TOKEN_LENGTH);
    assert(web_auth_validate(&auth, token, 1000u));
    assert(!web_auth_validate(&auth, "invalid-token", 1000u));
    assert(web_auth_remaining_ms(&auth, token, 1000u) == 300000u);
    assert(web_auth_remaining_ms(&auth, token, 300999u) == 1u);
    assert(!web_auth_validate(&auth, token, 301000u));
    assert(!web_auth_has_active(&auth, 301000u));

    /* Deadline arithmetic stays correct across uint32 wraparound. */
    assert(web_auth_unlock(&auth, "1234", "1234", 0xfffffff0u, 100u,
                           0xfedcba9876543210ull, token, sizeof(token)));
    assert(web_auth_validate(&auth, token, 0x00000020u));
    assert(!web_auth_validate(&auth, token, 0x00000054u));

    puts("web auth tests passed");
    return 0;
}
