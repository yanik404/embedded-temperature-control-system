#include "dnsserver.h"

#include <string.h>

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define DNS_PORT 53u
#define DNS_HEADER_SIZE 12u
#define DNS_MAX_PACKET_SIZE 512u
#define DNS_TYPE_A 1u
#define DNS_CLASS_IN 1u
#define DNS_TTL_SECONDS 60u

static uint16_t read_u16(const uint8_t *data) {
    return (uint16_t)((uint16_t)data[0] << 8u) | data[1];
}

static void write_u16(uint8_t *data, uint16_t value) {
    data[0] = (uint8_t)(value >> 8u);
    data[1] = (uint8_t)value;
}

static void write_u32(uint8_t *data, uint32_t value) {
    data[0] = (uint8_t)(value >> 24u);
    data[1] = (uint8_t)(value >> 16u);
    data[2] = (uint8_t)(value >> 8u);
    data[3] = (uint8_t)value;
}

static size_t question_end(const uint8_t *query, size_t query_length) {
    size_t offset = DNS_HEADER_SIZE;
    while (offset < query_length) {
        const uint8_t label_length = query[offset++];
        if (label_length == 0u) break;
        if ((label_length & 0xc0u) != 0u || label_length > 63u ||
            offset + label_length > query_length) return 0u;
        offset += label_length;
    }
    return offset + 4u <= query_length ? offset + 4u : 0u;
}

static size_t build_response(const dns_server_t *server, const uint8_t *query,
                             size_t query_length, uint8_t *reply, size_t reply_capacity) {
    if (query_length < DNS_HEADER_SIZE || reply_capacity < DNS_HEADER_SIZE ||
        (query[2] & 0x80u) != 0u || read_u16(query + 4u) == 0u) return 0u;

    const size_t end = question_end(query, query_length);
    if (end == 0u || end > reply_capacity) return 0u;
    memcpy(reply, query, end);

    const uint16_t query_flags = read_u16(query + 2u);
    write_u16(reply + 2u, (uint16_t)(0x8400u | (query_flags & 0x0100u)));
    write_u16(reply + 4u, 1u);
    write_u16(reply + 6u, 0u);
    write_u16(reply + 8u, 0u);
    write_u16(reply + 10u, 0u);

    const uint16_t query_type = read_u16(query + end - 4u);
    const uint16_t query_class = read_u16(query + end - 2u);
    if (query_type != DNS_TYPE_A || query_class != DNS_CLASS_IN) return end;
    if (end + 16u > reply_capacity) return 0u;

    write_u16(reply + 6u, 1u);
    reply[end] = 0xc0u;       /* Compressed pointer to the question name. */
    reply[end + 1u] = 0x0cu;
    write_u16(reply + end + 2u, DNS_TYPE_A);
    write_u16(reply + end + 4u, DNS_CLASS_IN);
    write_u32(reply + end + 6u, DNS_TTL_SECONDS);
    write_u16(reply + end + 10u, 4u);
    memcpy(reply + end + 12u, &ip4_addr_get_u32(ip_2_ip4(&server->ip)), 4u);
    return end + 16u;
}

static void process_request(void *argument, struct udp_pcb *udp, struct pbuf *packet,
                            const ip_addr_t *source, u16_t source_port) {
    dns_server_t *server = argument;
    uint8_t query[DNS_MAX_PACKET_SIZE];
    uint8_t reply[DNS_MAX_PACKET_SIZE];
    if (packet->tot_len > sizeof(query)) {
        pbuf_free(packet);
        return;
    }

    const u16_t query_length = pbuf_copy_partial(packet, query, sizeof(query), 0u);
    pbuf_free(packet);
    const size_t reply_length = build_response(server, query, query_length,
                                               reply, sizeof(reply));
    if (reply_length == 0u) return;

    struct pbuf *response = pbuf_alloc(PBUF_TRANSPORT, (u16_t)reply_length, PBUF_RAM);
    if (response == NULL) return;
    memcpy(response->payload, reply, reply_length);
    (void)udp_sendto_if(udp, response, source, source_port, server->netif);
    pbuf_free(response);
}

bool dns_server_init(dns_server_t *server, struct netif *netif, const ip_addr_t *ip) {
    if (server == NULL || netif == NULL || ip == NULL) return false;
    memset(server, 0, sizeof(*server));
    ip_addr_copy(server->ip, *ip);
    server->netif = netif;
    server->udp = udp_new_ip_type(IPADDR_TYPE_V4);
    if (server->udp == NULL) return false;
    if (udp_bind(server->udp, IP_ANY_TYPE, DNS_PORT) != ERR_OK) {
        dns_server_deinit(server);
        return false;
    }
    udp_bind_netif(server->udp, netif);
    udp_recv(server->udp, process_request, server);
    return true;
}

void dns_server_deinit(dns_server_t *server) {
    if (server != NULL && server->udp != NULL) {
        udp_remove(server->udp);
        server->udp = NULL;
    }
}
