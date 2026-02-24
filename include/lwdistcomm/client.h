/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: client.h Client interface for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_CLIENT_H
#define LWDISTCOMM_CLIENT_H

#include "types.h"
#include "address.h"
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create client instance */
lwdistcomm_client_t *lwdistcomm_client_create(const lwdistcomm_client_options_t *options);

/* Connect to server */
bool lwdistcomm_client_connect(lwdistcomm_client_t *client, const lwdistcomm_address_t *addr);

/* Disconnect from server */
bool lwdistcomm_client_disconnect(lwdistcomm_client_t *client);

/* Check if client is connected */
bool lwdistcomm_client_is_connected(const lwdistcomm_client_t *client);

/* Send RPC request */
bool lwdistcomm_client_rpc(lwdistcomm_client_t *client, const char *url, const lwdistcomm_message_t *msg, lwdistcomm_client_rpc_cb_t callback, void *arg);

/* Subscribe to topic */
bool lwdistcomm_client_subscribe(lwdistcomm_client_t *client, const char *url, lwdistcomm_client_message_cb_t callback, void *arg);

/* Unsubscribe from topic */
bool lwdistcomm_client_unsubscribe(lwdistcomm_client_t *client, const char *url);

/* Send datagram */
bool lwdistcomm_client_send_datagram(lwdistcomm_client_t *client, const char *url, const lwdistcomm_message_t *msg);

/* Set datagram callback */
void lwdistcomm_client_set_datagram_callback(lwdistcomm_client_t *client, lwdistcomm_client_datagram_cb_t callback, void *arg);

/* Process events */
bool lwdistcomm_client_process_events(lwdistcomm_client_t *client);

/* Get file descriptors for event polling */
int lwdistcomm_client_get_fds(lwdistcomm_client_t *client, fd_set *rfds);

/* Process input events */
bool lwdistcomm_client_process_input(lwdistcomm_client_t *client, const fd_set *rfds);

/* Start discovery */
bool lwdistcomm_client_start_discovery(lwdistcomm_client_t *client);

/* Stop discovery */
bool lwdistcomm_client_stop_discovery(lwdistcomm_client_t *client);

/* Auto discover and connect to server */
bool lwdistcomm_client_autodiscover(lwdistcomm_client_t *client);

/* Destroy client instance */
void lwdistcomm_client_destroy(lwdistcomm_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_CLIENT_H */
