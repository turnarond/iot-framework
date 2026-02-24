/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: client.c Client implementation for LwDistComm.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "client_impl.h"

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
static const struct timeval lwdistcomm_client_def_send_timeout = {
    .tv_sec  = (LWDISTCOMM_CLIENT_DEF_SEND_TIMEOUT / 1000),
    .tv_usec = (LWDISTCOMM_CLIENT_DEF_SEND_TIMEOUT % 1000) * 1000
};

/* Clients list lock, header, timer thread */
static void *lwdistcomm_client_timer = NULL;
static lwdistcomm_client_t *lwdistcomm_client_list = NULL;
static void *lwdistcomm_client_lock = NULL;

/* Client fast pending buffer check */
#define LWDISTCOMM_CLIENT_PENDING_IS_IN_POOL(client, pending) \
        ((pending) >= client->pool && \
        (pending) <= client->pool + LWDISTCOMM_CLIENT_FAST_PENDING_POOL - 1)

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

/* Client timer thread handle */
static void *lwdistcomm_client_timer_handle(void *arg)
{
    bool emit;
    lwdistcomm_client_t *client;
    lwdistcomm_client_pendq_t *pendq, *temp;

    (void)arg;

    do {
        // Sleep for timer period
        usleep(LWDISTCOMM_CLIENT_TIMER_PERIOD * 1000);

        // Lock client list
        // TODO: Implement proper locking

        if (!lwdistcomm_client_list) {
            // No clients, exit loop
            break;
        }

        LIST_FOREACH(client, lwdistcomm_client_list) {
            emit = false;

            // Lock client
            // TODO: Implement proper locking

            LIST_FOREACH_SAFE(pendq, temp, client->head) {
                if (pendq->alive > LWDISTCOMM_CLIENT_TIMER_PERIOD) {
                    pendq->alive -= LWDISTCOMM_CLIENT_TIMER_PERIOD;
                } else {
                    pendq->alive = 0;
                    emit = true;
                    break;
                }
            }

            // Unlock client
            // TODO: Implement proper locking

            if (emit) {
                // Signal event
                uint64_t val = 1;
                write(client->evtfd[1], &val, sizeof(val));
            }
        }

        // Unlock client list
        // TODO: Implement proper locking

    } while (true);

    return NULL;
}

/* Pendq free */
static void lwdistcomm_client_pendq_free(lwdistcomm_client_t *client, lwdistcomm_client_pendq_t *pendq)
{
    if (LWDISTCOMM_CLIENT_PENDING_IS_IN_POOL(client, pendq)) {
        // Lock client
        // TODO: Implement proper locking

        INSERT_TO_HEADER(pendq, client->free);

        // Unlock client
        // TODO: Implement proper locking

    } else {
        free(pendq);
    }
}

/* Create client instance */
lwdistcomm_client_t *lwdistcomm_client_create(const lwdistcomm_client_options_t *options)
{
    int i, err = 0;
    lwdistcomm_client_t *client;

    client = (lwdistcomm_client_t *)malloc(sizeof(lwdistcomm_client_t) + 
                                           (sizeof(lwdistcomm_client_pendq_t) * LWDISTCOMM_CLIENT_FAST_PENDING_POOL));
    if (!client) {
        return NULL;
    }

    memset(client, 0, sizeof(lwdistcomm_client_t));

    client->sock = -1;

    // Create event fd pair
    client->evtfd[0] = eventfd(0, EFD_NONBLOCK);
    client->evtfd[1] = eventfd(0, EFD_NONBLOCK);
    if (client->evtfd[0] < 0 || client->evtfd[1] < 0) {
        goto error;
    }

    // Allocate send buffer
    client->sendbuf = malloc(LWDISTCOMM_MSG_MAX_LEN * 2);
    if (!client->sendbuf) {
        err = 1;
        goto error;
    }

    // Initialize fast pending pool
    client->pool = (lwdistcomm_client_pendq_t *)(client + 1);
    for (i = 0; i < LWDISTCOMM_CLIENT_FAST_PENDING_POOL; i++) {
        INSERT_TO_HEADER(&client->pool[i], client->free);
    }

    // Initialize security if options provided
    if (options && options->security_options) {
        client->security = lwdistcomm_security_create(options->security_options);
        if (options->auth_info) {
            lwdistcomm_security_set_auth(client->security, options->auth_info);
        }
    }

    // Initialize receive buffer
    client->recvbuf = (uint8_t *)client->sendbuf + LWDISTCOMM_MSG_MAX_LEN;
    lwdistcomm_msg_init_recv(&client->recv);
    client->send_timeout = lwdistcomm_client_def_send_timeout;
    client->valid = true;

    // Initialize discovery fields
    client->enable_discovery = true; // Enable discovery by default
    client->discovery_port = 7400; // Default discovery port
    client->discovery_socket = -1;
    client->discovery_thread_running = false;
    strncpy(client->client_name, "LwDistComm-Client", sizeof(client->client_name));
    memset(&client->discovered_server, 0, sizeof(client->discovered_server));
    strcpy(client->discovered_server.ip, "127.0.0.1"); // Default IP
    client->discovered_server.port = 5555; // Default port

    // Add to client list
    // TODO: Implement proper locking
    INSERT_TO_HEADER(client, lwdistcomm_client_list);

    // Start timer thread if first client
    if (lwdistcomm_client_list == client) {
        // TODO: Start timer thread
    }

    // Start discovery thread
    lwdistcomm_client_start_discovery(client);

    return client;

error:
    if (err > 1) {
        free(client->sendbuf);
    }
    if (err > 0) {
        if (client->evtfd[0] >= 0) close(client->evtfd[0]);
        if (client->evtfd[1] >= 0) close(client->evtfd[1]);
    }
    free(client);
    return NULL;
}

