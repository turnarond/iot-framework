/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: server.c Server implementation for LwDistComm.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <pthread.h>
#include "server_impl.h"

// Forward declarations
bool lwdistcomm_server_start_discovery(lwdistcomm_server_t *server);
bool lwdistcomm_server_stop_discovery(lwdistcomm_server_t *server);

/* Include transport functions declarations */
extern int lwdistcomm_transport_create_socket(lwdistcomm_addr_type_t type, bool non_blocking, int sock_type);
extern int lwdistcomm_transport_create_tcp_socket(lwdistcomm_addr_type_t type, bool non_blocking);
extern int lwdistcomm_transport_create_udp_socket(lwdistcomm_addr_type_t type, bool non_blocking);
extern bool lwdistcomm_transport_connect(int sock, const lwdistcomm_address_t *addr);
extern bool lwdistcomm_transport_bind(int sock, const lwdistcomm_address_t *addr);
extern bool lwdistcomm_transport_listen(int sock, int backlog);
extern int lwdistcomm_transport_accept(int sock, struct sockaddr *addr, socklen_t *addr_len);
extern ssize_t lwdistcomm_transport_send(int sock, const void *data, size_t len);
extern ssize_t lwdistcomm_transport_recv(int sock, void *buffer, size_t len, int flags);
extern ssize_t lwdistcomm_transport_sendto(int sock, const void *data, size_t len, const lwdistcomm_address_t *addr);
extern ssize_t lwdistcomm_transport_recvfrom(int sock, void *buffer, size_t len, lwdistcomm_address_t *addr);
extern void lwdistcomm_transport_close(int sock);
extern bool lwdistcomm_transport_set_timeout(int sock, int timeout_ms);

/* Default send timeout */
static const struct timeval lwdistcomm_server_def_send_timeout = {
    .tv_sec  = (LWDISTCOMM_SERVER_DEF_SEND_TIMEOUT / 1000),
    .tv_usec = (LWDISTCOMM_SERVER_DEF_SEND_TIMEOUT % 1000) * 1000
};

/* Server list lock, header, timer thread */
static void *lwdistcomm_server_timer = NULL;
static lwdistcomm_server_t *lwdistcomm_server_list = NULL;
static void *lwdistcomm_server_lock = NULL;

/* List operations */
#define LIST_FOREACH(item, head) for (item = head; item; item = item->next)
#define LIST_FOREACH_SAFE(item, temp, head) for (item = head, temp = item ? item->next : NULL; item; item = temp, temp = item ? item->next : NULL)
#define INSERT_TO_HEADER(item_ptr, head) do { \
    (item_ptr)->next = head; \
    if (head) head->prev = (item_ptr); \
    head = (item_ptr); \
    (item_ptr)->prev = NULL; \
} while (0)
#define INSERT_TO_FIFO(item_ptr, head, tail) do { \
    if (!head) { \
        head = tail = (item_ptr); \
        (item_ptr)->prev = (item_ptr)->next = NULL; \
    } else { \
        tail->next = (item_ptr); \
        (item_ptr)->prev = tail; \
        (item_ptr)->next = NULL; \
        tail = (item_ptr); \
    } \
} while (0)
#define DELETE_FROM_LIST(item_ptr, head) do { \
    if ((item_ptr)->prev) (item_ptr)->prev->next = (item_ptr)->next; \
    else head = (item_ptr)->next; \
    if ((item_ptr)->next) (item_ptr)->next->prev = (item_ptr)->prev; \
} while (0)
#define DELETE_FROM_FIFO(item_ptr, head, tail) do { \
    if ((item_ptr)->prev) (item_ptr)->prev->next = (item_ptr)->next; \
    else head = (item_ptr)->next; \
    if ((item_ptr)->next) (item_ptr)->next->prev = (item_ptr)->prev; \
    else tail = (item_ptr)->prev; \
} while (0)

/* Server timer thread handle */
static void *lwdistcomm_server_timer_handle(void *arg)
{
    bool emit;
    lwdistcomm_server_t *server;
    lwdistcomm_server_hst_t *hst;

    (void)arg;

    do {
        // Sleep for timer period
        usleep(LWDISTCOMM_SERVER_TIMER_PERIOD * 1000);

        // Lock server list
        // TODO: Implement proper locking

        if (!lwdistcomm_server_list) {
            // No servers, exit loop
            break;
        }

        LIST_FOREACH(server, lwdistcomm_server_list) {
            if (!server->hst_h) {
                continue;
            }

            emit = false;

            // Lock server
            // TODO: Implement proper locking

            LIST_FOREACH(hst, server->hst_h) {
                if (hst->alive > LWDISTCOMM_SERVER_TIMER_PERIOD) {
                    hst->alive -= LWDISTCOMM_SERVER_TIMER_PERIOD;
                } else {
                    emit = true;
                }
            }

            // Unlock server
            // TODO: Implement proper locking

            if (emit) {
                // Signal event
                uint64_t val = 1;
                write(server->evtfd[1], &val, sizeof(val));
            }
        }

        // Unlock server list
        // TODO: Implement proper locking

    } while (true);

    return NULL;
}

