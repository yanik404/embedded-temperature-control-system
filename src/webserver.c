#include "webserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dhcpserver.h"
#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "web_assets.h"

static webserver_config_t server_config;
static struct tcp_pcb *listener;
static dhcp_server_t dhcp_server;
static bool access_point_active;
static bool dhcp_active;
static char response[1024];

static err_t send_response(struct tcp_pcb *client, const char *status, const char *type,
                           const char *body) {
    const size_t body_length = strlen(body);
    char header[192];
    const int header_length = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %u\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n",
        status, type, (unsigned)body_length);
    err_t error = tcp_write(client, header, (u16_t)header_length, TCP_WRITE_FLAG_COPY);
    if (error == ERR_OK) error = tcp_write(client, body, (u16_t)body_length, TCP_WRITE_FLAG_COPY);
    tcp_output(client);
    tcp_close(client);
    return error;
}

static void status_json(void) {
    const system_status_t *s = server_config.status;
    snprintf(response, sizeof(response),
        "{\"state\":\"%s\",\"fault\":\"%s\",\"temperature\":%.2f,\"temperature2\":%.2f,"
        "\"setpoint\":%.2f,\"error\":%.2f,\"power\":%.1f,\"fan_rpm\":%u,"
        "\"current1\":%.3f,\"current2\":%.3f,\"sensor_ok\":%s,\"current_ok\":%s,"
        "\"cup\":%s,\"wifi\":%s,\"night\":%s}",
        system_state_name(s->state), error_name(s->error), s->temperature_c, s->temperature_2_c,
        s->setpoint_c, s->control_error_c, s->peltier_power_percent, s->fan_rpm,
        s->peltier_1_current_a, s->peltier_2_current_a,
        s->temperature_valid ? "true" : "false", s->current_valid ? "true" : "false",
        s->cup_detected ? "true" : "false", s->wifi_connected ? "true" : "false",
        s->night_mode ? "true" : "false");
}

static err_t receive(void *arg, struct tcp_pcb *client, struct pbuf *packet, err_t error) {
    (void)arg;
    if (packet == NULL) { tcp_close(client); return ERR_OK; }
    if (error != ERR_OK) { pbuf_free(packet); tcp_abort(client); return error; }
    char request[512];
    const u16_t copied = pbuf_copy_partial(packet, request, sizeof(request) - 1u, 0u);
    request[copied] = '\0';
    tcp_recved(client, packet->tot_len);
    pbuf_free(packet);

    if (strncmp(request, "GET /api/status ", 16) == 0) {
        status_json();
        return send_response(client, "200 OK", "application/json", response);
    }
    if (strncmp(request, "POST /api/start ", 16) == 0) {
        server_config.start();
        return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
    }
    if (strncmp(request, "POST /api/stop ", 15) == 0) {
        server_config.stop();
        return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
    }
    if (strncmp(request, "POST /api/setpoint?value=", 25) == 0) {
        float value = strtof(request + 25, NULL);
        if (value >= SETPOINT_MIN_C && value <= SETPOINT_MAX_C) server_config.set_setpoint(value);
        return send_response(client, "200 OK", "application/json", "{\"ok\":true}");
    }
    if (strncmp(request, "GET / ", 6) == 0) {
        return send_response(client, "200 OK", "text/html; charset=utf-8", dashboard_html);
    }
    return send_response(client, "404 Not Found", "text/plain", "Not found");
}

static err_t accept_client(void *arg, struct tcp_pcb *client, err_t error) {
    (void)arg;
    if (error != ERR_OK || client == NULL) return ERR_VAL;
    tcp_recv(client, receive);
    return ERR_OK;
}

bool webserver_init(const webserver_config_t *config) {
    if (config == NULL || config->status == NULL || config->start == NULL ||
        config->stop == NULL || config->set_setpoint == NULL) return false;
    server_config = *config;
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
    if (dhcp_active) dhcp_server_deinit(&dhcp_server);
    dhcp_active = false;
    if (access_point_active) cyw43_arch_disable_ap_mode();
    access_point_active = false;
    cyw43_arch_deinit();
}

bool webserver_is_connected(void) {
    return access_point_active && dhcp_active && listener != NULL;
}