/* Connect to server */
bool lwdistcomm_client_connect(lwdistcomm_client_t *client, const lwdistcomm_address_t *addr)
{
    if (!client || !client->valid || !addr) {
        return false;
    }

    client->connected = false;

    if (client->sock >= 0) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
    }

    lwdistcomm_client_timeout_all(client);

    // Create socket based on address type
    client->sock = lwdistcomm_transport_create_socket(addr->type, false, SOCK_STREAM);
    if (client->sock < 0) {
        return false;
    }

    // Connect to server
    if (!lwdistcomm_transport_connect(client->sock, addr)) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
        return false;
    }

    // Set send timeout
    lwdistcomm_transport_set_timeout(client->sock, LWDISTCOMM_CLIENT_DEF_SEND_TIMEOUT);

    // Send service info request
    lwdistcomm_msg_header_t *header = lwdistcomm_msg_init_header(client->sendbuf, LWDISTCOMM_MSG_TYPE_SERVINFO, 0, 0);
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
        return false;
    }

    if (!lwdistcomm_client_send(client, len)) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
        return false;
    }

    // Receive service info response
    uint8_t buf[1024];
    ssize_t num = lwdistcomm_transport_recv(client->sock, buf, sizeof(buf), 0);
    if (num <= 0) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
        return false;
    }

    // Process response
    lwdistcomm_msg_recv_t recv;
    lwdistcomm_msg_init_recv(&recv);
    lwdistcomm_msg_input(&recv, buf, num, NULL, NULL);

    client->connected = true;
    return true;
}

/* Disconnect from server */
bool lwdistcomm_client_disconnect(lwdistcomm_client_t *client)
{
    if (!client || !client->valid || !client->connected) {
        return false;
    }

    client->connected = false;

    if (client->sock >= 0) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
    }

    lwdistcomm_client_timeout_all(client);
    return true;
}

/* Check if client is connected */
bool lwdistcomm_client_is_connected(const lwdistcomm_client_t *client)
{
    return (client ? (client->valid && client->connected) : false);
}

