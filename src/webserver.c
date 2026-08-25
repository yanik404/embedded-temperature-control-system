#include "webserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "safety.h"
#include "web_assets.h"
#include "web_auth.h"

#define HTTP_REQUEST_BUFFER_SIZE 1536u
#define HTTP_CLIENT_COUNT 4u
#define HTTP_RESPONSE_HEADER_SIZE 192u
#define HTTP_RESPONSE_CHUNK_SIZE TCP_MSS

typedef struct {
    char data[HTTP_REQUEST_BUFFER_SIZE];
    char response_header[HTTP_RESPONSE_HEADER_SIZE];
    const char *response_body;
    size_t length;
    size_t response_header_length;
    size_t response_body_length;
    size_t response_queued;
    size_t response_acked;
    bool in_use;
    bool response_active;
} http_client_t;

static webserver_config_t server_config;
static struct tcp_pcb *listener;
static http_client_t http_clients[HTTP_CLIENT_COUNT];
static bool wifi_initialized;
static bool connect_in_progress;
static bool first_connection_attempt = true;
static int previous_link_status = CYW43_LINK_DOWN;
static uint32_t connection_started_ms;
static uint32_t retry_due_ms;
static web_auth_t control_auth;

static uint32_t milliseconds(void) {
    return to_ms_since_boot(get_absolute_time());
}

static bool deadline_reached(uint32_t now, uint32_t deadline) {
    return (int32_t)(now - deadline) >= 0;
}

static const char *wifi_status_name(int status) {
    switch (status) {
        case CYW43_LINK_UP: return "verbunden";
        case CYW43_LINK_NOIP: return "verbunden, keine IP-Adresse";
        case CYW43_LINK_JOIN: return "WLAN verbunden, DHCP ausstehend";
        case CYW43_LINK_DOWN: return "getrennt";
        case CYW43_LINK_FAIL: return "Verbindung fehlgeschlagen";
        case CYW43_LINK_NONET: return "SSID nicht gefunden";
        case CYW43_LINK_BADAUTH: return "Authentifizierung fehlgeschlagen";
        default: return "unbekannt";
    }
}

static void start_wifi_connection(uint32_t now) {
    if (!first_connection_attempt) printf("[WLAN] Erneuter Verbindungsversuch\n");
    printf("[WLAN] WLAN verbindet...\n");
    printf("[WLAN] SSID: %s\n", WIFI_SSID);
    const int result = cyw43_arch_wifi_connect_async(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK);
    first_connection_attempt = false;
    connect_in_progress = result == 0;
    connection_started_ms = now;
    if (!connect_in_progress) {
        printf("[WLAN] Verbindungsfehler (Fehlercode %d)\n", result);
        retry_due_ms = now + WIFI_RETRY_DELAY_MS;
    }
}

static http_client_t *allocate_http_client(void) {
    for (size_t i = 0; i < HTTP_CLIENT_COUNT; ++i) {
        if (!http_clients[i].in_use) {
            memset(&http_clients[i], 0, sizeof(http_clients[i]));
            http_clients[i].in_use = true;
            return &http_clients[i];
        }
    }
    return NULL;
}

static void release_http_client(http_client_t *context) {
    if (context != NULL) memset(context, 0, sizeof(*context));
}

static const char *error_description(error_code_t error) {
    switch (error) {
        case ERROR_NONE: return "Keine Fehler erkannt";
        case ERROR_TEMP_SENSOR: return "Temperatursensor fehlerhaft";
        case ERROR_OVERTEMPERATURE: return "Maximale sichere Temperatur ueberschritten";
        case ERROR_UNDERTEMPERATURE: return "Minimale sichere Temperatur unterschritten";
        case ERROR_FAN: return "Luefterdrehzahl nicht plausibel";
        case ERROR_OVERCURRENT: return "Ueberstrom an einem Peltierkanal";
        case ERROR_CURRENT_SENSOR: return "Strommessung nicht verfuegbar";
        case ERROR_CUP_REMOVED: return "Becher wurde entfernt";
        case ERROR_POWER_SUPPLY: return "5V-Leistungsversorgung fehlt";
        default: return "Unbekannter Systemfehler";
    }
}

