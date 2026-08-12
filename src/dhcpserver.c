/*
 * DHCP server from Raspberry Pi pico-examples/pico_w/wifi/access_point.
 * Originates from MicroPython's netutils DHCP server.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2018-2019 Damien P. George
 */

#include "dhcpserver.h"

#include <errno.h>
#include <string.h>

#include "cyw43_config.h"
#include "lwip/udp.h"

#define DHCPDISCOVER 1
#define DHCPOFFER 2
#define DHCPREQUEST 3
#define DHCPACK 5

#define DHCP_OPT_SUBNET_MASK 1
#define DHCP_OPT_ROUTER 3
#define DHCP_OPT_DNS 6
#define DHCP_OPT_REQUESTED_IP 50
#define DHCP_OPT_IP_LEASE_TIME 51
#define DHCP_OPT_MSG_TYPE 53
#define DHCP_OPT_SERVER_ID 54
#define DHCP_OPT_END 255

#define PORT_DHCP_SERVER 67
#define PORT_DHCP_CLIENT 68
#define DEFAULT_LEASE_TIME_S (24 * 60 * 60)
#define MAC_LEN 6

typedef struct {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[312];
} dhcp_message_t;

static int socket_new(struct udp_pcb **udp, void *callback_data, udp_recv_fn callback) {
    *udp = udp_new();
    if (*udp == NULL) return -ENOMEM;
    udp_recv(*udp, callback, callback_data);
    return 0;
}

static void socket_free(struct udp_pcb **udp) {
    if (*udp != NULL) {
        udp_remove(*udp);
        *udp = NULL;
    }
}

static int socket_send(struct udp_pcb *udp, struct netif *netif, const void *buffer,
                       size_t length, uint32_t ip, uint16_t port) {
    if (length > UINT16_MAX) length = UINT16_MAX;
    struct pbuf *packet = pbuf_alloc(PBUF_TRANSPORT, (u16_t)length, PBUF_RAM);
    if (packet == NULL) return -ENOMEM;
    memcpy(packet->payload, buffer, length);

    ip_addr_t destination;
    IP4_ADDR(ip_2_ip4(&destination), ip >> 24 & 0xff, ip >> 16 & 0xff,
             ip >> 8 & 0xff, ip & 0xff);
    const err_t error = netif != NULL
                            ? udp_sendto_if(udp, packet, &destination, port, netif)
                            : udp_sendto(udp, packet, &destination, port);
    pbuf_free(packet);
    return error == ERR_OK ? (int)length : error;
}

static uint8_t *option_find(uint8_t *option, uint8_t command) {
    for (int i = 0; i < 308 && option[i] != DHCP_OPT_END;) {
        if (option[i] == command) return &option[i];
        i += 2 + option[i + 1];
    }
    return NULL;
}

static void option_write_bytes(uint8_t **option, uint8_t command,
                               size_t length, const void *data) {
    uint8_t *output = *option;
    *output++ = command;
    *output++ = (uint8_t)length;
    memcpy(output, data, length);
    *option = output + length;
}

static void option_write_u8(uint8_t **option, uint8_t command, uint8_t value) {
    uint8_t *output = *option;
    *output++ = command;
    *output++ = 1;
    *output++ = value;
    *option = output;
}

static void option_write_u32(uint8_t **option, uint8_t command, uint32_t value) {
    uint8_t *output = *option;
    *output++ = command;
    *output++ = 4;
    *output++ = (uint8_t)(value >> 24);
    *output++ = (uint8_t)(value >> 16);
    *output++ = (uint8_t)(value >> 8);
    *output++ = (uint8_t)value;
    *option = output;
}

