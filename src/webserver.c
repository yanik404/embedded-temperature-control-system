#include "webserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include "http_request.h"
#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "safety.h"
#include "web_assets.h"

static webserver_config_t server_config;
static struct tcp_pcb *listener;
static dhcp_server_t dhcp_server;
static dns_server_t dns_server;
static bool access_point_active;
static bool dhcp_active;
static bool dns_active;
static char response[3072];

#define HTTP_REQUEST_BUFFER_SIZE 1536u
#define HTTP_CLIENT_COUNT 4u

typedef struct {
    char data[HTTP_REQUEST_BUFFER_SIZE];
    size_t length;
    bool in_use;
} http_client_t;

static http_client_t http_clients[HTTP_CLIENT_COUNT];

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
    /* The dashboard lives permanently in flash, so lwIP can reference it without
       copying the complete page into scarce RAM. Dynamic API bodies are copied. */
    const u8_t body_flags = body == dashboard_html ? 0u : TCP_WRITE_FLAG_COPY;
    if (error == ERR_OK) error = tcp_write(client, body, (u16_t)body_length, body_flags);
    tcp_output(client);
    tcp_close(client);
    return error;
}

static err_t send_dashboard_redirect(struct tcp_pcb *client) {
    static const char body[] =
        "<!doctype html><html><head><meta charset=\"utf-8\">"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://" WIFI_AP_IP_ADDRESS "/\">"
        "</head><body><a href=\"http://" WIFI_AP_IP_ADDRESS "/\">Dashboard oeffnen</a>"
        "</body></html>";
    char header[256];
    const int header_length = snprintf(header, sizeof(header),
        "HTTP/1.1 302 Found\r\nLocation: http://%s/\r\nContent-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %u\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n",
        WIFI_AP_IP_ADDRESS, (unsigned)strlen(body));
    err_t result = tcp_write(client, header, (u16_t)header_length, TCP_WRITE_FLAG_COPY);
    if (result == ERR_OK) result = tcp_write(client, body, sizeof(body) - 1u, 0u);
    tcp_output(client);
    tcp_close(client);
    return result;
}

static err_t send_android_captive_page(struct tcp_pcb *client) {
    /* A non-empty 200 response is deliberately not an Internet-validation
       success. Android classifies it as captive, then its login WebView follows
       the meta/JavaScript navigation to the actual dashboard. */
    static const char body[] =
        "<!doctype html><html><head><meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://" WIFI_AP_IP_ADDRESS "/\">"
        "<title>Becherhalter</title></head><body>"
        "<h1>Becherhalter</h1><p>Lokales Dashboard wird geoeffnet.</p>"
        "<a href=\"http://" WIFI_AP_IP_ADDRESS "/\">Dashboard oeffnen</a>"
        "<script>location.replace('http://" WIFI_AP_IP_ADDRESS "/')</script>"
        "</body></html>";
    return send_response(client, "200 OK", "text/html; charset=utf-8", body);
}

static void status_json(void) {
    const system_status_t *s = server_config.status;
    snprintf(response, sizeof(response),
        "{\"state\":\"%s\",\"fault\":\"%s\",\"fault_description\":\"%s\","
        "\"temperature\":%.2f,\"temperature1\":%.2f,\"temperature2\":%.2f,"
        "\"setpoint\":%.2f,\"error\":%.2f,\"power\":%.1f,\"fan_rpm\":%u,\"fan_percent\":%u,"
        "\"current1\":%.3f,\"current2\":%.3f,\"light_level\":%.3f,"
        "\"sensor_ok\":%s,\"temp1_ok\":%s,\"temp2_ok\":%s,"
        "\"current_ok\":%s,\"current1_ok\":%s,\"current2_ok\":%s,"
        "\"tla2024_ok\":%s,\"light_ok\":%s,\"display_initialized\":%s,\"leds_initialized\":%s,"
        "\"cup\":%s,\"power_good\":%s,\"wifi\":%s,\"night\":%s,"
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
        s->wifi_connected ? "true" : "false", s->night_mode ? "true" : "false",
        start_allowed() ? "true" : "false", start_block_reason());
}

