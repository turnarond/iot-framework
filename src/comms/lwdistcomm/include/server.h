/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: server.h Server interface for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_SERVER_H
#define LWDISTCOMM_SERVER_H

#include "types.h"
#include "address.h"
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create server instance */
lwdistcomm_server_t *lwdistcomm_server_create(const lwdistcomm_server_options_t *options);

/* Start server */
bool lwdistcomm_server_start(lwdistcomm_server_t *server, const lwdistcomm_address_t *addr);

/* Stop server */
bool lwdistcomm_server_stop(lwdistcomm_server_t *server);

/* Check if server is running */
bool lwdistcomm_server_is_running(const lwdistcomm_server_t *server);

/* Publish message to subscribers */
bool lwdistcomm_server_publish(lwdistcomm_server_t *server, const char *url, const lwdistcomm_message_t *msg);

/* Add RPC handler */
bool lwdistcomm_server_add_handler(lwdistcomm_server_t *server, const char *url, lwdistcomm_server_handler_cb_t callback, void *arg);

/* Remove RPC handler */
bool lwdistcomm_server_remove_handler(lwdistcomm_server_t *server, const char *url);

/* Set authentication callback */
void lwdistcomm_server_set_auth_callback(lwdistcomm_server_t *server, lwdistcomm_server_auth_cb_t callback, void *arg);

/* Set client connect/disconnect callback */
void lwdistcomm_server_set_client_callback(lwdistcomm_server_t *server, lwdistcomm_server_client_cb_t callback, void *arg);

/* Set datagram callback */
void lwdistcomm_server_set_datagram_callback(lwdistcomm_server_t *server, lwdistcomm_server_datagram_cb_t callback, void *arg);

/* Get number of connected clients */
int lwdistcomm_server_get_client_count(const lwdistcomm_server_t *server);

/* Process events */
bool lwdistcomm_server_process_events(lwdistcomm_server_t *server);

/* Get file descriptors for event polling */
int lwdistcomm_server_get_fds(lwdistcomm_server_t *server, fd_set *rfds);

/* Process input events */
bool lwdistcomm_server_process_input(lwdistcomm_server_t *server, const fd_set *rfds);

/* Destroy server instance */
void lwdistcomm_server_destroy(lwdistcomm_server_t *server);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_SERVER_H */
