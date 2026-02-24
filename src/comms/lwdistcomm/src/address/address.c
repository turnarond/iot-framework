/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: address.c Address management implementation for LwDistComm.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../../include/address.h"

/* Create address structure */
lwdistcomm_address_t *lwdistcomm_address_create(lwdistcomm_addr_type_t type)
{
    lwdistcomm_address_t *addr = (lwdistcomm_address_t *)malloc(sizeof(lwdistcomm_address_t));
    if (!addr) {
        return NULL;
    }

    memset(addr, 0, sizeof(lwdistcomm_address_t));
    addr->type = type;

    switch (type) {
    case LWDISTCOMM_ADDR_TYPE_UNIX:
        addr->addr.unix_addr.sun_family = AF_UNIX;
        addr->addr_len = sizeof(struct sockaddr_un);
        break;
    case LWDISTCOMM_ADDR_TYPE_IPV4:
        addr->addr.ipv4_addr.sin_family = AF_INET;
        addr->addr_len = sizeof(struct sockaddr_in);
        break;
    case LWDISTCOMM_ADDR_TYPE_IPV6:
        addr->addr.ipv6_addr.sin6_family = AF_INET6;
        addr->addr_len = sizeof(struct sockaddr_in6);
        break;
    default:
        free(addr);
        return NULL;
    }

    return addr;
}

/* Parse address from string */
bool lwdistcomm_address_parse(lwdistcomm_address_t *addr, const char *addr_str)
{
    if (!addr || !addr_str) {
        return false;
    }

    switch (addr->type) {
    case LWDISTCOMM_ADDR_TYPE_UNIX:
        return lwdistcomm_address_set_unix_path(addr, addr_str);
    
    case LWDISTCOMM_ADDR_TYPE_IPV4: {
        char ip[INET_ADDRSTRLEN];
        uint16_t port;
        if (sscanf(addr_str, "%[^:]:%hu", ip, &port) == 2) {
            return lwdistcomm_address_set_ipv4(addr, ip, port);
        }
        return false;
    }
    
    case LWDISTCOMM_ADDR_TYPE_IPV6: {
        char ip[INET6_ADDRSTRLEN];
        uint16_t port;
        if (sscanf(addr_str, "[%[^]]]:%hu", ip, &port) == 2) {
            return lwdistcomm_address_set_ipv6(addr, ip, port);
        }
        return false;
    }
    
    default:
        return false;
    }
}

/* Set Unix domain socket path */
bool lwdistcomm_address_set_unix_path(lwdistcomm_address_t *addr, const char *path)
{
    if (!addr || !path || addr->type != LWDISTCOMM_ADDR_TYPE_UNIX) {
        return false;
    }

    if (strlen(path) >= sizeof(addr->addr.unix_addr.sun_path)) {
        return false;
    }

    strcpy(addr->addr.unix_addr.sun_path, path);
    return true;
}

/* Set IPv4 address and port */
bool lwdistcomm_address_set_ipv4(lwdistcomm_address_t *addr, const char *ip, uint16_t port)
{
    if (!addr || !ip || addr->type != LWDISTCOMM_ADDR_TYPE_IPV4) {
        return false;
    }

    if (inet_pton(AF_INET, ip, &addr->addr.ipv4_addr.sin_addr) != 1) {
        return false;
    }

    addr->addr.ipv4_addr.sin_port = htons(port);
    return true;
}

/* Set IPv6 address and port */
bool lwdistcomm_address_set_ipv6(lwdistcomm_address_t *addr, const char *ip, uint16_t port)
{
    if (!addr || !ip || addr->type != LWDISTCOMM_ADDR_TYPE_IPV6) {
        return false;
    }

    if (inet_pton(AF_INET6, ip, &addr->addr.ipv6_addr.sin6_addr) != 1) {
        return false;
    }

    addr->addr.ipv6_addr.sin6_port = htons(port);
    return true;
}

/* Get address type */
lwdistcomm_addr_type_t lwdistcomm_address_get_type(const lwdistcomm_address_t *addr)
{
    if (!addr) {
        return LWDISTCOMM_ADDR_TYPE_UNIX;
    }
    return addr->type;
}

/* Get socket address structure */
struct sockaddr *lwdistcomm_address_get_sockaddr(lwdistcomm_address_t *addr)
{
    if (!addr) {
        return NULL;
    }

    switch (addr->type) {
    case LWDISTCOMM_ADDR_TYPE_UNIX:
        return (struct sockaddr *)&addr->addr.unix_addr;
    case LWDISTCOMM_ADDR_TYPE_IPV4:
        return (struct sockaddr *)&addr->addr.ipv4_addr;
    case LWDISTCOMM_ADDR_TYPE_IPV6:
        return (struct sockaddr *)&addr->addr.ipv6_addr;
    default:
        return NULL;
    }
}

/* Get socket address length */
socklen_t lwdistcomm_address_get_len(const lwdistcomm_address_t *addr)
{
    if (!addr) {
        return 0;
    }
    return addr->addr_len;
}

/* Destroy address structure */
void lwdistcomm_address_destroy(lwdistcomm_address_t *addr)
{
    if (addr) {
        free(addr);
    }
}