static const char *thermal_output_name(thermal_output_mode_t mode) {
    switch (mode) {
        case THERMAL_OUTPUT_HEATING: return "HEIZEN";
        case THERMAL_OUTPUT_COOLING: return "KUEHLEN";
        case THERMAL_OUTPUT_OFF:
        default: return "AUS";
    }
}

static bool start_allowed(void) {
    const system_status_t *s = server_config.status;
    return (s->state == SYSTEM_READY || s->state == SYSTEM_OFF) && safety_can_start(s);
}

static const char *start_block_reason(void) {
    const system_status_t *s = server_config.status;
    if (s->state == SYSTEM_HEATING || s->state == SYSTEM_COOLING ||
        s->state == SYSTEM_HOLDING) return "Temperaturregelung ist bereits aktiv";
    if (s->error != ERROR_NONE) return error_description(s->error);
    if (!s->temperature_valid) return "Temperatursensor fehlerhaft";
    if (!s->current_valid) return "Strommessung nicht verfuegbar";
    if (!s->cup_detected) return "Kein Becher erkannt";
    if (!s->power_5v_ok) return "5V-Leistungsversorgung fehlt";
    return "Start ist freigegeben";
}

static const char *request_body(const char *request) {
    const char *body = strstr(request, "\r\n\r\n");
    if (body != NULL) return body + 4u;
    body = strstr(request, "\n\n");
    return body != NULL ? body + 2u : NULL;
}

static size_t request_header_length(const char *request) {
    const char *body = strstr(request, "\r\n\r\n");
    if (body != NULL) return (size_t)(body + 4u - request);
    body = strstr(request, "\n\n");
    return body != NULL ? (size_t)(body + 2u - request) : 0u;
}

static size_t request_content_length(const char *request) {
    static const char name[] = "content-length";
    const char *line = strchr(request, '\n');
    while (line != NULL && *++line != '\0' && *line != '\r' && *line != '\n') {
        size_t i = 0u;
        while (i < sizeof(name) - 1u) {
            char value = line[i];
            if (value >= 'A' && value <= 'Z') value = (char)(value + ('a' - 'A'));
            if (value != name[i]) break;
            ++i;
        }
        if (i == sizeof(name) - 1u && line[i] == ':') {
            return (size_t)strtoul(line + i + 1u, NULL, 10);
        }
        line = strchr(line, '\n');
    }
    return 0u;
}

static bool form_value(const char *body, const char *name, char *value, size_t value_size) {
    if (body == NULL || name == NULL || value == NULL || value_size == 0u) return false;
    const size_t name_length = strlen(name);
    const char *cursor = body;
    while (*cursor != '\0') {
        if ((cursor == body || cursor[-1] == '&') && strncmp(cursor, name, name_length) == 0 &&
            cursor[name_length] == '=') {
            cursor += name_length + 1u;
            size_t written = 0u;
            while (*cursor != '\0' && *cursor != '&' && written + 1u < value_size) {
                value[written++] = *cursor++;
            }
            value[written] = '\0';
            return true;
        }
        cursor = strchr(cursor, '&');
        if (cursor == NULL) break;
        ++cursor;
    }
    value[0] = '\0';
    return false;
}

static bool request_token(const char *request, char *token, size_t token_size) {
    return form_value(request_body(request), "token", token, token_size);
}

static bool request_authorized(const char *request) {
    char token[WEB_AUTH_TOKEN_LENGTH + 1u];
    return request_token(request, token, sizeof(token)) &&
           web_auth_validate(&control_auth, token, milliseconds());
}

static err_t send_response(http_client_t *context, struct tcp_pcb *client,
                           const char *status, const char *type, const char *body);

static err_t send_unauthorized(http_client_t *context, struct tcp_pcb *client) {
    return send_response(context, client, "401 Unauthorized", "application/json",
                         "{\"ok\":false,\"reason\":\"Steuerung gesperrt\"}");
}

static err_t send_response_data(http_client_t *context, struct tcp_pcb *client);
static err_t response_sent(void *arg, struct tcp_pcb *client, u16_t acknowledged);
static err_t response_poll(void *arg, struct tcp_pcb *client);