/* Client hash */
static uint32_t lwdistcomm_server_cli_hash(uint32_t id)
{
    return id & LWDISTCOMM_SERVER_CLI_HASH_MASK;
}

/* Find client */
static lwdistcomm_server_cli_t *lwdistcomm_server_cli_find(lwdistcomm_server_t *server, uint32_t id)
{
    int hash = lwdistcomm_server_cli_hash(id);
    lwdistcomm_server_cli_t *cli;

    LIST_FOREACH(cli, server->clis[hash]) {
        if (cli->id == id) {
            break;
        }
    }

    return cli;
}

/* Assign new client ID */
static uint32_t lwdistcomm_server_cli_newid(lwdistcomm_server_t *server)
{
    uint32_t id;

    do {
        id = server->ncid;
        server->ncid++;
    } while (lwdistcomm_server_cli_find(server, id));

    return id;
}

/* Initialize a client */
static void lwdistcomm_server_cli_init(lwdistcomm_server_t *server, lwdistcomm_server_cli_t *cli)
{
    int hash;

    cli->id = lwdistcomm_server_cli_newid(server);
    hash = lwdistcomm_server_cli_hash(cli->id);
    INSERT_TO_HEADER(cli, server->clis[hash]);

    cli->hst.alive = LWDISTCOMM_SERVER_DEF_HANDSHAKE_TIMEOUT;
    INSERT_TO_HEADER(&cli->hst, server->hst_h);

    // Set TCP_NODELAY for better performance
    int nodelay = 1;
    setsockopt(cli->sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
}

/* Destroy a client */
static void lwdistcomm_server_cli_destroy(lwdistcomm_server_t *server, lwdistcomm_server_cli_t *cli)
{
    int hash = lwdistcomm_server_cli_hash(cli->id);
    lwdistcomm_server_sub_t *sub, *sub_temp;

    LIST_FOREACH_SAFE(sub, sub_temp, cli->subscribed) {
        DELETE_FROM_LIST(sub, cli->subscribed);
        free(sub);
    }

    DELETE_FROM_LIST(cli, server->clis[hash]);

    if (cli->hst.alive) {
        cli->hst.alive = 0;
        DELETE_FROM_LIST(&cli->hst, server->hst_h);
    }

    lwdistcomm_transport_close(cli->sock);
    free(cli);
}

/* Client subscribe match */
static bool lwdistcomm_server_cli_sub_match(lwdistcomm_server_cli_t *cli, const char *url)
{
    size_t url_len = strlen(url);
    lwdistcomm_server_sub_t *sub;

    LIST_FOREACH(sub, cli->subscribed) {
        if (sub->len == 1) {
            return true; // Match all
        } else if (sub->len == url_len && !memcmp(sub->url, url, sub->len)) {
            return true; // Exact match
        } else if (sub->url[sub->len - 1] == '/') {
            // Prefix match
            size_t path_len = sub->len - 1;
            if (url_len >= path_len && !memcmp(sub->url, url, path_len)) {
                if (url_len == path_len || url[path_len] == '/') {
                    return true;
                }
            }
        }
    }

    return false;
}

/* Client send message */
static bool lwdistcomm_server_cli_sendmsg(lwdistcomm_server_cli_t *cli, lwdistcomm_msg_header_t *header, const char *url, const lwdistcomm_message_t *msg)
{
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        return false;
    }

    uint8_t *buffer = (uint8_t *)header;
    ssize_t num, total = 0;

    do {
        num = lwdistcomm_transport_send(cli->sock, &buffer[total], len - total);
        if (num > 0) {
            total += num;
        } else {
            // Send failed, client is disconnected
            return false;
        }
    } while (total < len);

    return true;
}

/* Command match */
static bool lwdistcomm_server_cmd_match(lwdistcomm_server_t *server, const char *url, lwdistcomm_server_handler_cb_t *callback, void **arg)
{
    size_t url_len = strlen(url);
    lwdistcomm_server_cmd_t *cmd;

    // Exact match
    int hash = 0;
    for (size_t i = 0; i < url_len; i += 2) {
        hash += url[i];
    }
    hash &= LWDISTCOMM_SERVER_CMD_HASH_MASK;

    LIST_FOREACH(cmd, server->cmds[hash]) {
        if (cmd->len == url_len && !memcmp(cmd->url, url, url_len)) {
            *callback = cmd->callback;
            *arg = cmd->arg;
            return true;
        }
    }

    // Prefix match
    LIST_FOREACH(cmd, server->prefix_h) {
        if (cmd->len <= url_len && !memcmp(cmd->url, url, cmd->len)) {
            if (cmd->len == url_len || url[cmd->len] == '/') {
                *callback = cmd->callback;
                *arg = cmd->arg;
                return true;
            }
        }
    }

    // Default handler
    if (server->def_cmd) {
        *callback = server->def_cmd->callback;
        *arg = server->def_cmd->arg;
        return true;
    }

    return false;
}

