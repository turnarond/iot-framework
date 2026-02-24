/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: server_impl.h Server internal implementation for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_SERVER_IMPL_H
#define LWDISTCOMM_SERVER_IMPL_H

#include "../../include/server.h"
#include "../../include/security.h"
#include "../../include/message.h"

/* Message constants */
#define LWDISTCOMM_MSG_MAX_LEN  131072
#define LWDISTCOMM_MSG_HDR_LEN  sizeof(lwdistcomm_msg_header_t)
#define LWDISTCOMM_MSG_MAX_DATA (LWDISTCOMM_MSG_MAX_LEN - LWDISTCOMM_MSG_HDR_LEN)

/* Client hash */
#define LWDISTCOMM_SERVER_CLI_HASH_SIZE  64
#define LWDISTCOMM_SERVER_CLI_HASH_MASK  0x2f

/* Command hash */
#define LWDISTCOMM_SERVER_CMD_HASH_SIZE  32
#define LWDISTCOMM_SERVER_CMD_HASH_MASK  0x1f

/* Subscription node */
typedef struct lwdistcomm_server_sub {
    struct lwdistcomm_server_sub *next;
    struct lwdistcomm_server_sub *prev;
    size_t len;
    char url[1];
} lwdistcomm_server_sub_t;

/* Client handshake timer */
typedef struct lwdistcomm_server_hst {
    struct lwdistcomm_server_hst *next;
    struct lwdistcomm_server_hst *prev;
    int alive;
} lwdistcomm_server_hst_t;

/* Client node */
typedef struct lwdistcomm_server_cli {
    bool active;
    bool onconn;
    struct lwdistcomm_server_cli *next;
    struct lwdistcomm_server_cli *prev;
    lwdistcomm_server_sub_t *subscribed;
    lwdistcomm_server_hst_t hst;
    lwdistcomm_msg_recv_t recv;
    int sock;
    uint32_t id;
} lwdistcomm_server_cli_t;

/* Server command */
typedef struct lwdistcomm_server_cmd {
    struct lwdistcomm_server_cmd *next;
    struct lwdistcomm_server_cmd *prev;
    lwdistcomm_server_handler_cb_t callback;
    void *arg;
    size_t len;
    char url[1];
} lwdistcomm_server_cmd_t;

/* Server structure */
struct lwdistcomm_server {
    bool valid;
    char ifname[IF_NAMESIZE];
    uint32_t ncid;
    lwdistcomm_server_t *next;
    lwdistcomm_server_t *prev;
    lwdistcomm_server_hst_t *hst_h;
    lwdistcomm_server_cli_t *clis[LWDISTCOMM_SERVER_CLI_HASH_SIZE];
    lwdistcomm_server_cmd_t *cmds[LWDISTCOMM_SERVER_CMD_HASH_SIZE];
    lwdistcomm_server_cmd_t *def_cmd;
    lwdistcomm_server_cmd_t *prefix_h;
    lwdistcomm_server_cmd_t *prefix_t;
    lwdistcomm_server_datagram_cb_t ondat;
    void *darg;
    lwdistcomm_server_client_cb_t oncli;
    void *carg;
    lwdistcomm_server_auth_cb_t onauth;
    void *aarg;
    void *lock;
    struct timeval send_timeout;
    int sock;
    int evtfd[2];
    void *sendbuf;
    void *recvbuf;
    lwdistcomm_security_t *security;
    bool enable_discovery;
    int discovery_port;
    int discovery_socket;
    pthread_t discovery_thread;
    bool discovery_thread_running;
    char server_name[64];
};

/* Server timer period */
#define LWDISTCOMM_SERVER_TIMER_PERIOD  100
#define LWDISTCOMM_SERVER_DEF_SEND_TIMEOUT  100
#define LWDISTCOMM_SERVER_DEF_HANDSHAKE_TIMEOUT  5000
#define LWDISTCOMM_SERVER_KEEPALIVE_TIMEOUT  10
#define LWDISTCOMM_SERVER_BACKLOG  32

/* Internal functions */
static uint32_t lwdistcomm_server_cli_hash(uint32_t id);
static lwdistcomm_server_cli_t *lwdistcomm_server_cli_find(lwdistcomm_server_t *server, uint32_t id);
static uint32_t lwdistcomm_server_cli_newid(lwdistcomm_server_t *server);
static void lwdistcomm_server_cli_init(lwdistcomm_server_t *server, lwdistcomm_server_cli_t *cli);
static void lwdistcomm_server_cli_destroy(lwdistcomm_server_t *server, lwdistcomm_server_cli_t *cli);
static bool lwdistcomm_server_cli_sub_match(lwdistcomm_server_cli_t *cli, const char *url);
static bool lwdistcomm_server_cli_sendmsg(lwdistcomm_server_cli_t *cli, lwdistcomm_msg_header_t *header, const char *url, const lwdistcomm_message_t *msg);
static bool lwdistcomm_server_cmd_match(lwdistcomm_server_t *server, const char *url, lwdistcomm_server_handler_cb_t *callback, void **arg);
static bool lwdistcomm_server_input(void *arg, lwdistcomm_msg_header_t *header);

#endif /* LWDISTCOMM_SERVER_IMPL_H */