/* Send RPC request */
bool lwdistcomm_client_rpc(lwdistcomm_client_t *client, const char *url, const lwdistcomm_message_t *msg, lwdistcomm_client_rpc_cb_t callback, void *arg)
{
    if (!client || !client->valid || !client->connected || !url) {
        return false;
    }

    lwdistcomm_client_pendq_t *pendq = NULL;
    uint16_t seqno;

    if (callback) {
        pendq = lwdistcomm_client_prepare_pendq(client, false, arg, LWDISTCOMM_CLIENT_FTYPE_RPC, LWDISTCOMM_CLIENT_DEF_TIMEOUT);
        if (!pendq) {
            return false;
        }
        pendq->callback.rpc = callback;
        seqno = pendq->seqno;
    } else {
        seqno = lwdistcomm_client_prepare_seqno(client);
    }

    // Initialize message header
    lwdistcomm_msg_header_t *header = lwdistcomm_msg_init_header(client->sendbuf, LWDISTCOMM_MSG_TYPE_RPC, 0, seqno);

    // Set URL
    if (!lwdistcomm_msg_set_url(header, url)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Set payload
    if (msg && !lwdistcomm_msg_set_payload(header, msg)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Validate header
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Send message
    if (!lwdistcomm_client_send(client, len)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Add to pending queue
    if (pendq) {
        INSERT_TO_FIFO(pendq, client->head, client->tail);
        client->rpc_pending++;
    }

    return true;
}

/* Subscribe to topic */
bool lwdistcomm_client_subscribe(lwdistcomm_client_t *client, const char *url, lwdistcomm_client_message_cb_t callback, void *arg)
{
    if (!client || !client->valid || !client->connected || !url) {
        return false;
    }

    client->onmsg = callback;
    client->marg = arg;

    return lwdistcomm_client_request(client, LWDISTCOMM_MSG_TYPE_SUBSCRIBE, url, NULL, NULL, NULL, LWDISTCOMM_CLIENT_DEF_TIMEOUT);
}

/* Unsubscribe from topic */
bool lwdistcomm_client_unsubscribe(lwdistcomm_client_t *client, const char *url)
{
    if (!client || !client->valid || !client->connected || !url) {
        return false;
    }

    return lwdistcomm_client_request(client, LWDISTCOMM_MSG_TYPE_UNSUBSCRIBE, url, NULL, NULL, NULL, LWDISTCOMM_CLIENT_DEF_TIMEOUT);
}

/* Send datagram */
bool lwdistcomm_client_send_datagram(lwdistcomm_client_t *client, const char *url, const lwdistcomm_message_t *msg)
{
    if (!client || !client->valid || !client->connected || !url || !msg) {
        return false;
    }

    // Initialize message header
    lwdistcomm_msg_header_t *header = lwdistcomm_msg_init_header(client->sendbuf, LWDISTCOMM_MSG_TYPE_DATAGRAM, 0, 0);

    // Set URL
    if (!lwdistcomm_msg_set_url(header, url)) {
        return false;
    }

    // Set payload
    if (!lwdistcomm_msg_set_payload(header, msg)) {
        return false;
    }

    // Validate header
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        return false;
    }

    // Send message
    return lwdistcomm_client_send(client, len);
}

/* Set datagram callback */
void lwdistcomm_client_set_datagram_callback(lwdistcomm_client_t *client, lwdistcomm_client_datagram_cb_t callback, void *arg)
{
    if (client) {
        client->ondat = callback;
        client->darg = arg;
    }
}

/* Process events */
bool lwdistcomm_client_process_events(lwdistcomm_client_t *client)
{
    if (!client || !client->valid) {
        return false;
    }

    fd_set rfds;
    int max_fd = lwdistcomm_client_get_fds(client, &rfds);
    if (max_fd < 0) {
        return false;
    }

    struct timeval timeout = {0, 10000}; // 10ms timeout
    int ret = select(max_fd + 1, &rfds, NULL, NULL, &timeout);
    if (ret > 0) {
        return lwdistcomm_client_process_input(client, &rfds);
    }

    return true;
}

/* Get file descriptors for event polling */
int lwdistcomm_client_get_fds(lwdistcomm_client_t *client, fd_set *rfds)
{
    if (!client || !client->valid || !rfds) {
        return -1;
    }

    FD_ZERO(rfds);

    if (!client->connected) {
        FD_SET(client->evtfd[0], rfds);
        return client->evtfd[0];
    }

    FD_SET(client->sock, rfds);
    int max_fd = client->sock;

    FD_SET(client->evtfd[0], rfds);
    if (max_fd < client->evtfd[0]) {
        max_fd = client->evtfd[0];
    }

    return max_fd;
}

/* Process input events */
bool lwdistcomm_client_process_input(lwdistcomm_client_t *client, const fd_set *rfds)
{
    if (!client || !client->valid || !rfds) {
        return false;
    }

    if (client->connected) {
        if (FD_ISSET(client->sock, rfds)) {
            ssize_t num = lwdistcomm_transport_recv(client->sock, client->recvbuf, LWDISTCOMM_MSG_MAX_LEN, 0);
            if (num > 0) {
                lwdistcomm_msg_input(&client->recv, client->recvbuf, num, lwdistcomm_client_input, client);
            } else if (num == 0 || (num < 0 && errno != EWOULDBLOCK)) {
                client->connected = false;
                lwdistcomm_client_timeout_all(client);
                return false;
            }
        }
    }

    if (FD_ISSET(client->evtfd[0], rfds)) {
        uint64_t val;
        read(client->evtfd[0], &val, sizeof(val));

        lwdistcomm_client_pendq_t *pendq, *to_head = NULL, *to_tail = NULL;

        // Lock client
        // TODO: Implement proper locking

        LIST_FOREACH_SAFE(pendq, pendq, client->head) {
            if (pendq->alive <= 0) {
                if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_RPC) {
                    client->rpc_pending--;
                }
                DELETE_FROM_FIFO(pendq, client->head, client->tail);
                INSERT_TO_FIFO(pendq, to_head, to_tail);
            }
        }

        // Unlock client
        // TODO: Implement proper locking

        if (to_head) {
            LIST_FOREACH_SAFE(pendq, pendq, to_head) {
                if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_RPC && pendq->callback.rpc) {
                    pendq->callback.rpc(pendq->arg, LWDISTCOMM_STATUS_NO_RESPONDING, NULL);
                }
                DELETE_FROM_FIFO(pendq, to_head, to_tail);
                lwdistcomm_client_pendq_free(client, pendq);
            }
        }
    }

    return true;
}