static void process_request(void *argument, struct udp_pcb *udp, struct pbuf *packet,
                            const ip_addr_t *source, u16_t source_port) {
    dhcp_server_t *server = argument;
    (void)udp;
    (void)source;
    (void)source_port;
    dhcp_message_t message;

    const size_t minimum_size = 243;
    if (packet->tot_len < minimum_size ||
        pbuf_copy_partial(packet, &message, sizeof(message), 0) < minimum_size) {
        pbuf_free(packet);
        return;
    }

    message.op = DHCPOFFER;
    memcpy(message.yiaddr, &ip4_addr_get_u32(ip_2_ip4(&server->ip)), 4);
    uint8_t *option = message.options + 4;
    uint8_t *message_type = option_find(option, DHCP_OPT_MSG_TYPE);
    if (message_type == NULL) goto done;

    if (message_type[2] == DHCPDISCOVER) {
        int lease_index = DHCPS_MAX_IP;
        for (int i = 0; i < DHCPS_MAX_IP; ++i) {
            if (memcmp(server->lease[i].mac, message.chaddr, MAC_LEN) == 0) {
                lease_index = i;
                break;
            }
            if (lease_index == DHCPS_MAX_IP) {
                const uint8_t empty_mac[MAC_LEN] = {0};
                if (memcmp(server->lease[i].mac, empty_mac, MAC_LEN) == 0) lease_index = i;
                const uint32_t expiry = ((uint32_t)server->lease[i].expiry << 16) | 0xffffu;
                if ((int32_t)(expiry - cyw43_hal_ticks_ms()) < 0) {
                    memset(server->lease[i].mac, 0, MAC_LEN);
                    lease_index = i;
                }
            }
        }
        if (lease_index == DHCPS_MAX_IP) goto done;
        message.yiaddr[3] = DHCPS_BASE_IP + lease_index;
        option_write_u8(&option, DHCP_OPT_MSG_TYPE, DHCPOFFER);
    } else if (message_type[2] == DHCPREQUEST) {
        uint8_t *requested = option_find(option, DHCP_OPT_REQUESTED_IP);
        if (requested == NULL ||
            memcmp(requested + 2, &ip4_addr_get_u32(ip_2_ip4(&server->ip)), 3) != 0) goto done;
        const uint8_t lease_index = requested[5] - DHCPS_BASE_IP;
        if (lease_index >= DHCPS_MAX_IP) goto done;
        const uint8_t empty_mac[MAC_LEN] = {0};
        if (memcmp(server->lease[lease_index].mac, message.chaddr, MAC_LEN) != 0) {
            if (memcmp(server->lease[lease_index].mac, empty_mac, MAC_LEN) != 0) goto done;
            memcpy(server->lease[lease_index].mac, message.chaddr, MAC_LEN);
        }
        server->lease[lease_index].expiry =
            (uint16_t)((cyw43_hal_ticks_ms() + DEFAULT_LEASE_TIME_S * 1000u) >> 16);
        message.yiaddr[3] = DHCPS_BASE_IP + lease_index;
        option_write_u8(&option, DHCP_OPT_MSG_TYPE, DHCPACK);
    } else {
        goto done;
    }

    option_write_bytes(&option, DHCP_OPT_SERVER_ID, 4,
                       &ip4_addr_get_u32(ip_2_ip4(&server->ip)));
    option_write_bytes(&option, DHCP_OPT_SUBNET_MASK, 4,
                       &ip4_addr_get_u32(ip_2_ip4(&server->nm)));
    option_write_bytes(&option, DHCP_OPT_ROUTER, 4,
                       &ip4_addr_get_u32(ip_2_ip4(&server->ip)));
    option_write_bytes(&option, DHCP_OPT_DNS, 4,
                       &ip4_addr_get_u32(ip_2_ip4(&server->ip)));
    option_write_u32(&option, DHCP_OPT_IP_LEASE_TIME, DEFAULT_LEASE_TIME_S);
    *option++ = DHCP_OPT_END;
    socket_send(server->udp, ip_current_input_netif(), &message,
                (size_t)(option - (uint8_t *)&message), UINT32_MAX, PORT_DHCP_CLIENT);

done:
    pbuf_free(packet);
}

void dhcp_server_init(dhcp_server_t *server, struct netif *netif,
                      ip_addr_t *ip, ip_addr_t *netmask) {
    ip_addr_copy(server->ip, *ip);
    ip_addr_copy(server->nm, *netmask);
    memset(server->lease, 0, sizeof(server->lease));
    server->udp = NULL;
    if (socket_new(&server->udp, server, process_request) != 0) return;
    if (udp_bind(server->udp, IP_ANY_TYPE, PORT_DHCP_SERVER) != ERR_OK) {
        socket_free(&server->udp);
        return;
    }
    if (netif != NULL) udp_bind_netif(server->udp, netif);
}

void dhcp_server_deinit(dhcp_server_t *server) {
    socket_free(&server->udp);
}