static err_t abort_http_client(http_client_t *context, struct tcp_pcb *client,
                               err_t error, const char *reason) {
    printf("[HTTP] TCP-Abbruch: %s (Fehlercode %d)\n", reason, error);
    tcp_arg(client, NULL);
    tcp_recv(client, NULL);
    tcp_sent(client, NULL);
    tcp_poll(client, NULL, 0u);
    tcp_err(client, NULL);
    release_http_client(context);
    tcp_abort(client);
    return ERR_ABRT;
}

static err_t close_completed_response(http_client_t *context, struct tcp_pcb *client) {
    const size_t total = context->response_header_length + context->response_body_length;
    if (context->response_queued < total || context->response_acked < total) return ERR_OK;

    printf("[HTTP] Response vollstaendig gesendet (%u Bytes)\n", (unsigned)total);
    tcp_arg(client, NULL);
    const err_t error = tcp_close(client);
    if (error == ERR_OK) {
        printf("[HTTP] Verbindung geschlossen\n");
        release_http_client(context);
        return ERR_OK;
    }
    tcp_arg(client, context);
    if (error == ERR_MEM) {
        printf("[HTTP] tcp_close ERR_MEM, spaeterer Versuch\n");
        return ERR_OK;
    }
    return abort_http_client(context, client, error, "tcp_close fehlgeschlagen");
}

static err_t send_response_data(http_client_t *context, struct tcp_pcb *client) {
    const size_t total = context->response_header_length + context->response_body_length;

    while (context->response_queued < total) {
        const u16_t available = tcp_sndbuf(client);
        if (available == 0u) break;

        const char *source;
        size_t segment_remaining;
        if (context->response_queued < context->response_header_length) {
            source = context->response_header + context->response_queued;
            segment_remaining = context->response_header_length - context->response_queued;
        } else {
            const size_t body_offset = context->response_queued - context->response_header_length;
            source = context->response_body + body_offset;
            segment_remaining = context->response_body_length - body_offset;
        }

        size_t chunk = segment_remaining;
        if (chunk > HTTP_RESPONSE_CHUNK_SIZE) chunk = HTTP_RESPONSE_CHUNK_SIZE;
        if (chunk > available) chunk = available;
        const u8_t flags = context->response_queued + chunk < total ? TCP_WRITE_FLAG_MORE : 0u;
        const err_t error = tcp_write(client, source, (u16_t)chunk, flags);
        if (error == ERR_MEM) {
            printf("[HTTP] tcp_write ERR_MEM, %u Bytes verbleiben\n",
                   (unsigned)(total - context->response_queued));
            break;
        }
        if (error != ERR_OK) {
            return abort_http_client(context, client, error, "tcp_write fehlgeschlagen");
        }
        context->response_queued += chunk;
        printf("[HTTP] Gesendet/eingereiht: %u/%u Bytes, verbleibend: %u\n",
               (unsigned)context->response_queued, (unsigned)total,
               (unsigned)(total - context->response_queued));
    }

    const err_t output_error = tcp_output(client);
    if (output_error == ERR_MEM) {
        printf("[HTTP] tcp_output ERR_MEM, ACK/Poll wird abgewartet\n");
    }
    if (output_error != ERR_OK && output_error != ERR_MEM) {
        return abort_http_client(context, client, output_error, "tcp_output fehlgeschlagen");
    }
    return close_completed_response(context, client);
}

static err_t response_sent(void *arg, struct tcp_pcb *client, u16_t acknowledged) {
    http_client_t *context = arg;
    if (context == NULL || !context->response_active) {
        return abort_http_client(context, client, ERR_VAL, "ungueltiger Sendestatus");
    }
    context->response_acked += acknowledged;
    const size_t total = context->response_header_length + context->response_body_length;
    if (context->response_acked > total) context->response_acked = total;
    return send_response_data(context, client);
}

static err_t response_poll(void *arg, struct tcp_pcb *client) {
    http_client_t *context = arg;
    if (context == NULL || !context->response_active) {
        return abort_http_client(context, client, ERR_VAL, "ungueltiger Poll-Status");
    }
    return send_response_data(context, client);
}