/* Destroy client instance */
void lwdistcomm_client_destroy(lwdistcomm_client_t *client)
{
    if (!client || !client->valid) {
        return;
    }

    client->valid = false;

    // Stop discovery thread
    lwdistcomm_client_stop_discovery(client);

    // Remove from client list
    // TODO: Implement proper locking
    DELETE_FROM_LIST(client, lwdistcomm_client_list);

    if (client->sock >= 0) {
        lwdistcomm_transport_close(client->sock);
        client->sock = -1;
    }

    close(client->evtfd[0]);
    close(client->evtfd[1]);
    free(client->sendbuf);

    // Cleanup pending queue
    lwdistcomm_client_pendq_t *pendq, *temp;
    LIST_FOREACH_SAFE(pendq, temp, client->head) {
        if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_RPC && pendq->callback.rpc) {
            pendq->callback.rpc(pendq->arg, LWDISTCOMM_STATUS_NO_RESPONDING, NULL);
        }
        DELETE_FROM_FIFO(pendq, client->head, client->tail);
        lwdistcomm_client_pendq_free(client, pendq);
    }

    // Destroy security context
    if (client->security) {
        lwdistcomm_security_destroy(client->security);
    }

    free(client);
}

/* Prepare a non-queued seqno */
static uint16_t lwdistcomm_client_prepare_seqno(lwdistcomm_client_t *client)
{
    uint16_t seqno;

    // TODO: Implement atomic operation
    if (client->seqno_nq == 0) {
        seqno = 1;
        client->seqno_nq = 2;
    } else {
        seqno = client->seqno_nq;
        client->seqno_nq++;
    }

    return seqno << LWDISTCOMM_CLIENT_MAX_POFFSET;
}

/* Prepare a pendq */
static lwdistcomm_client_pendq_t *lwdistcomm_client_prepare_pendq(lwdistcomm_client_t *client, bool fast, void *arg, uint32_t ftype, int timeout)
{
    uint16_t i, seqno;
    lwdistcomm_client_pendq_t *pendq = NULL, *queued;

    if (fast) {
        // Lock client
        // TODO: Implement proper locking

        if (client->free) {
            pendq = client->free;
            DELETE_FROM_LIST(pendq, client->free);
        }

        // Unlock client
        // TODO: Implement proper locking
    }

    if (!pendq) {
        pendq = (lwdistcomm_client_pendq_t *)malloc(sizeof(lwdistcomm_client_pendq_t));
        if (!pendq) {
            return NULL;
        }
    }

    // Lock client
    // TODO: Implement proper locking

    for (i = 0; i < LWDISTCOMM_CLIENT_MAX_PENDING; i++) {
        seqno = client->seqno;
        client->seqno = (seqno + 1) & LWDISTCOMM_CLIENT_MAX_PENDING;

        LIST_FOREACH(queued, client->head) {
            if (queued->seqno == seqno) {
                break;
            }
        }

        if (!queued) {
            pendq->seqno = seqno;
            break;
        }
    }

    // Unlock client
    // TODO: Implement proper locking

    if (i >= LWDISTCOMM_CLIENT_MAX_PENDING) {
        lwdistcomm_client_pendq_free(client, pendq);
        return NULL;
    }

    pendq->arg = arg;
    pendq->ftype = ftype;
    pendq->alive = timeout;

    return pendq;
}

