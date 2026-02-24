#ifndef LWDISTCOMM_DDS_SUBSCRIBER_IMPL_H
#define LWDISTCOMM_DDS_SUBSCRIBER_IMPL_H

#include "../../include/dds/dds.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Subscriber内部实现结构
 */
struct lwdistcomm_dds_subscriber_impl_s {
    lwdistcomm_dds_qos_t qos;
    bool enabled;
    lwdistcomm_dds_domain_participant_t *participant;
    
    /* 数据读取器列表 */
    lwdistcomm_dds_data_reader_t **data_readers;
    uint32_t num_data_readers;
    uint32_t max_data_readers;
    
    /* 互斥锁 */
    pthread_mutex_t mutex;
};

/**
 * 创建Subscriber内部实现
 */
lwdistcomm_dds_subscriber_impl_t *lwdistcomm_dds_subscriber_impl_create(const lwdistcomm_dds_subscriber_options_t *options, lwdistcomm_dds_domain_participant_t *participant);

/**
 * 销毁Subscriber内部实现
 */
void lwdistcomm_dds_subscriber_impl_destroy(lwdistcomm_dds_subscriber_impl_t *impl);

/**
 * 添加DataReader到Subscriber
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_subscriber_impl_add_data_reader(lwdistcomm_dds_subscriber_impl_t *impl, lwdistcomm_dds_data_reader_t *reader);

/**
 * 从Subscriber移除DataReader
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_subscriber_impl_remove_data_reader(lwdistcomm_dds_subscriber_impl_t *impl, lwdistcomm_dds_data_reader_t *reader);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_SUBSCRIBER_IMPL_H */
