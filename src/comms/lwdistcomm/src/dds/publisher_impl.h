#ifndef LWDISTCOMM_DDS_PUBLISHER_IMPL_H
#define LWDISTCOMM_DDS_PUBLISHER_IMPL_H

#include "../../include/dds/dds.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Publisher内部实现结构
 */
struct lwdistcomm_dds_publisher_impl_s {
    lwdistcomm_dds_qos_t qos;
    bool enabled;
    lwdistcomm_dds_domain_participant_t *participant;
    
    /* 数据写入器列表 */
    lwdistcomm_dds_data_writer_t **data_writers;
    uint32_t num_data_writers;
    uint32_t max_data_writers;
    
    /* 互斥锁 */
    pthread_mutex_t mutex;
};

/**
 * 创建Publisher内部实现
 */
lwdistcomm_dds_publisher_impl_t *lwdistcomm_dds_publisher_impl_create(const lwdistcomm_dds_publisher_options_t *options, lwdistcomm_dds_domain_participant_t *participant);

/**
 * 销毁Publisher内部实现
 */
void lwdistcomm_dds_publisher_impl_destroy(lwdistcomm_dds_publisher_impl_t *impl);

/**
 * 添加DataWriter到Publisher
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_publisher_impl_add_data_writer(lwdistcomm_dds_publisher_impl_t *impl, lwdistcomm_dds_data_writer_t *writer);

/**
 * 从Publisher移除DataWriter
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_publisher_impl_remove_data_writer(lwdistcomm_dds_publisher_impl_t *impl, lwdistcomm_dds_data_writer_t *writer);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_PUBLISHER_IMPL_H */
