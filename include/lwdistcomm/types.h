/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: types.h Common types definition for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_TYPES_H
#define LWDISTCOMM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Address types */
typedef enum {
    LWDISTCOMM_ADDR_TYPE_UNIX,    // Unix domain socket
    LWDISTCOMM_ADDR_TYPE_IPV4,     // IPv4 address
    LWDISTCOMM_ADDR_TYPE_IPV6      // IPv6 address
} lwdistcomm_addr_type_t;

/* Address structure */
typedef struct {
    lwdistcomm_addr_type_t type;
    union {
        struct sockaddr_un unix_addr;
        struct sockaddr_in ipv4_addr;
        struct sockaddr_in6 ipv6_addr;
    } addr;
    socklen_t addr_len;
} lwdistcomm_address_t;

/* Message structure */
typedef struct {
    void *data;
    size_t data_len;
} lwdistcomm_message_t;

/* Security options */
typedef struct {
    bool enable_tls;
    const char *ca_cert;
    const char *client_cert;
    const char *client_key;
    const char *server_cert;
    const char *server_key;
    bool verify_peer;
} lwdistcomm_security_options_t;

/* Authentication information */
typedef struct {
    const char *username;
    const char *password;
    const char *token;
} lwdistcomm_auth_info_t;

/* Client options */
typedef struct {
    lwdistcomm_security_options_t *security_options;
    lwdistcomm_auth_info_t *auth_info;
} lwdistcomm_client_options_t;

/* Server options */
typedef struct {
    lwdistcomm_security_options_t *security_options;
} lwdistcomm_server_options_t;

/* Client callback types */
typedef void (*lwdistcomm_client_message_cb_t)(void *arg, const char *url, const lwdistcomm_message_t *msg);
typedef void (*lwdistcomm_client_rpc_cb_t)(void *arg, int status, const lwdistcomm_message_t *msg);
typedef void (*lwdistcomm_client_subscribe_cb_t)(void *arg, bool success);
typedef void (*lwdistcomm_client_datagram_cb_t)(void *arg, const char *url, const lwdistcomm_message_t *msg);

/* Server callback types */
typedef bool (*lwdistcomm_server_auth_cb_t)(void *arg, const char *username, const char *password);
typedef void (*lwdistcomm_server_handler_cb_t)(void *arg, uint32_t client_id, const char *url, const lwdistcomm_message_t *msg, lwdistcomm_message_t *response);
typedef void (*lwdistcomm_server_client_cb_t)(void *arg, uint32_t client_id, bool connected);
typedef void (*lwdistcomm_server_datagram_cb_t)(void *arg, uint32_t client_id, const char *url, const lwdistcomm_message_t *msg);

/* Forward declarations */
typedef struct lwdistcomm_client lwdistcomm_client_t;
typedef struct lwdistcomm_server lwdistcomm_server_t;
typedef struct lwdistcomm_security lwdistcomm_security_t;

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_TYPES_H */
