#include "webserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "safety.h"
#include "web_assets.h"

#define HTTP_REQUEST_BUFFER_SIZE 1536u
#define HTTP_CLIENT_COUNT 4u

typedef struct {
    char data[HTTP_REQUEST_BUFFER_SIZE];
    size_t length;
    bool in_use;
} http_client_t;

static webserver_config_t server_config;
static struct tcp_pcb *listener;
static http_client_t http_clients[HTTP_CLIENT_COUNT];
static bool wifi_initialized;
static bool connect_in_progress;
static uint32_t connection_started_ms;
static uint32_t retry_due_ms;
static char response[3072];

static uint32_t milliseconds(void) {
    return to_ms_since_boot(get_absolute_time());
}

static bool deadline_reached(uint32_t now, uint32_t deadline) {
    return (int32_t)(now - deadline) >= 0;
}

static void start_wifi_connection(uint32_t now) {
    const int result = cyw43_arch_wifi_connect_async(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK);
    connect_in_progress = result == 0;
    connection_started_ms = now;
    if (!connect_in_progress) retry_due_ms = now + WIFI_RETRY_DELAY_MS;
}

static http_client_t *allocate_http_client(void) {
    for (size_t i = 0; i < HTTP_CLIENT_COUNT; ++i) {
        if (!http_clients[i].in_use) {
            http_clients[i].in_use = true;
            http_clients[i].length = 0u;
            http_clients[i].data[0] = '\0';
            return &http_clients[i];
        }
    }
    return NULL;
}

static void release_http_client(http_client_t *context) {
    if (context != NULL) {
        context->length = 0u;
        context->data[0] = '\0';
        context->in_use = false;
    }
}

static const char *error_description(error_code_t error) {
    switch (error) {
        case ERROR_NONE: return "Keine Fehler erkannt";
        case ERROR_TEMP_SENSOR: return "Temperatursensor fehlerhaft";
        case ERROR_OVERTEMPERATURE: return "Maximale sichere Temperatur ueberschritten";
        case ERROR_FAN: return "Luefterdrehzahl nicht plausibel";
        case ERROR_OVERCURRENT: return "Ueberstrom an einem Peltierkanal";
        case ERROR_CURRENT_SENSOR: return "Strommessung nicht verfuegbar";
        case ERROR_CUP_REMOVED: return "Becher wurde entfernt";
        case ERROR_POWER_SUPPLY: return "5V-Leistungsversorgung fehlt";
        default: return "Unbekannter Systemfehler";
    }
}

static bool start_allowed(void) {
    const system_status_t *s = server_config.status;
    return (s->state == SYSTEM_READY || s->state == SYSTEM_OFF) && safety_can_start(s);
}

static const char *start_block_reason(void) {
    const system_status_t *s = server_config.status;
    if (s->state == SYSTEM_HEATING || s->state == SYSTEM_HOLDING) return "Heizvorgang ist bereits aktiv";
    if (s->error != ERROR_NONE) return error_description(s->error);
    if (!s->temperature_valid) return "Temperatursensor fehlerhaft";
    if (!s->current_valid) return "Strommessung nicht verfuegbar";
    if (!s->cup_detected) return "Kein Becher erkannt";
    if (!s->power_5v_ok) return "5V-Leistungsversorgung fehlt";
    return "Start ist freigegeben";
}

static err_t send_response(struct tcp_pcb *client, const char *status, const char *type,
                           const char *body) {
    const size_t body_length = strlen(body);
    char header[192];
    const int header_length = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %u\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n",
        status, type, (unsigned)body_length);
    err_t error = tcp_write(client, header, (u16_t)header_length, TCP_WRITE_FLAG_COPY);
    const u8_t body_flags = body == dashboard_html ? 0u : TCP_WRITE_FLAG_COPY;
    if (error == ERR_OK) error = tcp_write(client, body, (u16_t)body_length, body_flags);
    tcp_output(client);
    tcp_close(client);
    return error;
}

