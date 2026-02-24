/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: transport.c Transport layer implementation for LwDistComm.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../include/address.h"

/* Create socket based on address type */
int lwdistcomm_transport_create_socket(lwdistcomm_addr_type_t type, bool non_blocking, int sock_type)
{
    int sock = -1;
    int domain;

    switch (type) {
    case LWDISTCOMM_ADDR_TYPE_UNIX:
        domain = AF_UNIX;
        break;
    case LWDISTCOMM_ADDR_TYPE_IPV4:
        domain = AF_INET;
        break;
    case LWDISTCOMM_ADDR_TYPE_IPV6:
        domain = AF_INET6;
        break;
    default:
        return -1;
    }

    sock = socket(domain, sock_type, 0);
    if (sock < 0) {
        return -1;
    }

    // Set non-blocking if requested
    if (non_blocking) {
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags < 0) {
            close(sock);
            return -1;
        }
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
            close(sock);
            return -1;
        }
    }

    // Set TCP_NODELAY for better performance if using TCP
    if (sock_type == SOCK_STREAM) {
        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    }

    return sock;
}

/* Create TCP socket */
int lwdistcomm_transport_create_tcp_socket(lwdistcomm_addr_type_t type, bool non_blocking)
{
    return lwdistcomm_transport_create_socket(type, non_blocking, SOCK_STREAM);
}

/* Create UDP socket */
int lwdistcomm_transport_create_udp_socket(lwdistcomm_addr_type_t type, bool non_blocking)
{
    return lwdistcomm_transport_create_socket(type, non_blocking, SOCK_DGRAM);
}

/* Connect to address */
bool lwdistcomm_transport_connect(int sock, const lwdistcomm_address_t *addr)
{
    if (sock < 0 || !addr) {
        return false;
    }

    struct sockaddr *saddr = lwdistcomm_address_get_sockaddr(addr);
    socklen_t addr_len = lwdistcomm_address_get_len(addr);

    if (!saddr || addr_len == 0) {
        return false;
    }

    int ret = connect(sock, saddr, addr_len);
    if (ret < 0) {
        return false;
    }

    return true;
}

/* Bind socket to address */
bool lwdistcomm_transport_bind(int sock, const lwdistcomm_address_t *addr)
{
    if (sock < 0 || !addr) {
        return false;
    }

    struct sockaddr *saddr = lwdistcomm_address_get_sockaddr(addr);
    socklen_t addr_len = lwdistcomm_address_get_len(addr);

    if (!saddr || addr_len == 0) {
        return false;
    }

    // For Unix domain socket, remove existing path
    if (addr->type == LWDISTCOMM_ADDR_TYPE_UNIX) {
        unlink(addr->addr.unix_addr.sun_path);
    }

    int ret = bind(sock, saddr, addr_len);
    if (ret < 0) {
        return false;
    }

    return true;
}

/* Listen for connections */
bool lwdistcomm_transport_listen(int sock, int backlog)
{
    if (sock < 0) {
        return false;
    }

    int ret = listen(sock, backlog);
    if (ret < 0) {
        return false;
    }

    return true;
}

/* Accept connection */
int lwdistcomm_transport_accept(int sock, struct sockaddr *addr, socklen_t *addr_len)
{
    if (sock < 0) {
        return -1;
    }

    return accept(sock, addr, addr_len);
}

/* Send data */
ssize_t lwdistcomm_transport_send(int sock, const void *data, size_t len)
{
    if (sock < 0 || !data || len == 0) {
        return -1;
    }

    // Use MSG_NOSIGNAL flag to prevent SIGPIPE
    return send(sock, data, len, MSG_NOSIGNAL);
}

/* Receive data */
ssize_t lwdistcomm_transport_recv(int sock, void *buffer, size_t len, int flags)
{
    if (sock < 0 || !buffer || len == 0) {
        return -1;
    }

    return recv(sock, buffer, len, flags);
}

/* Send UDP data */
ssize_t lwdistcomm_transport_sendto(int sock, const void *data, size_t len, const lwdistcomm_address_t *addr)
{
    if (sock < 0 || !data || len == 0 || !addr) {
        return -1;
    }

    struct sockaddr *saddr = lwdistcomm_address_get_sockaddr(addr);
    socklen_t addr_len = lwdistcomm_address_get_len(addr);

    if (!saddr || addr_len == 0) {
        return -1;
    }

    return sendto(sock, data, len, 0, saddr, addr_len);
}

/* Receive UDP data */
ssize_t lwdistcomm_transport_recvfrom(int sock, void *buffer, size_t len, lwdistcomm_address_t *addr)
{
    if (sock < 0 || !buffer || len == 0) {
        return -1;
    }

    struct sockaddr_storage saddr;
    socklen_t addr_len = sizeof(saddr);

    ssize_t ret = recvfrom(sock, buffer, len, 0, (struct sockaddr *)&saddr, &addr_len);
    if (ret < 0) {
        return ret;
    }

    if (addr) {
        // TODO: Convert sockaddr_storage to lwdistcomm_address_t
    }

    return ret;
}

/* Close socket */
void lwdistcomm_transport_close(int sock)
{
    if (sock >= 0) {
        close(sock);
    }
}

/* Set socket timeout */
bool lwdistcomm_transport_set_timeout(int sock, int timeout_ms)
{
    if (sock < 0) {
        return false;
    }

    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    // Set send timeout
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        return false;
    }

    // Set receive timeout
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        return false;
    }

    return true;
}