static err_t send_response(http_client_t *context, struct tcp_pcb *client,
                           const char *status, const char *type, const char *body) {
    context->response_body = body;
    context->response_body_length = strlen(body);
    const int header_length = snprintf(context->response_header,
        sizeof(context->response_header),
        "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %u\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n",
        status, type, (unsigned)context->response_body_length);
    if (header_length < 0 || (size_t)header_length >= sizeof(context->response_header)) {
        return abort_http_client(context, client, ERR_VAL, "HTTP-Header zu gross");
    }

    context->response_header_length = (size_t)header_length;
    context->response_queued = 0u;
    context->response_acked = 0u;
    context->response_active = true;
    tcp_sent(client, response_sent);
    tcp_poll(client, response_poll, 2u);
    printf("[HTTP] Response-Groesse: %u Bytes (%u Header + %u Body)\n",
           (unsigned)(context->response_header_length + context->response_body_length),
           (unsigned)context->response_header_length,
           (unsigned)context->response_body_length);
    return send_response_data(context, client);
}

static void status_json(char *buffer, size_t buffer_size) {
    const system_status_t *s = server_config.status;
    char ip[16];
    (void)webserver_get_ip(ip, sizeof(ip));
    snprintf(buffer, buffer_size,
        "{\"state\":\"%s\",\"fault\":\"%s\",\"fault_description\":\"%s\","
        "\"temperature\":%.2f,\"temperature1\":%.2f,\"temperature2\":%.2f,"
        "\"setpoint\":%.2f,\"error\":%.2f,\"power\":%.1f,\"thermal_mode\":\"%s\","
        "\"fan_rpm\":%u,\"fan_percent\":%u,"
        "\"kp\":%.3f,\"ki\":%.3f,\"p_term\":%.2f,\"i_term\":%.2f,"
        "\"output_limited\":%s,\"anti_windup\":%s,\"control_period_ms\":%u,"
        "\"min_safe_temperature\":%.1f,\"max_safe_temperature\":%.1f,"
        "\"max_cooling_power\":%.1f,\"uptime_ms\":%llu,"
        "\"current1\":%.3f,\"current2\":%.3f,\"light_level\":%.3f,"
        "\"sensor_ok\":%s,\"temp1_ok\":%s,\"temp2_ok\":%s,"
        "\"current_ok\":%s,\"current1_ok\":%s,\"current2_ok\":%s,"
        "\"tla2024_ok\":%s,\"light_ok\":%s,\"display_initialized\":%s,\"leds_initialized\":%s,"
        "\"cup\":%s,\"power_good\":%s,\"wifi\":%s,\"webserver_ready\":%s,"
        "\"wifi_ssid\":\"%s\",\"wifi_ip\":\"%s\",\"night\":%s,"
        "\"start_allowed\":%s,\"start_block_reason\":\"%s\","
        "\"control_unlock_available\":true}",
        system_state_name(s->state), error_name(s->error), error_description(s->error),
        s->temperature_c, s->temperature_1_c, s->temperature_2_c,
        s->setpoint_c, s->control_error_c, s->peltier_power_percent,
        thermal_output_name(s->thermal_output_mode), s->fan_rpm,
        s->fan_percent, PI_KP, PI_KI, s->controller_proportional_percent,
        s->controller_integral_percent,
        s->controller_output_limited ? "true" : "false",
        s->controller_anti_windup_active ? "true" : "false", CONTROL_PERIOD_MS,
        MIN_SAFE_TEMPERATURE_C, MAX_SAFE_TEMPERATURE_C, PELTIER_MAX_COOLING_PERCENT,
        (unsigned long long)to_ms_since_boot(get_absolute_time()),
        s->peltier_1_current_a, s->peltier_2_current_a, s->light_level,
        s->temperature_valid ? "true" : "false",
        s->temperature_1_valid ? "true" : "false", s->temperature_2_valid ? "true" : "false",
        s->current_valid ? "true" : "false",
        s->current_1_valid ? "true" : "false", s->current_2_valid ? "true" : "false",
        s->tla2024_available ? "true" : "false", s->light_sensor_available ? "true" : "false",
        s->display_initialized ? "true" : "false", s->status_leds_initialized ? "true" : "false",
        s->cup_detected ? "true" : "false", s->power_5v_ok ? "true" : "false",
        s->wifi_connected ? "true" : "false", s->webserver_ready ? "true" : "false",
        WIFI_SSID, ip,
        s->night_mode ? "true" : "false", start_allowed() ? "true" : "false",
        start_block_reason());
}