/* Create server instance */
lwdistcomm_server_t *lwdistcomm_server_create(const lwdistcomm_server_options_t *options)
{
    int err = 0;
    lwdistcomm_server_t *server;

    server = (lwdistcomm_server_t *)malloc(sizeof(lwdistcomm_server_t));
    if (!server) {
        return NULL;
    }

    memset(server, 0, sizeof(lwdistcomm_server_t));

    server->sock = -1;

    // Create event fd pair
    server->evtfd[0] = eventfd(0, EFD_NONBLOCK);
    server->evtfd[1] = eventfd(0, EFD_NONBLOCK);
    if (server->evtfd[0] < 0 || server->evtfd[1] < 0) {
        goto error;
    }

    // Allocate send buffer
    server->sendbuf = malloc(LWDISTCOMM_MSG_MAX_LEN * 2);
    if (!server->sendbuf) {
        err = 1;
        goto error;
    }

    // Initialize security if options provided
    if (options && options->security_options) {
        server->security = lwdistcomm_security_create(options->security_options);
    }

    // Initialize receive buffer
    server->recvbuf = (uint8_t *)server->sendbuf + LWDISTCOMM_MSG_MAX_LEN;
    server->send_timeout = lwdistcomm_server_def_send_timeout;
    server->valid = true;

    // Initialize discovery fields
    server->enable_discovery = true; // Enable discovery by default
    server->discovery_port = 7400; // Default discovery port
    server->discovery_socket = -1;
    server->discovery_thread_running = false;
    strncpy(server->server_name, "LwDistComm-Server", sizeof(server->server_name));

    // Add to server list
    // TODO: Implement proper locking
    INSERT_TO_HEADER(server, lwdistcomm_server_list);

    // Start timer thread if first server
    if (lwdistcomm_server_list == server) {
        // TODO: Start timer thread
    }

    return server;

error:
    if (err > 1) {
        free(server->sendbuf);
    }
    if (err > 0) {
        if (server->evtfd[0] >= 0) close(server->evtfd[0]);
        if (server->evtfd[1] >= 0) close(server->evtfd[1]);
    }
    free(server);
    return NULL;
}

/* Start server */
bool lwdistcomm_server_start(lwdistcomm_server_t *server, const lwdistcomm_address_t *addr)
{
    if (!server || !server->valid || !addr) {
        return false;
    }

    // Create socket based on address type
    server->sock = lwdistcomm_transport_create_socket(addr->type, false, SOCK_STREAM);
    if (server->sock < 0) {
        return false;
    }

    // Set socket options
    int en = 1;
    setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en));
    setsockopt(server->sock, IPPROTO_TCP, TCP_NODELAY, &en, sizeof(en));

    // Bind to address
    if (!lwdistcomm_transport_bind(server->sock, addr)) {
        lwdistcomm_transport_close(server->sock);
        server->sock = -1;
        return false;
    }

    // Bind to specific interface if specified
    if (server->ifname[0]) {
        // TODO: Implement interface binding
    }

    // Start listening
    if (!lwdistcomm_transport_listen(server->sock, LWDISTCOMM_SERVER_BACKLOG)) {
        lwdistcomm_transport_close(server->sock);
        server->sock = -1;
        return false;
    }

    // Start discovery thread
    lwdistcomm_server_start_discovery(server);

    return true;
}

/* Stop server */
bool lwdistcomm_server_stop(lwdistcomm_server_t *server)
{
    if (!server || !server->valid) {
        return false;
    }

    if (server->sock >= 0) {
        lwdistcomm_transport_close(server->sock);
        server->sock = -1;
    }

    return true;
}

/* Check if server is running */
bool lwdistcomm_server_is_running(const lwdistcomm_server_t *server)
{
    return (server && server->valid && server->sock >= 0);
}

/* Publish message to subscribers */
bool lwdistcomm_server_publish(lwdistcomm_server_t *server, const char *url, const lwdistcomm_message_t *msg)
{
    if (!server || !server->valid || !url) {
        return false;
    }

    // Initialize message header
    lwdistcomm_msg_header_t *header = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_PUBLISH, 0, 0);

    // Set URL
    if (!lwdistcomm_msg_set_url(header, url)) {
        return false;
    }

    // Set payload
    if (msg && !lwdistcomm_msg_set_payload(header, msg)) {
        return false;
    }

    // Validate header
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        return false;
    }

    // Send to subscribed clients
    for (int i = 0; i < LWDISTCOMM_SERVER_CLI_HASH_SIZE; i++) {
        lwdistcomm_server_cli_t *cli, *cli_temp;
        LIST_FOREACH_SAFE(cli, cli_temp, server->clis[i]) {
            if (cli->active && lwdistcomm_server_cli_sub_match(cli, url)) {
                if (!lwdistcomm_server_cli_sendmsg(cli, header, url, msg)) {
                    // Send failed, client is disconnected
                    lwdistcomm_server_cli_destroy(server, cli);
                }
            }
        }
    }

    return true;
}

