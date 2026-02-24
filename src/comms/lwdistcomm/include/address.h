/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: address.h Address management for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_ADDRESS_H
#define LWDISTCOMM_ADDRESS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create address structure */
lwdistcomm_address_t *lwdistcomm_address_create(lwdistcomm_addr_type_t type);

/* Parse address from string */
bool lwdistcomm_address_parse(lwdistcomm_address_t *addr, const char *addr_str);

/* Set Unix domain socket path */
bool lwdistcomm_address_set_unix_path(lwdistcomm_address_t *addr, const char *path);

/* Set IPv4 address and port */
bool lwdistcomm_address_set_ipv4(lwdistcomm_address_t *addr, const char *ip, uint16_t port);

/* Set IPv6 address and port */
bool lwdistcomm_address_set_ipv6(lwdistcomm_address_t *addr, const char *ip, uint16_t port);

/* Get address type */
lwdistcomm_addr_type_t lwdistcomm_address_get_type(const lwdistcomm_address_t *addr);

/* Get socket address structure */
struct sockaddr *lwdistcomm_address_get_sockaddr(lwdistcomm_address_t *addr);

/* Get socket address length */
socklen_t lwdistcomm_address_get_len(const lwdistcomm_address_t *addr);

/* Destroy address structure */
void lwdistcomm_address_destroy(lwdistcomm_address_t *addr);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_ADDRESS_H */