static void status_json(void) {
    const system_status_t *s = server_config.status;
    char ip[16];
    (void)webserver_get_ip(ip, sizeof(ip));
    snprintf(response, sizeof(response),
        "{\"state\":\"%s\",\"fault\":\"%s\",\"fault_description\":\"%s\","
        "\"temperature\":%.2f,\"temperature1\":%.2f,\"temperature2\":%.2f,"
        "\"setpoint\":%.2f,\"error\":%.2f,\"power\":%.1f,\"fan_rpm\":%u,\"fan_percent\":%u,"
        "\"current1\":%.3f,\"current2\":%.3f,\"light_level\":%.3f,"
        "\"sensor_ok\":%s,\"temp1_ok\":%s,\"temp2_ok\":%s,"
        "\"current_ok\":%s,\"current1_ok\":%s,\"current2_ok\":%s,"
        "\"tla2024_ok\":%s,\"light_ok\":%s,\"display_initialized\":%s,\"leds_initialized\":%s,"
        "\"cup\":%s,\"power_good\":%s,\"wifi\":%s,\"wifi_ssid\":\"%s\",\"wifi_ip\":\"%s\",\"night\":%s,"
        "\"start_allowed\":%s,\"start_block_reason\":\"%s\"}",
        system_state_name(s->state), error_name(s->error), error_description(s->error),
        s->temperature_c, s->temperature_1_c, s->temperature_2_c,
        s->setpoint_c, s->control_error_c, s->peltier_power_percent, s->fan_rpm,
        s->fan_percent, s->peltier_1_current_a, s->peltier_2_current_a, s->light_level,
        s->temperature_valid ? "true" : "false",
        s->temperature_1_valid ? "true" : "false", s->temperature_2_valid ? "true" : "false",
        s->current_valid ? "true" : "false",
        s->current_1_valid ? "true" : "false", s->current_2_valid ? "true" : "false",
        s->tla2024_available ? "true" : "false", s->light_sensor_available ? "true" : "false",
        s->display_initialized ? "true" : "false", s->status_leds_initialized ? "true" : "false",
        s->cup_detected ? "true" : "false", s->power_5v_ok ? "true" : "false",
        s->wifi_connected ? "true" : "false", WIFI_SSID, ip,
        s->night_mode ? "true" : "false", start_allowed() ? "true" : "false",
        start_block_reason());
}

static err_t process_http_request(struct tcp_pcb *client, const char *text) {
    if (strncmp(text, "GET /api/status ", 16u) == 0) {
        status_json();
        return send_response(client, "200 OK", "application/json", response);
    }
    if (strncmp(text, "POST /api/start ", 16u) == 0) {
        if (start_allowed()) {
            server_config.start();
            return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
        }
        snprintf(response, sizeof(response), "{\"ok\":false,\"reason\":\"%s\"}",
                 start_block_reason());
        return send_response(client, "409 Conflict", "application/json", response);
    }
    if (strncmp(text, "POST /api/stop ", 15u) == 0) {
        server_config.stop();
        return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
    }
    static const char setpoint_prefix[] = "POST /api/setpoint?value=";
    if (strncmp(text, setpoint_prefix, sizeof(setpoint_prefix) - 1u) == 0) {
        const float value = strtof(text + sizeof(setpoint_prefix) - 1u, NULL);
        if (value >= SETPOINT_MIN_C && value <= SETPOINT_MAX_C) {
            server_config.set_setpoint(value);
            return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
        }
        return send_response(client, "400 Bad Request", "application/json", "{\"ok\":false}");
    }
    if (strncmp(text, "GET / ", 6u) == 0) {
        return send_response(client, "200 OK", "text/html; charset=utf-8", dashboard_html);
    }
    return send_response(client, "404 Not Found", "text/plain", "Not found");
}

static bool request_is_complete(const char *request) {
    return strstr(request, "\r\n\r\n") != NULL || strstr(request, "\n\n") != NULL;
}

static void client_error(void *arg, err_t error) {
    (void)error;
    release_http_client(arg);
}