/* Client send */
static bool lwdistcomm_client_send(lwdistcomm_client_t *client, size_t len)
{
    uint8_t *buffer = (uint8_t *)client->sendbuf;
    ssize_t num, total = 0;

    do {
        num = lwdistcomm_transport_send(client->sock, &buffer[total], len - total);
        if (num > 0) {
            total += num;
        } else {
            return false;
        }
    } while (total < len);

    return true;
}

/* Client send message */
static bool lwdistcomm_client_sendmsg(lwdistcomm_client_t *client, lwdistcomm_msg_header_t *header, const char *url, const lwdistcomm_message_t *msg)
{
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        return false;
    }

    return lwdistcomm_client_send(client, len);
}

/* All RPC callback timeout */
static void lwdistcomm_client_timeout_all(lwdistcomm_client_t *client)
{
    lwdistcomm_client_pendq_t *pendq, *to_head = NULL, *temp;

    // Lock client
    // TODO: Implement proper locking

    to_head = client->head;
    client->head = client->tail = NULL;
    client->rpc_pending = 0;

    // Unlock client
    // TODO: Implement proper locking

    if (to_head) {
        LIST_FOREACH_SAFE(pendq, temp, to_head) {
            if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_RPC && pendq->callback.rpc) {
                pendq->callback.rpc(pendq->arg, LWDISTCOMM_STATUS_NO_RESPONDING, NULL);
            }
            DELETE_FROM_LIST(pendq, to_head);
            lwdistcomm_client_pendq_free(client, pendq);
        }
    }
}

/* Client input callback */
static bool lwdistcomm_client_input(void *arg, lwdistcomm_msg_header_t *header)
{
    lwdistcomm_client_t *client = (lwdistcomm_client_t *)arg;
    lwdistcomm_client_pendq_t *pendq, *temp;
    uint16_t seqno;
    char *url;
    size_t url_len;
    lwdistcomm_message_t msg;

    if (header->type == LWDISTCOMM_MSG_TYPE_PUBLISH || header->type == LWDISTCOMM_MSG_TYPE_DATAGRAM) {
        lwdistcomm_msg_get_url(header, &url, &url_len);
        lwdistcomm_msg_get_payload(header, &msg);

        if (header->type == LWDISTCOMM_MSG_TYPE_PUBLISH) {
            if (client->onmsg) {
                client->onmsg(client->marg, url, &msg);
            }
        } else if (client->ondat) {
            client->ondat(client->darg, url, &msg);
        }
        return true;
    }

    seqno = ntohs(header->seqno);

    // Lock client
    // TODO: Implement proper locking

    LIST_FOREACH_SAFE(pendq, temp, client->head) {
        if (pendq->seqno == seqno) {
            if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_RPC) {
                client->rpc_pending--;
            }
            DELETE_FROM_FIFO(pendq, client->head, client->tail);
            break;
        }
    }

    // Unlock client
    // TODO: Implement proper locking

    if (pendq) {
        switch (header->type) {
        case LWDISTCOMM_MSG_TYPE_SUBSCRIBE:
        case LWDISTCOMM_MSG_TYPE_UNSUBSCRIBE:
        case LWDISTCOMM_MSG_TYPE_PINGECHO:
            if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_SUB && pendq->callback.sub) {
                pendq->callback.sub(pendq->arg, header->status == 0);
            }
            break;

        case LWDISTCOMM_MSG_TYPE_RPC:
            if (pendq->ftype == LWDISTCOMM_CLIENT_FTYPE_RPC && pendq->callback.rpc) {
                lwdistcomm_msg_get_payload(header, &msg);
                pendq->callback.rpc(pendq->arg, header->status, &msg);
            }
            break;

        default:
            break;
        }

        lwdistcomm_client_pendq_free(client, pendq);
    }

    return client->valid;
}