/* Add RPC handler */
bool lwdistcomm_server_add_handler(lwdistcomm_server_t *server, const char *url, lwdistcomm_server_handler_cb_t callback, void *arg)
{
    if (!server || !server->valid || !url || !callback) {
        return false;
    }

    size_t url_len = strlen(url);
    bool is_prefix = (url[url_len - 1] == '/');
    bool is_default = (url_len == 1 && url[0] == '/');

    lwdistcomm_server_cmd_t *cmd = (lwdistcomm_server_cmd_t *)malloc(sizeof(lwdistcomm_server_cmd_t) + url_len);
    if (!cmd) {
        return false;
    }

    memset(cmd, 0, sizeof(lwdistcomm_server_cmd_t));
    cmd->callback = callback;
    cmd->arg = arg;
    cmd->len = is_prefix ? url_len - 1 : url_len;
    memcpy(cmd->url, url, cmd->len);
    cmd->url[cmd->len] = '\0';

    // Lock server
    // TODO: Implement proper locking

    if (is_default) {
        if (server->def_cmd) {
            free(server->def_cmd);
        }
        server->def_cmd = cmd;
    } else if (is_prefix) {
        INSERT_TO_FIFO(cmd, server->prefix_h, server->prefix_t);
    } else {
        int hash = 0;
        for (size_t i = 0; i < cmd->len; i += 2) {
            hash += cmd->url[i];
        }
        hash &= LWDISTCOMM_SERVER_CMD_HASH_MASK;
        INSERT_TO_HEADER(cmd, server->cmds[hash]);
    }

    // Unlock server
    // TODO: Implement proper locking

    return true;
}

/* Remove RPC handler */
bool lwdistcomm_server_remove_handler(lwdistcomm_server_t *server, const char *url)
{
    if (!server || !server->valid || !url) {
        return false;
    }

    size_t url_len = strlen(url);
    bool is_prefix = (url[url_len - 1] == '/');
    bool is_default = (url_len == 1 && url[0] == '/');
    size_t path_len = is_prefix ? url_len - 1 : url_len;

    lwdistcomm_server_cmd_t *cmd = NULL;

    // Lock server
    // TODO: Implement proper locking

    if (is_default) {
        cmd = server->def_cmd;
        server->def_cmd = NULL;
    } else if (is_prefix) {
        lwdistcomm_server_cmd_t *temp;
        LIST_FOREACH_SAFE(cmd, temp, server->prefix_h) {
            if (cmd->len == path_len && !memcmp(cmd->url, url, path_len)) {
                DELETE_FROM_LIST(cmd, server->prefix_h);
                break;
            }
        }
    } else {
        int hash = 0;
        for (size_t i = 0; i < path_len; i += 2) {
            hash += url[i];
        }
        hash &= LWDISTCOMM_SERVER_CMD_HASH_MASK;

        lwdistcomm_server_cmd_t *temp;
        LIST_FOREACH_SAFE(cmd, temp, server->cmds[hash]) {
            if (cmd->len == path_len && !memcmp(cmd->url, url, path_len)) {
                DELETE_FROM_LIST(cmd, server->cmds[hash]);
                break;
            }
        }
    }

    // Unlock server
    // TODO: Implement proper locking

    if (cmd) {
        free(cmd);
        return true;
    }

    return false;
}

/* Set authentication callback */
void lwdistcomm_server_set_auth_callback(lwdistcomm_server_t *server, lwdistcomm_server_auth_cb_t callback, void *arg)
{
    if (server) {
        server->onauth = callback;
        server->aarg = arg;
    }
}

/* Set client connect/disconnect callback */
void lwdistcomm_server_set_client_callback(lwdistcomm_server_t *server, lwdistcomm_server_client_cb_t callback, void *arg)
{
    if (server) {
        server->oncli = callback;
        server->carg = arg;
    }
}

/* Set datagram callback */
void lwdistcomm_server_set_datagram_callback(lwdistcomm_server_t *server, lwdistcomm_server_datagram_cb_t callback, void *arg)
{
    if (server) {
        server->ondat = callback;
        server->darg = arg;
    }
}

/* Get number of connected clients */
int lwdistcomm_server_get_client_count(const lwdistcomm_server_t *server)
{
    if (!server || !server->valid) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < LWDISTCOMM_SERVER_CLI_HASH_SIZE; i++) {
        lwdistcomm_server_cli_t *cli;
        LIST_FOREACH(cli, server->clis[i]) {
            if (cli->active) {
                count++;
            }
        }
    }

    return count;
}

