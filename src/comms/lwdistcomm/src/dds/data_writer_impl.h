#ifndef LWDISTCOMM_DDS_DATA_WRITER_IMPL_H
#define LWDISTCOMM_DDS_DATA_WRITER_IMPL_H

#include "../../include/dds/dds.h"
#include "../../include/lwdistcomm.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DDS transport type declaration */
typedef struct {
    int udp_socket;
    lwdistcomm_addr_type_t addr_type;
    bool non_blocking;
} lwdistcomm_dds_transport_t;

/**
 * DataWriter内部实现结构
 */
struct lwdistcomm_dds_data_writer_impl_s {
    lwdistcomm_dds_qos_t qos;
    bool enabled;
    lwdistcomm_dds_publisher_t *publisher;
    lwdistcomm_dds_topic_t *topic;
    
    /* 已匹配的DataReader数量 */
    uint32_t matched_readers;
    
    /* 传输适配器 */
    lwdistcomm_dds_transport_t *transport;
    
    /* 互斥锁 */
    pthread_mutex_t mutex;
};

/**
 * 创建DataWriter内部实现
 */
lwdistcomm_dds_data_writer_impl_t *lwdistcomm_dds_data_writer_impl_create(const lwdistcomm_dds_data_writer_options_t *options, lwdistcomm_dds_publisher_t *publisher);

/**
 * 销毁DataWriter内部实现
 */
void lwdistcomm_dds_data_writer_impl_destroy(lwdistcomm_dds_data_writer_impl_t *impl);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_DATA_WRITER_IMPL_H */
