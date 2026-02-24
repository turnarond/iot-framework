/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: message.h Message parsing and construction for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_MESSAGE_H
#define LWDISTCOMM_MESSAGE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Message types */
#define LWDISTCOMM_MSG_TYPE_SERVINFO     0x00
#define LWDISTCOMM_MSG_TYPE_RPC          0x01
#define LWDISTCOMM_MSG_TYPE_SUBSCRIBE    0x02
#define LWDISTCOMM_MSG_TYPE_UNSUBSCRIBE  0x03
#define LWDISTCOMM_MSG_TYPE_PUBLISH      0x04
#define LWDISTCOMM_MSG_TYPE_DATAGRAM     0x05
#define LWDISTCOMM_MSG_TYPE_AUTH         0x06
#define LWDISTCOMM_MSG_FLAG_REPLY        0xfc
#define LWDISTCOMM_MSG_TYPE_NOOP         0xfe
#define LWDISTCOMM_MSG_TYPE_PINGECHO     0xff

/* Status codes */
#define LWDISTCOMM_STATUS_SUCCESS        0
#define LWDISTCOMM_STATUS_PASSWORD       1
#define LWDISTCOMM_STATUS_ARGUMENTS      2
#define LWDISTCOMM_STATUS_INVALID_URL    3
#define LWDISTCOMM_STATUS_NO_RESPONDING  4
#define LWDISTCOMM_STATUS_NO_PERMISSIONS 5
#define LWDISTCOMM_STATUS_NO_MEMORY      6
#define LWDISTCOMM_STATUS_AUTH_FAILED    7

/* Message header */
typedef struct {
    uint8_t magic;          // Magic number
    uint8_t version;        // Protocol version
    uint8_t type;           // Message type
    uint8_t status;         // Status code
    uint16_t url_len;       // URL length
    uint16_t seqno;         // Sequence number
    uint32_t data_len;      // Data length
} lwdistcomm_msg_header_t;

/* Message receive buffer */
typedef struct {
    uint32_t cur_len;
    uint32_t total_len;
    uint8_t buffer[131072];  // 128KB max packet size
} lwdistcomm_msg_recv_t;

/* Initialize message header */
lwdistcomm_msg_header_t *lwdistcomm_msg_init_header(void *buffer, uint8_t type, uint8_t status, uint16_t seqno);

/* Initialize receive buffer */
void lwdistcomm_msg_init_recv(lwdistcomm_msg_recv_t *recv);

/* Validate message header */
bool lwdistcomm_msg_validate_header(const lwdistcomm_msg_header_t *header, size_t *total_len);

/* Set message URL */
bool lwdistcomm_msg_set_url(lwdistcomm_msg_header_t *header, const char *url);

/* Set message payload */
bool lwdistcomm_msg_set_payload(lwdistcomm_msg_header_t *header, const lwdistcomm_message_t *msg);

/* Get message URL */
bool lwdistcomm_msg_get_url(const lwdistcomm_msg_header_t *header, char **url, size_t *url_len);

/* Get message payload */
bool lwdistcomm_msg_get_payload(const lwdistcomm_msg_header_t *header, lwdistcomm_message_t *msg);

/* Process input data */
typedef bool (*lwdistcomm_msg_input_cb_t)(void *arg, lwdistcomm_msg_header_t *header);
bool lwdistcomm_msg_input(lwdistcomm_msg_recv_t *recv, void *buffer, size_t len, lwdistcomm_msg_input_cb_t callback, void *arg);

/* Create message from buffer */
lwdistcomm_message_t *lwdistcomm_message_create(void *data, size_t data_len);

/* Destroy message */
void lwdistcomm_message_destroy(lwdistcomm_message_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_MESSAGE_H */
