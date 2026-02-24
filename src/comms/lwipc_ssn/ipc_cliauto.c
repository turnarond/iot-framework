/*
* Copyright (c) 2023 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: vsoa_cliauto.c Vehicle SOA client auto robot.
*
* Author: Han.hui <hanhui@acoinfo.com>
*
*/

#include <stdlib.h>
#include <string.h>
#include "ipc_cliauto.h"
#include "ipc_platform.h"

/* Timer convert ms to timespec */
#define IPC_CLIAUTO_MS_TO_TS(ts, ms) \
{ \
    (ts)->tv_sec  = ((ms) / 1000); \
    (ts)->tv_nsec = ((ms) % 1000) * 1000000; \
}

/* URL list */
typedef struct ipc_urls {
    char **urls;
    int url_cnt;
} ipc_urls_t;

/* Structure client auto */
struct ipc_client_auto {
    bool valid;
    bool start;
    ipc_client_t *client;
    vsoa_thread_t eloop;
    vsoa_mutex_t lock;
    char *hostname;
    ipc_urls_t subs;
    ipc_urls_t pubs;
    unsigned int keepalive;
    unsigned int ping_to;
    unsigned int conn_timeout;
    unsigned int reconn_delay;
    unsigned int rpc_timeout;
    ipc_client_conn_func_t onconn;
    void *onconn_arg;
    struct sockaddr_storage serv_addr;
};

/*
* URLs new
*/
static bool ipc_client_auto_urls_new (ipc_urls_t *vurls, char *const urls[], int url_cnt)
{
    int i = -1;

    vurls->urls = (char **)malloc(sizeof(char *) * url_cnt);
    if (!vurls->urls) {
        goto    error;
    }
    for (i = 0; i < url_cnt; i++) {
        vurls->urls[i] = strdup(urls[i]);
        if (!vurls->urls[i]) {
            break;
        }
    }
    if (i < url_cnt) {
        for (--i; i >= 0; i--) {
            free(vurls->urls[i]);
        }
        free(vurls->urls);
        goto    error;
    }

    vurls->url_cnt = url_cnt;
    return  (true);

error:
    for (; i >= 0; i--) {
        if (vurls->urls[i]) {
            free(vurls->urls[i]);
        }
    }
    if (vurls->urls) {
        free(vurls->urls);
        vurls->urls = NULL;
    }
    return  (false);
}

/*
* URLs free
*/
static void ipc_client_auto_urls_free (ipc_urls_t *vurls)
{
    int i;

    if (vurls->url_cnt) {
        for (i = 0; i < vurls->url_cnt; i++) {
            free(vurls->urls[i]);
        }

        free(vurls->urls);
        vurls->urls = NULL;
        vurls->url_cnt = 0;
    }
}

/*
* VSOA client auto event loop thread
*/
static void *ipc_client_auto_loop (void *arg)
{
    int max_fd, cnt;
    fd_set rfds;
    char *info = NULL;
    ipc_client_auto_t *cliauto = (ipc_client_auto_t *)arg;
    socklen_t addr_len;
    int64_t ka_period = cliauto->keepalive * 1000000;
    struct timespec timeout, turbo, keepalive, ping_to;
    struct timespec *select_to = &keepalive;

    IPC_CLIAUTO_MS_TO_TS(&timeout,   cliauto->conn_timeout);
    IPC_CLIAUTO_MS_TO_TS(&keepalive, cliauto->keepalive);
    IPC_CLIAUTO_MS_TO_TS(&ping_to,   cliauto->ping_to);

    while (cliauto->start) {
        addr_len = sizeof(struct sockaddr_storage);

        if (!ipc_client_connect(cliauto->client, cliauto->hostname, &timeout)) {
            goto    retry;
        }

        if (cliauto->onconn) {
            cliauto->onconn(cliauto->onconn_arg, cliauto, true);
        }

        while (cliauto->start) {
            if (!ipc_client_is_connect(cliauto->client)) {
                break;
            }

            FD_ZERO(&rfds);
            max_fd = ipc_client_fds(cliauto->client, &rfds);
            if (max_fd < 0) {
                break;
            }

            cnt = vsoa_select(max_fd + 1, &rfds, NULL, NULL, select_to);
            if (cnt > 0) {
                if (!ipc_client_process_events(cliauto->client, &rfds)) {
                    break;
                }
            } else if (cnt < 0) {
                break;

            }
        }

        if (ipc_client_is_connect(cliauto->client)) {
            ipc_client_disconnect(cliauto->client);
        }

        if (cliauto->onconn) {
            cliauto->onconn(cliauto->onconn_arg, cliauto, false);
        }

retry:
        cliauto->serv_addr.ss_family = AF_UNSPEC;
        if (info) {
            free(info);
            info = NULL;
        }

        vsoa_thread_msleep(cliauto->reconn_delay);
    }

    vsoa_thread_exit();

    return  (NULL);
}