static err_t process_http_request(struct tcp_pcb *client, const char *text) {
    http_request_t request;
    if (!http_request_parse(text, &request)) {
        return send_response(client, "400 Bad Request", "text/plain", "Bad request");
    }

    if (request.method == HTTP_METHOD_GET && http_request_path_equals(&request, "/api/status")) {
        status_json();
        return send_response(client, "200 OK", "application/json", response);
    }
    if (request.method == HTTP_METHOD_POST && http_request_path_equals(&request, "/api/start")) {
        if (start_allowed()) {
            server_config.start();
            return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
        }
        snprintf(response, sizeof(response), "{\"ok\":false,\"reason\":\"%s\"}",
                 start_block_reason());
        return send_response(client, "409 Conflict", "application/json", response);
    }
    if (request.method == HTTP_METHOD_POST && http_request_path_equals(&request, "/api/stop")) {
        server_config.stop();
        return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
    }
    static const char setpoint_prefix[] = "/api/setpoint?value=";
    if (request.method == HTTP_METHOD_POST &&
        strncmp(request.path, setpoint_prefix, sizeof(setpoint_prefix) - 1u) == 0) {
        float value = strtof(request.path + sizeof(setpoint_prefix) - 1u, NULL);
        if (value >= SETPOINT_MIN_C && value <= SETPOINT_MAX_C) {
            server_config.set_setpoint(value);
            return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
        }
        return send_response(client, "400 Bad Request", "application/json", "{\"ok\":false}");
    }

    /* Android treats a non-empty 200 as captive (while an empty 200 may be
       normalized to 204). The small page is more compatible with OEM network
       monitors than relying solely on their redirect handling. */
    if (http_request_is_android_probe(&request)) return send_android_captive_page(client);

    const bool dashboard_host = request.host[0] == '\0' ||
                                http_request_host_equals(&request, WIFI_AP_IP_ADDRESS);
    if (request.method == HTTP_METHOD_GET && http_request_path_equals(&request, "/") &&
        dashboard_host) {
        return send_response(client, "200 OK", "text/html; charset=utf-8", dashboard_html);
    }
    /* Android (/generate_204), Apple (/hotspot-detect.html), Windows
       (/connecttest.txt, /ncsi.txt) and other HTTP probes all reach this
       fallback through wildcard DNS. A redirect deliberately indicates a
       captive network and opens the local dashboard where the OS permits it. */
    if (request.method == HTTP_METHOD_GET || request.method == HTTP_METHOD_HEAD) {
        return send_dashboard_redirect(client);
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

    /* Starting the radio never grants heating permission; that remains solely
       controlled by the application state machine and safety module. */
    cyw43_arch_enable_ap_mode(WIFI_AP_SSID, WIFI_AP_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    access_point_active = true;

#if LWIP_IPV6
#define IP4_FIELD(address) ((address).u_addr.ip4)
#else
#define IP4_FIELD(address) (address)
#endif
    ip4_addr_t gateway;
    ip4_addr_t netmask;
    IP4_FIELD(gateway).addr = PP_HTONL(CYW43_DEFAULT_IP_AP_ADDRESS);
    IP4_FIELD(netmask).addr = PP_HTONL(CYW43_DEFAULT_IP_MASK);
#undef IP4_FIELD

    dhcp_server_init(&dhcp_server, &cyw43_state.netif[CYW43_ITF_AP],
                     &gateway, &netmask);
    dhcp_active = dhcp_server.udp != NULL;
    if (!dhcp_active) {
        webserver_deinit();
        return false;
    }

    dns_active = dns_server_init(&dns_server, &cyw43_state.netif[CYW43_ITF_AP], &gateway);
    if (!dns_active) {
        webserver_deinit();
        return false;
    }

    cyw43_arch_lwip_begin();
    listener = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (listener != NULL && tcp_bind(listener, IP_ANY_TYPE, 80u) == ERR_OK) {
        listener = tcp_listen_with_backlog(listener, 4u);
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
    return true;
}

void webserver_deinit(void) {
    if (listener != NULL) {
        cyw43_arch_lwip_begin();
        tcp_close(listener);
        cyw43_arch_lwip_end();
    }
    listener = NULL;
    if (dns_active) dns_server_deinit(&dns_server);
    dns_active = false;
    if (dhcp_active) dhcp_server_deinit(&dhcp_server);
    dhcp_active = false;
    if (access_point_active) cyw43_arch_disable_ap_mode();
    access_point_active = false;
    cyw43_arch_deinit();
}

bool webserver_is_connected(void) {
    return access_point_active && dhcp_active && dns_active && listener != NULL;
}