/* Client request */
static bool lwdistcomm_client_request(lwdistcomm_client_t *client, uint8_t type, const char *url, const lwdistcomm_message_t *msg, lwdistcomm_client_subscribe_cb_t callback, void *arg, int timeout)
{
    lwdistcomm_client_pendq_t *pendq = NULL;
    uint16_t seqno;

    if (callback) {
        pendq = lwdistcomm_client_prepare_pendq(client, type == LWDISTCOMM_MSG_TYPE_PINGECHO, arg, LWDISTCOMM_CLIENT_FTYPE_SUB, timeout);
        if (!pendq) {
            return false;
        }
        pendq->callback.sub = callback;
        seqno = pendq->seqno;
    } else {
        seqno = lwdistcomm_client_prepare_seqno(client);
    }

    // Initialize message header
    lwdistcomm_msg_header_t *header = lwdistcomm_msg_init_header(client->sendbuf, type, 0, seqno);

    // Set URL
    if (!lwdistcomm_msg_set_url(header, url)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Set payload
    if (msg && !lwdistcomm_msg_set_payload(header, msg)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Validate header
    size_t len;
    if (!lwdistcomm_msg_validate_header(header, &len)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Send message
    if (!lwdistcomm_client_send(client, len)) {
        if (pendq) {
            lwdistcomm_client_pendq_free(client, pendq);
        }
        return false;
    }

    // Add to pending queue
    if (pendq) {
        INSERT_TO_FIFO(pendq, client->head, client->tail);
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
 * 客户端发现线程函数
 */
static void *lwdistcomm_client_discovery_thread_func(void *arg)
{
    lwdistcomm_client_t *client = (lwdistcomm_client_t *)arg;
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
    addr.sin_port = htons(client->discovery_port);
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return NULL;
    }
    
    client->discovery_socket = sockfd;
    client->discovery_thread_running = true;
    
    /* 主循环 */
    while (client->discovery_thread_running) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sockfd, &read_set);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ret = select(sockfd + 1, &read_set, NULL, NULL, &timeout);
        if (ret < 0) {
            continue;
        }
        
        if (ret > 0 && FD_ISSET(sockfd, &read_set)) {
            addr_len = sizeof(addr);
            int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
            if (n > 0) {
                // 处理发现消息
                discovery_msg_t msg;
                if (deserialize_discovery_msg(buffer, n, &msg) > 0) {
                    if (msg.type == DISCOVERY_MSG_SERVER_ANNOUNCE) {
                        // 发现新服务器
                        inet_ntop(AF_INET, &addr.sin_addr, client->discovered_server.ip, sizeof(client->discovered_server.ip));
                        strncpy(client->discovered_server.server_name, msg.server_name, sizeof(client->discovered_server.server_name));
                        client->discovered_server.port = msg.port;
                        
                        // 如果客户端未连接，则尝试连接
                        if (!client->connected) {
                            lwdistcomm_address_t *server_addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
                            if (server_addr) {
                                lwdistcomm_address_set_ipv4(server_addr, client->discovered_server.ip, client->discovered_server.port);
                                lwdistcomm_client_connect(client, server_addr);
                                lwdistcomm_address_destroy(server_addr);
                            }
                        }
                    }
                }
            }
        }
    }
    
    close(sockfd);
    client->discovery_socket = -1;
    
    return NULL;
}

/**
 * 启动客户端发现
 */
bool lwdistcomm_client_start_discovery(lwdistcomm_client_t *client)
{
    if (!client || !client->valid) {
        return false;
    }
    
    if (!client->enable_discovery) {
        return true;
    }
    
    if (client->discovery_thread_running) {
        return true;
    }
    
    int ret = pthread_create(&client->discovery_thread, NULL, lwdistcomm_client_discovery_thread_func, client);
    if (ret != 0) {
        return false;
    }
    
    return true;
}

/**
 * 停止客户端发现
 */
bool lwdistcomm_client_stop_discovery(lwdistcomm_client_t *client)
{
    if (!client || !client->valid) {
        return false;
    }
    
    if (!client->discovery_thread_running) {
        return true;
    }
    
    client->discovery_thread_running = false;
    pthread_join(client->discovery_thread, NULL);
    
    return true;
}

/**
 * 自动发现并连接服务器
 */
bool lwdistcomm_client_autodiscover(lwdistcomm_client_t *client)
{
    if (!client || !client->valid) {
        return false;
    }
    
    // 启动发现
    if (!lwdistcomm_client_start_discovery(client)) {
        return false;
    }
    
    // 等待发现服务器
    int attempts = 0;
    while (attempts < 5 && !client->connected) {
        usleep(1000000); // 1秒
        attempts++;
    }
    
    return client->connected;
}
