/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: client_impl.h Client internal implementation for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_CLIENT_IMPL_H
#define LWDISTCOMM_CLIENT_IMPL_H

#include "../../include/client.h"
#include "../../include/security.h"
#include "../../include/message.h"

/* Include necessary system headers */
#include <unistd.h>
#include <sys/eventfd.h>

/* Message constants */
#define LWDISTCOMM_MSG_MAX_LEN  131072
#define LWDISTCOMM_MSG_HDR_LEN  sizeof(lwdistcomm_msg_header_t)
#define LWDISTCOMM_MSG_MAX_DATA (LWDISTCOMM_MSG_MAX_LEN - LWDISTCOMM_MSG_HDR_LEN)

/* Client pending queue */
typedef struct lwdistcomm_client_pendq {
    struct lwdistcomm_client_pendq *next;
    struct lwdistcomm_client_pendq *prev;
    int16_t alive;
    uint16_t seqno;
    uint32_t ftype;
    union {
        lwdistcomm_client_message_cb_t msg;
        lwdistcomm_client_rpc_cb_t rpc;
        lwdistcomm_client_subscribe_cb_t sub;
        lwdistcomm_client_datagram_cb_t dat;
    } callback;
    void *arg;
} lwdistcomm_client_pendq_t;

/* Client structure */
struct lwdistcomm_client {
    bool valid;
    bool connected;
    lwdistcomm_client_t *next;
    lwdistcomm_client_t *prev;
    lwdistcomm_client_pendq_t *head;
    lwdistcomm_client_pendq_t *tail;
    lwdistcomm_client_pendq_t *free;
    lwdistcomm_client_pendq_t *pool;
    void *sendbuf;
    void *recvbuf;
    lwdistcomm_msg_recv_t recv;
    bool cid_valid;
    uint32_t rpc_pending;
    uint32_t cid;
    uint16_t seqno;
    uint16_t seqno_nq;
    int sock;
    int evtfd[2];
    struct timeval send_timeout;
    void *lock;
    lwdistcomm_client_message_cb_t onmsg;
    void *marg;
    lwdistcomm_client_datagram_cb_t ondat;
    void *darg;
    lwdistcomm_security_t *security;
    bool enable_discovery;
    int discovery_port;
    int discovery_socket;
    pthread_t discovery_thread;
    bool discovery_thread_running;
    char client_name[64];
    struct {
        char server_name[64];
        char ip[16];
        uint16_t port;
    } discovered_server;
};

/* Client callback types */
#define LWDISTCOMM_CLIENT_FTYPE_MSG  0
#define LWDISTCOMM_CLIENT_FTYPE_RPC  1
#define LWDISTCOMM_CLIENT_FTYPE_SUB  2
#define LWDISTCOMM_CLIENT_FTYPE_DAT  3

/* Client fast pending buffer */
#define LWDISTCOMM_CLIENT_FAST_PENDING_POOL  8
#define LWDISTCOMM_CLIENT_MAX_PENDING  0xff
#define LWDISTCOMM_CLIENT_MAX_POFFSET  8

/* Client timer period */
#define LWDISTCOMM_CLIENT_TIMER_PERIOD  10
#define LWDISTCOMM_CLIENT_DEF_TIMEOUT  60000
#define LWDISTCOMM_CLIENT_DEF_SEND_TIMEOUT  500

/* Internal functions */
static void lwdistcomm_client_pendq_free(lwdistcomm_client_t *client, lwdistcomm_client_pendq_t *pendq);
static lwdistcomm_client_pendq_t *lwdistcomm_client_prepare_pendq(lwdistcomm_client_t *client, bool fast, void *arg, uint32_t ftype, int timeout);
static bool lwdistcomm_client_send(lwdistcomm_client_t *client, size_t len);
static bool lwdistcomm_client_sendmsg(lwdistcomm_client_t *client, lwdistcomm_msg_header_t *header, const char *url, const lwdistcomm_message_t *msg);
static void lwdistcomm_client_timeout_all(lwdistcomm_client_t *client);
static bool lwdistcomm_client_input(void *arg, lwdistcomm_msg_header_t *header);
static uint16_t lwdistcomm_client_prepare_seqno(lwdistcomm_client_t *client);
static bool lwdistcomm_client_request(lwdistcomm_client_t *client, uint8_t type, const char *url, const lwdistcomm_message_t *msg, lwdistcomm_client_subscribe_cb_t callback, void *arg, int timeout);

#endif /* LWDISTCOMM_CLIENT_IMPL_H */