static void log_request_path(const char *text) {
    const char *start = strchr(text, ' ');
    if (start == NULL) return;
    ++start;
    const char *end = strchr(start, ' ');
    if (end == NULL) return;
    const int length = (int)(end - start);
    printf("[HTTP] Request-Pfad: %.*s\n", length, start);
}

static err_t process_http_request(http_client_t *context, struct tcp_pcb *client,
                                  const char *text) {
    log_request_path(text);
    if (strncmp(text, "GET /api/status ", 16u) == 0) {
        status_json(context->data, sizeof(context->data));
        return send_response(context, client, "200 OK", "application/json", context->data);
    }
    if (strncmp(text, "POST /api/unlock ", 17u) == 0) {
        char pin[16];
        char token[WEB_AUTH_TOKEN_LENGTH + 1u];
        if (!form_value(request_body(text), "pin", pin, sizeof(pin)) ||
            !web_auth_unlock(&control_auth, pin, WEB_CONTROL_PIN, milliseconds(),
                             WEB_CONTROL_SESSION_MS, get_rand_64(), token, sizeof(token))) {
            printf("[HTTP] Bedienfreigabe abgelehnt\n");
            return send_response(context, client, "403 Forbidden", "application/json",
                                 "{\"ok\":false,\"reason\":\"PIN falsch\"}");
        }
        snprintf(context->data, sizeof(context->data),
                 "{\"ok\":true,\"token\":\"%s\",\"expires_in_ms\":%u}",
                 token, WEB_CONTROL_SESSION_MS);
        printf("[HTTP] Bedienfreigabe fuer %u Sekunden erteilt\n",
               (unsigned)(WEB_CONTROL_SESSION_MS / 1000u));
        return send_response(context, client, "200 OK", "application/json", context->data);
    }
    if (strncmp(text, "POST /api/start ", 16u) == 0) {
        if (!request_authorized(text)) return send_unauthorized(context, client);
        if (start_allowed()) {
            server_config.start();
            return send_response(context, client, "200 OK", "application/json", "{\"ok\":true}");
        }
        snprintf(context->data, sizeof(context->data), "{\"ok\":false,\"reason\":\"%s\"}",
                 start_block_reason());
        return send_response(context, client, "409 Conflict", "application/json", context->data);
    }
    if (strncmp(text, "POST /api/stop ", 15u) == 0) {
        /* STOP is deliberately always available: authentication must never
           prevent a user on the local network from requesting a safe stop. */
        server_config.stop();
        return send_response(context, client, "200 OK", "application/json", "{\"ok\":true}");
    }
    static const char setpoint_prefix[] = "POST /api/setpoint?value=";
    if (strncmp(text, setpoint_prefix, sizeof(setpoint_prefix) - 1u) == 0) {
        if (!request_authorized(text)) return send_unauthorized(context, client);
        const float value = strtof(text + sizeof(setpoint_prefix) - 1u, NULL);
        if (value >= SETPOINT_MIN_C && value <= SETPOINT_MAX_C) {
            server_config.set_setpoint(value);
            return send_response(context, client, "200 OK", "application/json", "{\"ok\":true}");
        }
        return send_response(context, client, "400 Bad Request", "application/json", "{\"ok\":false}");
    }
    if (strncmp(text, "GET / ", 6u) == 0) {
        return send_response(context, client, "200 OK", "text/html; charset=utf-8", dashboard_html);
    }
    return send_response(context, client, "404 Not Found", "text/plain", "Not found");
}

static bool request_is_complete(const char *request) {
    const size_t header_length = request_header_length(request);
    if (header_length == 0u) return false;
    return strlen(request) >= header_length + request_content_length(request);
}

static void client_error(void *arg, err_t error) {
    printf("[HTTP] TCP-Fehler/Abbruch (Fehlercode %d)\n", error);
    release_http_client(arg);
}

