#include "wifi_settings.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"
#include "secrets.h"

#define WIFI_SETTINGS_MAGIC 0x57494649u
#define WIFI_SETTINGS_VERSION 2u
#define WIFI_SETTINGS_LEGACY_VERSION 1u
#define WIFI_SETTINGS_FLASH_OFFSET (2u * 1024u * 1024u - FLASH_SECTOR_SIZE)

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    char ssid[WIFI_SETTINGS_SSID_MAX + 1u];
    char password[WIFI_SETTINGS_PASSWORD_MAX + 1u];
    uint32_t checksum;
} wifi_settings_record_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t active_index;
    wifi_settings_profile_t profiles[WIFI_SETTINGS_PROFILE_COUNT];
    uint32_t checksum;
} wifi_settings_record_v2_t;

static uint32_t checksum_bytes(const void *record, size_t length) {
    const uint8_t *bytes = (const uint8_t *)record;
    uint32_t value = 2166136261u;
    for (size_t index = 0u; index < length; ++index) {
        value ^= bytes[index];
        value *= 16777619u;
    }
    return value;
}

static uint32_t checksum(const wifi_settings_record_t *record) {
    return checksum_bytes(record, offsetof(wifi_settings_record_t, checksum));
}

static uint32_t checksum_v2(const wifi_settings_record_v2_t *record) {
    return checksum_bytes(record, offsetof(wifi_settings_record_v2_t, checksum));
}

static bool valid(const wifi_settings_record_t *record) {
    return record->magic == WIFI_SETTINGS_MAGIC && record->version == WIFI_SETTINGS_LEGACY_VERSION &&
           record->ssid[WIFI_SETTINGS_SSID_MAX] == '\0' &&
           record->password[WIFI_SETTINGS_PASSWORD_MAX] == '\0' &&
           record->ssid[0] != '\0' && record->password[0] != '\0' &&
           record->checksum == checksum(record);
}

static bool valid_v2(const wifi_settings_record_v2_t *record) {
    if (record->magic != WIFI_SETTINGS_MAGIC || record->version != WIFI_SETTINGS_VERSION ||
        record->active_index >= WIFI_SETTINGS_PROFILE_COUNT || record->checksum != checksum_v2(record)) return false;
    for (size_t index = 0u; index < WIFI_SETTINGS_PROFILE_COUNT; ++index) {
        const wifi_settings_profile_t *profile = &record->profiles[index];
        if (profile->ssid[WIFI_SETTINGS_SSID_MAX] != '\0' ||
            profile->password[WIFI_SETTINGS_PASSWORD_MAX] != '\0') return false;
    }
    return record->profiles[record->active_index].ssid[0] != '\0' &&
           record->profiles[record->active_index].password[0] != '\0';
}

static const wifi_settings_record_v2_t *current_record(void) {
    return (const wifi_settings_record_v2_t *)(XIP_BASE + WIFI_SETTINGS_FLASH_OFFSET);
}