/* Process events */
bool lwdistcomm_server_process_events(lwdistcomm_server_t *server)
{
    if (!server || !server->valid) {
        return false;
    }

    fd_set rfds;
    int max_fd = lwdistcomm_server_get_fds(server, &rfds);
    if (max_fd < 0) {
        return false;
    }

    struct timeval timeout = {0, 10000}; // 10ms timeout
    int ret = select(max_fd + 1, &rfds, NULL, NULL, &timeout);
    if (ret > 0) {
        return lwdistcomm_server_process_input(server, &rfds);
    }

    return true;
}

/* Get file descriptors for event polling */
int lwdistcomm_server_get_fds(lwdistcomm_server_t *server, fd_set *rfds)
{
    if (!server || !server->valid || !rfds || server->sock < 0) {
        return -1;
    }

    FD_ZERO(rfds);

    FD_SET(server->sock, rfds);
    int max_fd = server->sock;

    FD_SET(server->evtfd[0], rfds);
    if (max_fd < server->evtfd[0]) {
        max_fd = server->evtfd[0];
    }

    // Add client sockets
    for (int i = 0; i < LWDISTCOMM_SERVER_CLI_HASH_SIZE; i++) {
        lwdistcomm_server_cli_t *cli;
        LIST_FOREACH(cli, server->clis[i]) {
            FD_SET(cli->sock, rfds);
            if (max_fd < cli->sock) {
                max_fd = cli->sock;
            }
        }
    }

    return max_fd;
}

/* Process input events */
bool lwdistcomm_server_process_input(lwdistcomm_server_t *server, const fd_set *rfds)
{
    if (!server || !server->valid || !rfds) {
        return false;
    }

    // Process client sockets
    for (int i = 0; i < LWDISTCOMM_SERVER_CLI_HASH_SIZE; i++) {
        lwdistcomm_server_cli_t *cli, *cli_temp;
        LIST_FOREACH_SAFE(cli, cli_temp, server->clis[i]) {
            if (FD_ISSET(cli->sock, rfds)) {
                ssize_t num = lwdistcomm_transport_recv(cli->sock, server->recvbuf, LWDISTCOMM_MSG_MAX_LEN, 0);
                if (num > 0) {
                    struct {
                        lwdistcomm_server_t *server;
                        lwdistcomm_server_cli_t *cli;
                    } input_arg = {server, cli};

                    lwdistcomm_msg_input(&cli->recv, server->recvbuf, num, lwdistcomm_server_input, &input_arg);
                }

                if (num == 0 || (num < 0 && errno != EWOULDBLOCK)) {
                    if (cli->onconn) {
                        cli->onconn = false;
                        if (server->oncli) {
                            server->oncli(server->carg, cli->id, false);
                        }
                    }

                    // Lock server
                    // TODO: Implement proper locking

                    lwdistcomm_server_cli_destroy(server, cli);

                    // Unlock server
                    // TODO: Implement proper locking
                }
            }
        }
    }

    // Process server socket (new connections)
    if (server->sock >= 0 && FD_ISSET(server->sock, rfds)) {
        struct sockaddr_storage addr;
        socklen_t addr_len = sizeof(addr);
        int sock = lwdistcomm_transport_accept(server->sock, (struct sockaddr *)&addr, &addr_len);
        if (sock >= 0) {
            lwdistcomm_server_cli_t *cli = (lwdistcomm_server_cli_t *)malloc(sizeof(lwdistcomm_server_cli_t));
            if (cli) {
                memset(cli, 0, sizeof(lwdistcomm_server_cli_t));
                cli->sock = sock;
                cli->active = false;
                lwdistcomm_msg_init_recv(&cli->recv);
                lwdistcomm_transport_set_timeout(sock, LWDISTCOMM_SERVER_DEF_SEND_TIMEOUT);

                // Lock server
                // TODO: Implement proper locking

                lwdistcomm_server_cli_init(server, cli);

                // Unlock server
                // TODO: Implement proper locking

            } else {
                lwdistcomm_transport_close(sock);
            }
        }
    }

    // Process event fd
    if (FD_ISSET(server->evtfd[0], rfds)) {
        uint64_t val;
        read(server->evtfd[0], &val, sizeof(val));

        // Lock server
        // TODO: Implement proper locking

        lwdistcomm_server_hst_t *hst, *hst_temp;
        LIST_FOREACH_SAFE(hst, hst_temp, server->hst_h) {
            if (hst->alive <= 0) {
                hst->alive = 0;
                DELETE_FROM_LIST(hst, server->hst_h);

                // Get client from handshake timer
                lwdistcomm_server_cli_t *cli = (lwdistcomm_server_cli_t *)((char *)hst - offsetof(lwdistcomm_server_cli_t, hst));
                lwdistcomm_server_cli_destroy(server, cli);
            }
        }

        // Unlock server
        // TODO: Implement proper locking
    }

    return true;
}