/* 
* VSOA client auto create 
* The callback function will be called in the client auto thread context
*/
ipc_client_auto_t *ipc_client_auto_create (ipc_client_msg_func_t onmsg, void *arg)
{
    int err = 0;
    ipc_client_auto_t *cliauto;

    cliauto = (ipc_client_auto_t *)malloc(sizeof(ipc_client_auto_t));
    if (!cliauto) {
        return  (NULL);
    }

    bzero(cliauto, sizeof(ipc_client_auto_t));

    if (vsoa_mutex_init(&cliauto->lock)) {
        goto    error;
    }

    cliauto->client = ipc_client_create(onmsg, arg);
    if (!cliauto->client) {
        err = 1;
        goto    error;
    }

    cliauto->valid = true;

    return  (cliauto);

error:
    if (err > 0) {
        vsoa_mutex_destroy(&cliauto->lock);
    }


    free(cliauto);
    return  (NULL);
}

/*
* VSOA client auto delete
* After this function is called, the `cliauto` object becomes invalid and is not allowed to be used again
*/
void ipc_client_auto_delete (ipc_client_auto_t *cliauto)
{
    if (!cliauto || !cliauto->valid) {
        return;
    }

    vsoa_mutex_lock(&cliauto->lock);

    cliauto->valid = false;
    vsoa_memory_barrier();

    if (cliauto->start) {
        cliauto->start = false;
        vsoa_thread_wait(&cliauto->eloop);
    }

    vsoa_mutex_unlock(&cliauto->lock);

    if (cliauto->client) {
        ipc_client_close(cliauto->client);
        cliauto->client = NULL;
    }

    if (cliauto->hostname) {
        free(cliauto->hostname);
    }

    if (cliauto->subs.url_cnt) {
        ipc_client_auto_urls_free(&cliauto->subs);
    }

    if (cliauto->pubs.url_cnt) {
        ipc_client_auto_urls_free(&cliauto->pubs);
    }

    vsoa_mutex_destroy(&cliauto->lock);
    free(cliauto);
}

/*
* VSOA client auto setup
* The callback function will be called in the client auto thread context
*/
bool ipc_client_auto_setup (ipc_client_auto_t *cliauto, ipc_client_conn_func_t onconn, void *arg)
{
    if (!cliauto || !cliauto->valid) {
        return  (false);
    }

    cliauto->onconn     = onconn;
    cliauto->onconn_arg = arg;

    return  (true);
}

/*
* VSOA client auto consistent
*/
bool ipc_client_auto_consistent (ipc_client_auto_t *cliauto, char *const urls[], int url_cnt, unsigned int rpc_timeout)
{
    if (!cliauto || !cliauto->valid || cliauto->start || !urls || url_cnt < 1 || rpc_timeout < 20) {
        return  (false);
    }

    vsoa_mutex_lock(&cliauto->lock);

    if (cliauto->pubs.url_cnt) {
        ipc_client_auto_urls_free(&cliauto->pubs);
    }

    if (!ipc_client_auto_urls_new(&cliauto->pubs, urls, url_cnt)) {
        vsoa_mutex_unlock(&cliauto->lock);
        return  (false);
    }

    cliauto->rpc_timeout = rpc_timeout;

    vsoa_mutex_unlock(&cliauto->lock);

    return  (true);
}

/*
* VSOA client ping turbo (experimental)
*/
bool ipc_client_auto_ping_turbo (ipc_client_auto_t *cliauto, unsigned int turbo, unsigned int max_cnt)
{
    if (!cliauto || !cliauto->valid || cliauto->start) {
        return  (false);
    }
    if ((turbo && turbo < 25) || max_cnt < 3) {
        return  (false);
    }

    vsoa_memory_barrier();

    return  (true);
}

