#include "display.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "hardware/i2c.h"

#define OLED_WIDTH 128u
#define OLED_HEIGHT 64u
#define OLED_BUFFER_SIZE (OLED_WIDTH * OLED_HEIGHT / 8u)

static uint8_t framebuffer[OLED_BUFFER_SIZE];
static bool available;

/* Compact 5x7 glyphs: digits, uppercase letters, punctuation used by the UI. */
static const char glyph_chars[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.:%-_/";
static const uint8_t glyphs[][5] = {
    {0,0,0,0,0},{0x3e,0x51,0x49,0x45,0x3e},{0,0x42,0x7f,0x40,0},{0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4b,0x31},{0x18,0x14,0x12,0x7f,0x10},{0x27,0x45,0x45,0x45,0x39},
    {0x3c,0x4a,0x49,0x49,0x30},{1,0x71,9,5,3},{0x36,0x49,0x49,0x49,0x36},{6,0x49,0x49,0x29,0x1e},
    {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},
    {0x7f,0x41,0x41,0x22,0x1c},{0x7f,0x49,0x49,0x49,0x41},{0x7f,9,9,9,1},
    {0x3e,0x41,0x49,0x49,0x7a},{0x7f,8,8,8,0x7f},{0,0x41,0x7f,0x41,0},
    {0x20,0x40,0x41,0x3f,1},{0x7f,8,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
    {0x7f,2,0x0c,2,0x7f},{0x7f,4,8,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},
    {0x7f,9,9,9,6},{0x3e,0x41,0x51,0x21,0x5e},{0x7f,9,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},{1,1,0x7f,1,1},{0x3f,0x40,0x40,0x40,0x3f},
    {0x1f,0x20,0x40,0x20,0x1f},{0x3f,0x40,0x38,0x40,0x3f},{0x63,0x14,8,0x14,0x63},
    {7,8,0x70,8,7},{0x61,0x51,0x49,0x45,0x43},{0,0x60,0x60,0,0},
    {0x36,0x36,0,0,0},{0x23,0x13,8,0x64,0x62},{8,8,8,8,8},{8,0x1c,0x2a,8,8},
    {0x40,0x30,8,6,1}
};

static void command(uint8_t value) {
    const uint8_t bytes[] = {0x00u, value};
    i2c_write_blocking(I2C_PORT, SSD1306_ADDRESS, bytes, sizeof(bytes), false);
}

static const uint8_t *find_glyph(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    const char *found = strchr(glyph_chars, c);
    return found == NULL ? glyphs[0] : glyphs[found - glyph_chars];
}

static void text_at(uint8_t x, uint8_t page, const char *text) {
    while (*text != '\0' && x + 5u < OLED_WIDTH && page < 8u) {
        const uint8_t *glyph = find_glyph(*text++);
        for (uint8_t col = 0; col < 5u; ++col) framebuffer[page * OLED_WIDTH + x++] = glyph[col];
        framebuffer[page * OLED_WIDTH + x++] = 0u;
    }
}

static void flush(void) {
    uint8_t packet[OLED_WIDTH + 1u];
    packet[0] = 0x40u;
    for (uint8_t page = 0; page < 8u; ++page) {
        command((uint8_t)(0xB0u + page));
        command(0x00u);
        command(0x10u);
        memcpy(&packet[1], &framebuffer[page * OLED_WIDTH], OLED_WIDTH);
        i2c_write_blocking(I2C_PORT, SSD1306_ADDRESS, packet, sizeof(packet), false);
    }
}

bool display_init(void) {
    const uint8_t probe = 0x00u;
    available = i2c_write_blocking(I2C_PORT, SSD1306_ADDRESS, &probe, 1, false) == 1;
    if (!available) return false;
    const uint8_t init[] = {0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,0x40,0x8D,0x14,
                            0x20,0x00,0xA1,0xC8,0xDA,0x12,0x81,0x7F,0xD9,0xF1,
                            0xDB,0x40,0xA4,0xA6,0xAF};
    for (size_t i = 0; i < sizeof(init); ++i) command(init[i]);
    memset(framebuffer, 0, sizeof(framebuffer));
    flush();
    return true;
}

void display_set_dimmed(bool dimmed) {
    if (!available) return;
    command(0x81u);
    command(dimmed ? 0x10u : 0x7Fu);
}

void display_update(const system_status_t *status) {
    if (!available || status == NULL) return;
    char line[24];
    memset(framebuffer, 0, sizeof(framebuffer));
    text_at(24, 0, "BECHERHALTER");
    text_at(8, 1, "WIFI: " WIFI_AP_SSID);
    snprintf(line, sizeof(line), "IST:  %5.1F C", status->temperature_c);
    text_at(8, 2, line);
    snprintf(line, sizeof(line), "SOLL: %5.1F C", status->setpoint_c);
    text_at(8, 3, line);
    text_at(8, 4, "IP: " WIFI_AP_IP_ADDRESS);
    text_at(8, 5, system_state_name(status->state));
    if (status->state == SYSTEM_ERROR) {
        text_at(8, 6, error_name(status->error));
    } else {
        snprintf(line, sizeof(line), "LEISTUNG: %3.0F %%", status->peltier_power_percent);
        text_at(8, 6, line);
        snprintf(line, sizeof(line), "FAN: %4u RPM", (unsigned)status->fan_rpm);
        text_at(8, 7, line);
    }
    flush();
}
