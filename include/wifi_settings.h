#pragma once

#include <stdbool.h>
#include <stddef.h>

#define WIFI_SETTINGS_SSID_MAX 32u
#define WIFI_SETTINGS_PASSWORD_MAX 63u
#define WIFI_SETTINGS_PROFILE_COUNT 4u

typedef struct {
    char ssid[WIFI_SETTINGS_SSID_MAX + 1u];
    char password[WIFI_SETTINGS_PASSWORD_MAX + 1u];
} wifi_settings_profile_t;

/* Loads the user-selected network, or the compiled fallback credentials. */
bool wifi_settings_load(char *ssid, size_t ssid_size, char *password, size_t password_size);
/* Replaces the complete credential record in the reserved flash sector. */
bool wifi_settings_save(const char *ssid, const char *password);
/* Returns all locally stored profiles. Passwords are only for internal firmware use. */
bool wifi_settings_profiles(wifi_settings_profile_t *profiles, size_t capacity,
                            size_t *count, size_t *active_index);
/* Adds or replaces a profile and makes it the active network. */
bool wifi_settings_add(const char *ssid, const char *password);
bool wifi_settings_select(const char *ssid);
bool wifi_settings_remove(const char *ssid);