#define DISCOVERY_MSG_MAX_SIZE 1024
#define DISCOVERY_INTERVAL_MS 1000

/**
 * 发现消息类型
 */
typedef enum {
    DISCOVERY_MSG_SERVER_ANNOUNCE,
    DISCOVERY_MSG_SERVER_LEAVE,
    DISCOVERY_MSG_TOPIC_ANNOUNCE
} discovery_msg_type_t;

/**
 * 发现消息结构
 */
typedef struct {
    discovery_msg_type_t type;
    char server_name[64];
    uint16_t port;
    char topic[128];
} discovery_msg_t;

/**
 * 序列化发现消息
 */
static int serialize_discovery_msg(const discovery_msg_t *msg, char *buffer, size_t buffer_size)
{
    if (!msg || !buffer) {
        return -1;
    }
    
    int offset = 0;
    memcpy(buffer + offset, &msg->type, sizeof(discovery_msg_type_t));
    offset += sizeof(discovery_msg_type_t);
    strncpy(buffer + offset, msg->server_name, sizeof(msg->server_name));
    offset += sizeof(msg->server_name);
    memcpy(buffer + offset, &msg->port, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    strncpy(buffer + offset, msg->topic, sizeof(msg->topic));
    offset += sizeof(msg->topic);
    
    return offset;
}

/**
 * 反序列化发现消息
 */
static int deserialize_discovery_msg(const char *buffer, size_t buffer_size, discovery_msg_t *msg)
{
    if (!buffer || !msg) {
        return -1;
    }
    
    int offset = 0;
    memcpy(&msg->type, buffer + offset, sizeof(discovery_msg_type_t));
    offset += sizeof(discovery_msg_type_t);
    strncpy(msg->server_name, buffer + offset, sizeof(msg->server_name));
    offset += sizeof(msg->server_name);
    memcpy(&msg->port, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    strncpy(msg->topic, buffer + offset, sizeof(msg->topic));
    offset += sizeof(msg->topic);
    
    return offset;
}

/**
 * 发送发现消息
 */
static int send_discovery_msg(int sockfd, const discovery_msg_t *msg, const struct sockaddr_in *addr)
{
    if (!msg || !addr) {
        return -1;
    }
    
    char buffer[DISCOVERY_MSG_MAX_SIZE];
    int msg_size = serialize_discovery_msg(msg, buffer, sizeof(buffer));
    if (msg_size < 0) {
        return -1;
    }
    
    return sendto(sockfd, buffer, msg_size, 0, (struct sockaddr *)addr, sizeof(*addr));
}

/**
 * 服务器发现线程函数
 */
static void *lwdistcomm_server_discovery_thread_func(void *arg)
{
    lwdistcomm_server_t *server = (lwdistcomm_server_t *)arg;
    int sockfd;
    struct sockaddr_in addr;
    char buffer[DISCOVERY_MSG_MAX_SIZE];
    socklen_t addr_len;
    
    /* 创建UDP套接字 */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return NULL;
    }
    
    /* 设置地址重用 */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* 设置广播权限 */
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    
    /* 绑定地址 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(server->discovery_port);
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return NULL;
    }
    
    server->discovery_socket = sockfd;
    server->discovery_thread_running = true;
    
    /* 发送服务器宣告消息 */
    discovery_msg_t announce_msg;
    memset(&announce_msg, 0, sizeof(announce_msg));
    announce_msg.type = DISCOVERY_MSG_SERVER_ANNOUNCE;
    strncpy(announce_msg.server_name, server->server_name, sizeof(announce_msg.server_name));
    announce_msg.port = 5555; // Default port
    strncpy(announce_msg.topic, "/dds/test/topic", sizeof(announce_msg.topic));
    
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(server->discovery_port);
    
    /* 主循环 */
    while (server->discovery_thread_running) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sockfd, &read_set);
        
        struct timeval timeout;
        timeout.tv_sec = DISCOVERY_INTERVAL_MS / 1000;
        timeout.tv_usec = (DISCOVERY_INTERVAL_MS % 1000) * 1000;
        
        int ret = select(sockfd + 1, &read_set, NULL, NULL, &timeout);
        if (ret < 0) {
            continue;
        }
        
        if (ret > 0 && FD_ISSET(sockfd, &read_set)) {
            addr_len = sizeof(addr);
            int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
            if (n > 0) {
                // Process discovery message
                discovery_msg_t msg;
                if (deserialize_discovery_msg(buffer, n, &msg) > 0) {
                    // Handle discovery messages if needed
                }
            }
        } else {
            // Send periodic announcement
            send_discovery_msg(sockfd, &announce_msg, &broadcast_addr);
        }
    }
    
    /* 发送服务器离开消息 */
    discovery_msg_t leave_msg;
    memset(&leave_msg, 0, sizeof(leave_msg));
    leave_msg.type = DISCOVERY_MSG_SERVER_LEAVE;
    strncpy(leave_msg.server_name, server->server_name, sizeof(leave_msg.server_name));
    send_discovery_msg(sockfd, &leave_msg, &broadcast_addr);
    
    close(sockfd);
    server->discovery_socket = -1;
    
    return NULL;
}