static void write_record(wifi_settings_record_v2_t *record) {
    uint8_t sector[FLASH_SECTOR_SIZE];
    memset(sector, 0xff, sizeof(sector));
    record->magic = WIFI_SETTINGS_MAGIC;
    record->version = WIFI_SETTINGS_VERSION;
    record->checksum = checksum_v2(record);
    memcpy(sector, record, sizeof(*record));
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(WIFI_SETTINGS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(WIFI_SETTINGS_FLASH_OFFSET, sector, FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
}

static bool valid_credentials(const char *ssid, const char *password) {
    return ssid != NULL && password != NULL && ssid[0] != '\0' && password[0] != '\0' &&
           strlen(ssid) <= WIFI_SETTINGS_SSID_MAX && strlen(password) <= WIFI_SETTINGS_PASSWORD_MAX;
}

static void record_from_current(wifi_settings_record_v2_t *destination) {
    memset(destination, 0, sizeof(*destination));
    const wifi_settings_record_v2_t *stored = current_record();
    if (valid_v2(stored)) {
        memcpy(destination, stored, sizeof(*destination));
        return;
    }
    const wifi_settings_record_t *legacy = (const wifi_settings_record_t *)stored;
    if (valid(legacy)) {
        snprintf(destination->profiles[0].ssid, sizeof(destination->profiles[0].ssid), "%s", legacy->ssid);
        snprintf(destination->profiles[0].password, sizeof(destination->profiles[0].password), "%s", legacy->password);
    }
}

bool wifi_settings_load(char *ssid, size_t ssid_size, char *password, size_t password_size) {
    if (ssid == NULL || password == NULL || ssid_size == 0u || password_size == 0u) return false;
    const wifi_settings_record_v2_t *record_v2 = current_record();
    if (valid_v2(record_v2)) {
        const wifi_settings_profile_t *profile = &record_v2->profiles[record_v2->active_index];
        snprintf(ssid, ssid_size, "%s", profile->ssid);
        snprintf(password, password_size, "%s", profile->password);
        return true;
    }
    const wifi_settings_record_t *record = (const wifi_settings_record_t *)record_v2;
    if (valid(record)) {
        snprintf(ssid, ssid_size, "%s", record->ssid);
        snprintf(password, password_size, "%s", record->password);
        return true;
    }
    snprintf(ssid, ssid_size, "%s", WIFI_SSID);
    snprintf(password, password_size, "%s", WIFI_PASSWORD);
    return false;
}

bool wifi_settings_save(const char *ssid, const char *password) {
    return wifi_settings_add(ssid, password);
}

bool wifi_settings_profiles(wifi_settings_profile_t *profiles, size_t capacity,
                            size_t *count, size_t *active_index) {
    if (count == NULL || active_index == NULL) return false;
    const wifi_settings_record_v2_t *record = current_record();
    if (!valid_v2(record)) { *count = 0u; *active_index = 0u; return false; }
    size_t used = 0u;
    for (size_t index = 0u; index < WIFI_SETTINGS_PROFILE_COUNT; ++index) {
        if (record->profiles[index].ssid[0] == '\0') continue;
        if (profiles != NULL && used < capacity) profiles[used] = record->profiles[index];
        if (index == record->active_index) *active_index = used;
        ++used;
    }
    *count = used;
    return true;
}

bool wifi_settings_add(const char *ssid, const char *password) {
    if (!valid_credentials(ssid, password)) return false;
    wifi_settings_record_v2_t record;
    record_from_current(&record);
    size_t slot = WIFI_SETTINGS_PROFILE_COUNT;
    for (size_t index = 0u; index < WIFI_SETTINGS_PROFILE_COUNT; ++index) {
        if (strcmp(record.profiles[index].ssid, ssid) == 0) { slot = index; break; }
        if (slot == WIFI_SETTINGS_PROFILE_COUNT && record.profiles[index].ssid[0] == '\0') slot = index;
    }
    if (slot == WIFI_SETTINGS_PROFILE_COUNT) return false;
    snprintf(record.profiles[slot].ssid, sizeof(record.profiles[slot].ssid), "%s", ssid);
    snprintf(record.profiles[slot].password, sizeof(record.profiles[slot].password), "%s", password);
    record.active_index = (uint16_t)slot;
    write_record(&record);
    return true;
}

bool wifi_settings_select(const char *ssid) {
    if (ssid == NULL || ssid[0] == '\0') return false;
    wifi_settings_record_v2_t record;
    record_from_current(&record);
    for (size_t index = 0u; index < WIFI_SETTINGS_PROFILE_COUNT; ++index) {
        if (strcmp(record.profiles[index].ssid, ssid) == 0 && record.profiles[index].password[0] != '\0') {
            record.active_index = (uint16_t)index;
            write_record(&record);
            return true;
        }
    }
    return false;
}

bool wifi_settings_remove(const char *ssid) {
    if (ssid == NULL || ssid[0] == '\0') return false;
    wifi_settings_record_v2_t record;
    record_from_current(&record);
    size_t removed = WIFI_SETTINGS_PROFILE_COUNT, remaining = 0u, first = WIFI_SETTINGS_PROFILE_COUNT;
    for (size_t index = 0u; index < WIFI_SETTINGS_PROFILE_COUNT; ++index) {
        if (strcmp(record.profiles[index].ssid, ssid) == 0) removed = index;
        else if (record.profiles[index].ssid[0] != '\0') { ++remaining; if (first == WIFI_SETTINGS_PROFILE_COUNT) first = index; }
    }
    if (removed == WIFI_SETTINGS_PROFILE_COUNT || remaining == 0u) return false;
    memset(&record.profiles[removed], 0, sizeof(record.profiles[removed]));
    if (record.active_index == removed) record.active_index = (uint16_t)first;
    write_record(&record);
    return true;
}
