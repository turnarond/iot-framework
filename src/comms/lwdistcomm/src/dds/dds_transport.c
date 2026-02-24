#include "../../include/dds/dds.h"
#include "../../include/address.h"
#include "../../include/lwdistcomm.h"
#include <stdlib.h>
#include <string.h>

/* Transport functions declarations */
extern int lwdistcomm_transport_create_udp_socket(lwdistcomm_addr_type_t type, bool non_blocking);
extern ssize_t lwdistcomm_transport_sendto(int sock, const void *data, size_t len, const lwdistcomm_address_t *addr);
extern ssize_t lwdistcomm_transport_recvfrom(int sock, void *buffer, size_t len, lwdistcomm_address_t *addr);
extern bool lwdistcomm_transport_bind(int sock, const lwdistcomm_address_t *addr);
extern bool lwdistcomm_transport_set_timeout(int sock, int timeout_ms);
extern void lwdistcomm_transport_close(int sock);

/**
 * DDS传输适配器结构
 */
typedef struct {
    int udp_socket;
    lwdistcomm_addr_type_t addr_type;
    bool non_blocking;
} lwdistcomm_dds_transport_t;

/**
 * 创建DDS传输适配器
 */
lwdistcomm_dds_transport_t *lwdistcomm_dds_transport_create(lwdistcomm_addr_type_t type, bool non_blocking)
{
    lwdistcomm_dds_transport_t *transport = (lwdistcomm_dds_transport_t *)malloc(sizeof(lwdistcomm_dds_transport_t));
    if (!transport) {
        return NULL;
    }
    
    /* 创建UDP套接字 */
    transport->udp_socket = lwdistcomm_transport_create_udp_socket(type, non_blocking);
    if (transport->udp_socket < 0) {
        free(transport);
        return NULL;
    }
    
    transport->addr_type = type;
    transport->non_blocking = non_blocking;
    
    return transport;
}

/**
 * 销毁DDS传输适配器
 */
void lwdistcomm_dds_transport_destroy(lwdistcomm_dds_transport_t *transport)
{
    if (!transport) {
        return;
    }
    
    if (transport->udp_socket >= 0) {
        lwdistcomm_transport_close(transport->udp_socket);
    }
    
    free(transport);
}

/**
 * 发送DDS数据
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_transport_send(lwdistcomm_dds_transport_t *transport, const void *data, uint32_t size, const lwdistcomm_address_t *addr)
{
    if (!transport || !data || size == 0 || !addr) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    ssize_t sent = lwdistcomm_transport_sendto(transport->udp_socket, data, size, addr);
    if (sent < 0 || (uint32_t)sent != size) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 接收DDS数据
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_transport_recv(lwdistcomm_dds_transport_t *transport, void *buffer, uint32_t *size, lwdistcomm_address_t *addr)
{
    if (!transport || !buffer || !size) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    ssize_t received = lwdistcomm_transport_recvfrom(transport->udp_socket, buffer, *size, addr);
    if (received < 0) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    *size = (uint32_t)received;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 绑定DDS传输适配器到地址
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_transport_bind(lwdistcomm_dds_transport_t *transport, const lwdistcomm_address_t *addr)
{
    if (!transport || !addr) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    bool bound = lwdistcomm_transport_bind(transport->udp_socket, addr);
    if (!bound) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置DDS传输适配器的超时
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_transport_set_timeout(lwdistcomm_dds_transport_t *transport, int timeout_ms)
{
    if (!transport) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    bool success = lwdistcomm_transport_set_timeout(transport->udp_socket, timeout_ms);
    if (!success) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