/**
 * 启动服务器发现
 */
bool lwdistcomm_server_start_discovery(lwdistcomm_server_t *server)
{
    if (!server || !server->valid) {
        return false;
    }
    
    if (!server->enable_discovery) {
        return true;
    }
    
    if (server->discovery_thread_running) {
        return true;
    }
    
    int ret = pthread_create(&server->discovery_thread, NULL, lwdistcomm_server_discovery_thread_func, server);
    if (ret != 0) {
        return false;
    }
    
    return true;
}

/**
 * 停止服务器发现
 */
bool lwdistcomm_server_stop_discovery(lwdistcomm_server_t *server)
{
    if (!server || !server->valid) {
        return false;
    }
    
    if (!server->discovery_thread_running) {
        return true;
    }
    
    server->discovery_thread_running = false;
    pthread_join(server->discovery_thread, NULL);
    
    return true;
}

/* Destroy server instance */
void lwdistcomm_server_destroy(lwdistcomm_server_t *server)
{
    if (!server || !server->valid) {
        return;
    }

    server->valid = false;

    // Stop discovery thread
    lwdistcomm_server_stop_discovery(server);

    // Remove from server list
    // TODO: Implement proper locking
    DELETE_FROM_LIST(server, lwdistcomm_server_list);

    if (server->sock >= 0) {
        lwdistcomm_transport_close(server->sock);
        server->sock = -1;
    }

    close(server->evtfd[0]);
    close(server->evtfd[1]);
    free(server->sendbuf);

    // Cleanup clients
    for (int i = 0; i < LWDISTCOMM_SERVER_CLI_HASH_SIZE; i++) {
        lwdistcomm_server_cli_t *cli, *cli_temp;
        LIST_FOREACH_SAFE(cli, cli_temp, server->clis[i]) {
            lwdistcomm_server_cli_destroy(server, cli);
        }
    }

    // Cleanup commands
    for (int i = 0; i < LWDISTCOMM_SERVER_CMD_HASH_SIZE; i++) {
        lwdistcomm_server_cmd_t *cmd, *cmd_temp;
        LIST_FOREACH_SAFE(cmd, cmd_temp, server->cmds[i]) {
            DELETE_FROM_LIST(cmd, server->cmds[i]);
            free(cmd);
        }
    }

    // Cleanup prefix commands
    lwdistcomm_server_cmd_t *cmd, *cmd_temp;
    LIST_FOREACH_SAFE(cmd, cmd_temp, server->prefix_h) {
        DELETE_FROM_LIST(cmd, server->prefix_h);
        free(cmd);
    }

    // Cleanup default command
    if (server->def_cmd) {
        free(server->def_cmd);
    }

    // Destroy security context
    if (server->security) {
        lwdistcomm_security_destroy(server->security);
    }

    free(server);
}

