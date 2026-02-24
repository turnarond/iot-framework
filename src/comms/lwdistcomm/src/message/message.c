/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: message.c Message parsing and construction implementation for LwDistComm.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../../include/message.h"

#define LWDISTCOMM_MSG_MAGIC    0x9
#define LWDISTCOMM_MSG_VERSION  0x1
#define LWDISTCOMM_MSG_HDR_LEN  sizeof(lwdistcomm_msg_header_t)
#define LWDISTCOMM_MSG_MAX_LEN  131072
#define LWDISTCOMM_MSG_MAX_DATA (LWDISTCOMM_MSG_MAX_LEN - LWDISTCOMM_MSG_HDR_LEN)

/* Initialize message header */
lwdistcomm_msg_header_t *lwdistcomm_msg_init_header(void *buffer, uint8_t type, uint8_t status, uint16_t seqno)
{
    lwdistcomm_msg_header_t *header = (lwdistcomm_msg_header_t *)buffer;
    
    memset(header, 0, LWDISTCOMM_MSG_HDR_LEN);
    header->magic = LWDISTCOMM_MSG_MAGIC;
    header->version = LWDISTCOMM_MSG_VERSION;
    header->type = type;
    header->status = status;
    header->seqno = htons(seqno);
    header->url_len = 0;
    header->data_len = 0;
    
    return header;
}

/* Initialize receive buffer */
void lwdistcomm_msg_init_recv(lwdistcomm_msg_recv_t *recv)
{
    if (recv) {
        memset(recv, 0, sizeof(lwdistcomm_msg_recv_t));
    }
}

/* Validate message header */
bool lwdistcomm_msg_validate_header(const lwdistcomm_msg_header_t *header, size_t *total_len)
{
    if (!header || !total_len) {
        return false;
    }
    
    if (header->magic != LWDISTCOMM_MSG_MAGIC || header->version != LWDISTCOMM_MSG_VERSION) {
        return false;
    }
    
    uint16_t url_len = ntohs(header->url_len);
    uint32_t data_len = ntohl(header->data_len);
    
    *total_len = LWDISTCOMM_MSG_HDR_LEN + url_len + data_len;
    
    if (*total_len > LWDISTCOMM_MSG_MAX_LEN) {
        return false;
    }
    
    return true;
}

/* Set message URL */
bool lwdistcomm_msg_set_url(lwdistcomm_msg_header_t *header, const char *url)
{
    if (!header || !url) {
        return false;
    }
    
    size_t url_len = strlen(url);
    if (url_len > 0xFFFF) {
        return false;
    }
    
    header->url_len = htons((uint16_t)url_len);
    memcpy((uint8_t *)header + LWDISTCOMM_MSG_HDR_LEN, url, url_len);
    
    return true;
}

/* Set message payload */
bool lwdistcomm_msg_set_payload(lwdistcomm_msg_header_t *header, const lwdistcomm_message_t *msg)
{
    if (!header) {
        return false;
    }
    
    if (!msg || !msg->data || msg->data_len == 0) {
        header->data_len = 0;
        return true;
    }
    
    if (msg->data_len > LWDISTCOMM_MSG_MAX_DATA) {
        return false;
    }
    
    uint16_t url_len = ntohs(header->url_len);
    void *data_ptr = (uint8_t *)header + LWDISTCOMM_MSG_HDR_LEN + url_len;
    
    memcpy(data_ptr, msg->data, msg->data_len);
    header->data_len = htonl((uint32_t)msg->data_len);
    
    return true;
}

/* Get message URL */
bool lwdistcomm_msg_get_url(const lwdistcomm_msg_header_t *header, char **url, size_t *url_len)
{
    if (!header || !url || !url_len) {
        return false;
    }
    
    *url_len = ntohs(header->url_len);
    if (*url_len == 0) {
        *url = NULL;
        return true;
    }
    
    *url = (char *)((uint8_t *)header + LWDISTCOMM_MSG_HDR_LEN);
    return true;
}

/* Get message payload */
bool lwdistcomm_msg_get_payload(const lwdistcomm_msg_header_t *header, lwdistcomm_message_t *msg)
{
    if (!header || !msg) {
        return false;
    }
    
    uint16_t url_len = ntohs(header->url_len);
    uint32_t data_len = ntohl(header->data_len);
    
    if (data_len == 0) {
        msg->data = NULL;
        msg->data_len = 0;
        return true;
    }
    
    msg->data = (void *)((uint8_t *)header + LWDISTCOMM_MSG_HDR_LEN + url_len);
    msg->data_len = data_len;
    
    return true;
}

/* Process input data */
bool lwdistcomm_msg_input(lwdistcomm_msg_recv_t *recv, void *buffer, size_t len, lwdistcomm_msg_input_cb_t callback, void *arg)
{
    if (!recv || !buffer || len == 0) {
        return false;
    }
    
    size_t copy_len = len;
    if (recv->cur_len + len > sizeof(recv->buffer)) {
        copy_len = sizeof(recv->buffer) - recv->cur_len;
    }
    
    memcpy(recv->buffer + recv->cur_len, buffer, copy_len);
    recv->cur_len += copy_len;
    
    while (recv->cur_len >= LWDISTCOMM_MSG_HDR_LEN) {
        lwdistcomm_msg_header_t *header = (lwdistcomm_msg_header_t *)recv->buffer;
        size_t total_len;
        
        if (!lwdistcomm_msg_validate_header(header, &total_len)) {
            return false;
        }
        
        if (recv->cur_len < total_len) {
            break;
        }
        
        if (callback) {
            if (!callback(arg, header)) {
                return false;
            }
        }
        
        size_t remaining = recv->cur_len - total_len;
        if (remaining > 0) {
            memmove(recv->buffer, recv->buffer + total_len, remaining);
        }
        recv->cur_len = remaining;
    }
    
    return true;
}

/* Create message from buffer */
lwdistcomm_message_t *lwdistcomm_message_create(void *data, size_t data_len)
{
    lwdistcomm_message_t *msg = (lwdistcomm_message_t *)malloc(sizeof(lwdistcomm_message_t));
    if (!msg) {
        return NULL;
    }
    
    if (data && data_len > 0) {
        msg->data = malloc(data_len);
        if (!msg->data) {
            free(msg);
            return NULL;
        }
        memcpy(msg->data, data, data_len);
        msg->data_len = data_len;
    } else {
        msg->data = NULL;
        msg->data_len = 0;
    }
    
    return msg;
}

/* Destroy message */
void lwdistcomm_message_destroy(lwdistcomm_message_t *msg)
{
    if (msg) {
        if (msg->data) {
            free(msg->data);
        }
        free(msg);
    }
}
