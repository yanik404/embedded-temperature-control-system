/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 * Copyright (c) 2018-2019 Damien P. George
 */
#pragma once

#include "lwip/ip_addr.h"

#define DHCPS_BASE_IP 16
#define DHCPS_MAX_IP 8

typedef struct {
    uint8_t mac[6];
    uint16_t expiry;
} dhcp_server_lease_t;

typedef struct {
    ip_addr_t ip;
    ip_addr_t nm;
    dhcp_server_lease_t lease[DHCPS_MAX_IP];
    struct udp_pcb *udp;
} dhcp_server_t;

void dhcp_server_init(dhcp_server_t *server, struct netif *netif,
                      ip_addr_t *ip, ip_addr_t *netmask);
void dhcp_server_deinit(dhcp_server_t *server);