/* Server input callback */
static bool lwdistcomm_server_input(void *arg, lwdistcomm_msg_header_t *header)
{
    struct {
        lwdistcomm_server_t *server;
        lwdistcomm_server_cli_t *cli;
    } *input_arg = arg;
    lwdistcomm_server_t *server = input_arg->server;
    lwdistcomm_server_cli_t *cli = input_arg->cli;

    if (header->type == LWDISTCOMM_MSG_TYPE_NOOP || header->type == LWDISTCOMM_MSG_FLAG_REPLY) {
        return true;
    }

    char *url;
    size_t url_len;
    lwdistcomm_message_t msg;

    lwdistcomm_msg_get_url(header, &url, &url_len);
    lwdistcomm_msg_get_payload(header, &msg);

    if (!cli->active) {
        cli->active = true;
    }

    if (header->type == LWDISTCOMM_MSG_TYPE_DATAGRAM) {
        if (server->ondat) {
            server->ondat(server->darg, cli->id, url, &msg);
        }
        return true;
    }

    switch (header->type) {
    case LWDISTCOMM_MSG_TYPE_SERVINFO:
    {
        // Send service info response
        lwdistcomm_msg_header_t *response = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_SERVINFO, 0, ntohs(header->seqno));
        lwdistcomm_message_t response_msg;
        response_msg.data = &cli->id;
        response_msg.data_len = sizeof(cli->id);
        lwdistcomm_msg_set_payload(response, &response_msg);
        if (!lwdistcomm_server_cli_sendmsg(cli, response, NULL, &response_msg)) {
            // Send failed, client is disconnected
            lwdistcomm_server_cli_destroy(server, cli);
            return true;
        }

        // Remove from handshake list
        cli->hst.alive = 0;
        DELETE_FROM_LIST(&cli->hst, server->hst_h);

        // Notify client connected
        if (!cli->onconn) {
            cli->onconn = true;
            if (server->oncli) {
                server->oncli(server->carg, cli->id, true);
            }
        }
        break;
    }

    case LWDISTCOMM_MSG_TYPE_RPC:
    {
        lwdistcomm_server_handler_cb_t callback;
        void *cb_arg;
        if (lwdistcomm_server_cmd_match(server, url, &callback, &cb_arg)) {
            lwdistcomm_message_t response;
            memset(&response, 0, sizeof(response));
            callback(cb_arg, cli->id, url, &msg, &response);
            
            // Send response
            lwdistcomm_msg_header_t *response_header = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_RPC, 0, ntohs(header->seqno));
            if (response.data) {
                lwdistcomm_msg_set_payload(response_header, &response);
            }
            if (!lwdistcomm_server_cli_sendmsg(cli, response_header, NULL, response.data ? &response : NULL)) {
                // Send failed, client is disconnected
                lwdistcomm_server_cli_destroy(server, cli);
                return true;
            }
        } else {
            // Send error response
            lwdistcomm_msg_header_t *response = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_RPC, LWDISTCOMM_STATUS_INVALID_URL, ntohs(header->seqno));
            if (!lwdistcomm_server_cli_sendmsg(cli, response, NULL, NULL)) {
                // Send failed, client is disconnected
                lwdistcomm_server_cli_destroy(server, cli);
                return true;
            }
        }
        break;
    }

    case LWDISTCOMM_MSG_TYPE_SUBSCRIBE:
    {
        lwdistcomm_server_sub_t *sub;
        LIST_FOREACH(sub, cli->subscribed) {
            if (sub->len == url_len && !memcmp(sub->url, url, sub->len)) {
                break;
            }
        }

        if (!sub) {
            sub = (lwdistcomm_server_sub_t *)malloc(sizeof(lwdistcomm_server_sub_t) + url_len);
            if (sub) {
                memset(sub, 0, sizeof(lwdistcomm_server_sub_t));
                sub->len = url_len;
                memcpy(sub->url, url, sub->len);
                sub->url[sub->len] = '\0';
                INSERT_TO_HEADER(sub, cli->subscribed);
            }
        }

        // Send response
        lwdistcomm_msg_header_t *response = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_SUBSCRIBE, 0, ntohs(header->seqno));
        if (!lwdistcomm_server_cli_sendmsg(cli, response, NULL, NULL)) {
            // Send failed, client is disconnected
            lwdistcomm_server_cli_destroy(server, cli);
            return true;
        }
        break;
    }

    case LWDISTCOMM_MSG_TYPE_UNSUBSCRIBE:
    {
        lwdistcomm_server_sub_t *sub, *sub_temp;
        LIST_FOREACH_SAFE(sub, sub_temp, cli->subscribed) {
            if (sub->len == url_len && !memcmp(sub->url, url, sub->len)) {
                DELETE_FROM_LIST(sub, cli->subscribed);
                free(sub);
                break;
            }
        }

        // Send response
        lwdistcomm_msg_header_t *response = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_UNSUBSCRIBE, 0, ntohs(header->seqno));
        if (!lwdistcomm_server_cli_sendmsg(cli, response, NULL, NULL)) {
            // Send failed, client is disconnected
            lwdistcomm_server_cli_destroy(server, cli);
            return true;
        }
        break;
    }

    case LWDISTCOMM_MSG_TYPE_AUTH:
    {
        // TODO: Implement authentication
        bool auth_success = true;
        if (server->onauth) {
            // Extract username and password from payload
            // For now, just accept all
            auth_success = server->onauth(server->aarg, "", "");
        }

        // Send response
        lwdistcomm_msg_header_t *response = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_AUTH, auth_success ? 0 : LWDISTCOMM_STATUS_AUTH_FAILED, ntohs(header->seqno));
        if (!lwdistcomm_server_cli_sendmsg(cli, response, NULL, NULL)) {
            // Send failed, client is disconnected
            lwdistcomm_server_cli_destroy(server, cli);
            return true;
        }
        break;
    }

    case LWDISTCOMM_MSG_TYPE_PINGECHO:
    {
        // Send ping response
        lwdistcomm_msg_header_t *response = lwdistcomm_msg_init_header(server->sendbuf, LWDISTCOMM_MSG_TYPE_PINGECHO, 0, ntohs(header->seqno));
        if (!lwdistcomm_server_cli_sendmsg(cli, response, NULL, NULL)) {
            // Send failed, client is disconnected
            lwdistcomm_server_cli_destroy(server, cli);
            return true;
        }
        break;
    }

    default:
        break;
    }

    return server->valid;
}