static err_t receive(void *arg, struct tcp_pcb *client, struct pbuf *packet, err_t error) {
    http_client_t *context = arg;
    if (packet == NULL) {
        release_http_client(context);
        tcp_arg(client, NULL);
        tcp_close(client);
        return ERR_OK;
    }
    if (error != ERR_OK || context == NULL) {
        pbuf_free(packet);
        release_http_client(context);
        tcp_arg(client, NULL);
        tcp_abort(client);
        return ERR_ABRT;
    }

    const size_t available = sizeof(context->data) - 1u - context->length;
    if (packet->tot_len > available) {
        tcp_recved(client, packet->tot_len);
        pbuf_free(packet);
        release_http_client(context);
        tcp_arg(client, NULL);
        return send_response(client, "431 Request Header Fields Too Large", "text/plain",
                             "Request headers too large");
    }

    const u16_t copied = pbuf_copy_partial(packet, context->data + context->length,
                                           packet->tot_len, 0u);
    context->length += copied;
    context->data[context->length] = '\0';
    tcp_recved(client, packet->tot_len);
    pbuf_free(packet);
    if (!request_is_complete(context->data)) return ERR_OK;

    tcp_arg(client, NULL);
    const err_t result = process_http_request(client, context->data);
    release_http_client(context);
    return result;
}

static err_t accept_client(void *arg, struct tcp_pcb *client, err_t error) {
    (void)arg;
    if (error != ERR_OK || client == NULL) return ERR_VAL;
    http_client_t *context = allocate_http_client();
    if (context == NULL) {
        tcp_abort(client);
        return ERR_ABRT;
    }
    tcp_arg(client, context);
    tcp_recv(client, receive);
    tcp_err(client, client_error);
    return ERR_OK;
}

bool webserver_init(const webserver_config_t *config) {
    if (config == NULL || config->status == NULL || config->start == NULL ||
        config->stop == NULL || config->set_setpoint == NULL) return false;
    server_config = *config;
    memset(http_clients, 0, sizeof(http_clients));
    if (cyw43_arch_init() != 0) return false;
    wifi_initialized = true;

    /* WLAN initialization never grants heating permission. That remains solely
       controlled by the application state machine and safety module. */
    cyw43_arch_enable_sta_mode();

    cyw43_arch_lwip_begin();
    listener = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (listener != NULL && tcp_bind(listener, IP_ANY_TYPE, 80u) == ERR_OK) {
        listener = tcp_listen_with_backlog(listener, HTTP_CLIENT_COUNT);
        if (listener != NULL) tcp_accept(listener, accept_client);
    } else if (listener != NULL) {
        tcp_close(listener);
        listener = NULL;
    }
    cyw43_arch_lwip_end();
    if (listener == NULL) {
        webserver_deinit();
        return false;
    }

    start_wifi_connection(milliseconds());
    return true;
}

void webserver_update(void) {
    if (!wifi_initialized) return;
    const uint32_t now = milliseconds();
    const int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

    if (link_status == CYW43_LINK_UP) {
        connect_in_progress = false;
        return;
    }
    if (link_status == CYW43_LINK_JOIN || link_status == CYW43_LINK_NOIP) {
        if (!connect_in_progress) {
            connect_in_progress = true;
            connection_started_ms = now;
        }
        if (!deadline_reached(now, connection_started_ms + WIFI_CONNECT_TIMEOUT_MS)) return;
    } else if (connect_in_progress && link_status >= CYW43_LINK_DOWN &&
               !deadline_reached(now, connection_started_ms + WIFI_CONNECT_TIMEOUT_MS)) {
        return;
    }

    if (connect_in_progress || link_status < CYW43_LINK_DOWN) {
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
        connect_in_progress = false;
        retry_due_ms = now + WIFI_RETRY_DELAY_MS;
        return;
    }
    if (deadline_reached(now, retry_due_ms)) start_wifi_connection(now);
}

bool webserver_is_connected(void) {
    return wifi_initialized && listener != NULL &&
           cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

bool webserver_get_ip(char *buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0u) return false;
    snprintf(buffer, buffer_size, "--");
    if (!webserver_is_connected()) return false;

    ip4_addr_t address;
    cyw43_arch_lwip_begin();
    ip4_addr_copy(address, *netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]));
    cyw43_arch_lwip_end();
    if (ip4_addr_isany_val(address)) return false;
    return ip4addr_ntoa_r(&address, buffer, (int)buffer_size) != NULL;
}

void webserver_deinit(void) {
    if (listener != NULL) {
        cyw43_arch_lwip_begin();
        tcp_close(listener);
        cyw43_arch_lwip_end();
    }
    listener = NULL;
    if (wifi_initialized) {
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
        cyw43_arch_deinit();
    }
    wifi_initialized = false;
    connect_in_progress = false;
}