static err_t receive(void *arg, struct tcp_pcb *client, struct pbuf *packet, err_t error) {
    http_client_t *context = arg;
    if (context != NULL && context->response_active) {
        if (packet == NULL) {
            tcp_recv(client, NULL);
            return ERR_OK;
        }
        if (error != ERR_OK) {
            pbuf_free(packet);
            return abort_http_client(context, client, error, "Empfang waehrend Response");
        }
        tcp_recved(client, packet->tot_len);
        pbuf_free(packet);
        return ERR_OK;
    }
    if (packet == NULL) {
        release_http_client(context);
        tcp_arg(client, NULL);
        if (tcp_close(client) == ERR_OK) return ERR_OK;
        tcp_abort(client);
        return ERR_ABRT;
    }
    if (error != ERR_OK || context == NULL) {
        pbuf_free(packet);
        return abort_http_client(context, client, error, "HTTP-Empfang fehlgeschlagen");
    }

    const size_t available = sizeof(context->data) - 1u - context->length;
    if (packet->tot_len > available) {
        tcp_recved(client, packet->tot_len);
        pbuf_free(packet);
        return send_response(context, client, "431 Request Header Fields Too Large",
                             "text/plain", "Request headers too large");
    }

    const u16_t copied = pbuf_copy_partial(packet, context->data + context->length,
                                           packet->tot_len, 0u);
    context->length += copied;
    context->data[context->length] = '\0';
    tcp_recved(client, packet->tot_len);
    pbuf_free(packet);
    if (!request_is_complete(context->data)) return ERR_OK;

    /* packet has been fully copied, acknowledged and freed. From this point
       only ERR_OK or ERR_ABRT (after tcp_abort) may be returned to lwIP. */
    const err_t result = process_http_request(context, client, context->data);
    return result == ERR_ABRT ? ERR_ABRT : ERR_OK;
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
    printf("[HTTP] Client verbunden\n");
    return ERR_OK;
}

bool webserver_init(const webserver_config_t *config) {
    if (config == NULL || config->status == NULL || config->start == NULL ||
        config->stop == NULL || config->set_setpoint == NULL) return false;
    server_config = *config;
    memset(http_clients, 0, sizeof(http_clients));
    web_auth_init(&control_auth);
    const int init_result = cyw43_arch_init();
    if (init_result != 0) {
        printf("[WLAN] Verbindungsfehler bei Initialisierung (Fehlercode %d)\n", init_result);
        return false;
    }
    wifi_initialized = true;
    first_connection_attempt = true;
    previous_link_status = CYW43_LINK_DOWN;

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
    printf("[HTTP] Dashboard: %u Bytes, TCP_SND_BUF: %u Bytes\n",
           (unsigned)(sizeof(dashboard_html) - 1u), (unsigned)TCP_SND_BUF);

    start_wifi_connection(milliseconds());
    return true;
}

void webserver_update(void) {
    if (!wifi_initialized) return;
    const uint32_t now = milliseconds();
    const int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

    if (link_status != previous_link_status) {
        if (link_status == CYW43_LINK_UP) {
            char ip[16];
            (void)webserver_get_ip(ip, sizeof(ip));
            printf("[WLAN] WLAN verbunden\n");
            printf("[WLAN] SSID: %s\n", WIFI_SSID);
            printf("[WLAN] IPv4-Adresse: %s\n", ip);
        } else {
            if (previous_link_status == CYW43_LINK_UP) {
                printf("[WLAN] WLAN getrennt (Statuscode %d)\n", link_status);
            }
            if (link_status < CYW43_LINK_DOWN) {
                printf("[WLAN] Verbindungsfehler: %s (Statuscode %d)\n",
                       wifi_status_name(link_status), link_status);
            }
        }
        previous_link_status = link_status;
    }

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
        if (link_status >= CYW43_LINK_DOWN) {
            printf("[WLAN] Verbindungsfehler: Zeitueberschreitung (Statuscode %d)\n",
                   link_status);
        }
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
        connect_in_progress = false;
        retry_due_ms = now + WIFI_RETRY_DELAY_MS;
        return;
    }
    if (deadline_reached(now, retry_due_ms)) start_wifi_connection(now);
}

bool webserver_is_connected(void) {
    return wifi_initialized &&
           cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

bool webserver_is_ready(void) {
    char ip[16];
    return listener != NULL && webserver_get_ip(ip, sizeof(ip));
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