/* 
* VSOA client auto start
* `server` can be ip:port or service hostname, client auto will automatically use position to query hostname address.
* `urls` and `cnt`: URLs to subscribe to when the link is established.
* `keepalive`: How often (millisecond) to perform a ping operation, the minimum is 50ms, and the ping timeout is same as this value.
* `conn_timeout`: Connection timeout, unit: millisecond, the minimum is 20ms.
* `reconn_delay`: When the connection times out or the password is incorrect or the network is disconnected, 
*                 the retry waiting time, unit: millisecond, the minimum is 20ms.
*/
bool ipc_client_auto_start (ipc_client_auto_t *cliauto, const char *server, 
                            char *const urls[], int url_cnt, 
                            unsigned int keepalive, unsigned int conn_timeout, unsigned int reconn_delay)
{
    bool ret = false;

    if (!cliauto || !cliauto->valid) {
        return  (false);
    }
    if (!server || (url_cnt > 0 && !urls)) {
        return  (false);
    }
    if (keepalive < 50 || conn_timeout < 20 || reconn_delay < 20) {
        return  (false);
    }

    vsoa_mutex_lock(&cliauto->lock);

    if (cliauto->start) {
        goto    out;
    }

    if (cliauto->hostname) {
        free(cliauto->hostname);
    }
    cliauto->hostname = strdup(server);
    if (!cliauto->hostname) {
        goto    out;
    }

    if (cliauto->subs.url_cnt) {
        ipc_client_auto_urls_free(&cliauto->subs);
    }

    if (url_cnt > 0) {
        if (!ipc_client_auto_urls_new(&cliauto->subs, urls, url_cnt)) {
            goto    out;
        }
    }

    cliauto->keepalive    = keepalive;
    cliauto->ping_to      = keepalive;
    cliauto->conn_timeout = conn_timeout;
    cliauto->reconn_delay = reconn_delay;

    cliauto->start = true;
    if (vsoa_thread_create(&cliauto->eloop, ipc_client_auto_loop, cliauto)) {
        cliauto->start = false;
        goto    out;
    }

    ret = true;

out:
    vsoa_mutex_unlock(&cliauto->lock);

    return  (ret);
}

/*
* VSOA client auto stop
* `ipc_client_auto_start` and `ipc_client_auto_stop` must be called sequentially
*/
bool ipc_client_auto_stop (ipc_client_auto_t *cliauto)
{
    if (!cliauto || !cliauto->valid) {
        return  (false);
    }

    vsoa_mutex_lock(&cliauto->lock);

    if (!cliauto->start) {
        vsoa_mutex_unlock(&cliauto->lock);
        return  (false);
    }

    cliauto->start = false;
    vsoa_thread_wait(&cliauto->eloop);
    ipc_client_disconnect(cliauto->client);

    vsoa_mutex_unlock(&cliauto->lock);

    return  (true);
}

/*
* VSOA client auto get client handle
* This handle is only used for communication,
* and cannot perform state operations such as connection closing,
* otherwise it will destroy the client auto logic
*/
ipc_client_t *ipc_client_auto_handle (ipc_client_auto_t *cliauto)
{
    return  ((cliauto && cliauto->valid) ? cliauto->client : NULL);
}

/*
* VSOA client auto get server address
* This function is recommended to be called in connect callback.
*/
bool ipc_client_auto_server_address (ipc_client_auto_t *cliauto, struct sockaddr *addr, socklen_t *namelen)
{
    if (!cliauto || !cliauto->valid || !addr || !namelen) {
        return  (false);
    }

    if (cliauto->serv_addr.ss_family == AF_INET) {
        if (*namelen < sizeof(struct sockaddr_in)) {
            return  (false);
        } else {
            *namelen = sizeof(struct sockaddr_in);
            memcpy(addr, &cliauto->serv_addr, sizeof(struct sockaddr_in));
            return  (true);
        }

    } else if (cliauto->serv_addr.ss_family == AF_INET6) {
        if (*namelen < sizeof(struct sockaddr_in6)) {
            return  (false);
        } else {
            *namelen = sizeof(struct sockaddr_in6);
            memcpy(addr, &cliauto->serv_addr, sizeof(struct sockaddr_in6));
            return  (true);
        }

    } else {
        return  (false);
    }
}
/*
* end
*/
