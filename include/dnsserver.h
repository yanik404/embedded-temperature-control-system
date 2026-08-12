#pragma once

#include <stdbool.h>

#include "lwip/ip_addr.h"

typedef struct {
    ip_addr_t ip;
    struct netif *netif;
    struct udp_pcb *udp;
} dns_server_t;

bool dns_server_init(dns_server_t *server, struct netif *netif, const ip_addr_t *ip);
void dns_server_deinit(dns_server_t *server);
