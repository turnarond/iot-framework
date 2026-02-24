#ifndef LWDISTCOMM_DDS_DATA_READER_IMPL_H
#define LWDISTCOMM_DDS_DATA_READER_IMPL_H

#include "../../include/dds/dds.h"
#include "../../include/lwdistcomm.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 数据样本结构
 */
typedef struct {
    void *data;
    uint32_t size;
    lwdistcomm_dds_sample_info_t info;
} lwdistcomm_dds_sample_t;

/**
 * DataReader内部实现结构
 */
struct lwdistcomm_dds_data_reader_impl_s {
    lwdistcomm_dds_qos_t qos;
    bool enabled;
    lwdistcomm_dds_subscriber_t *subscriber;
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_data_reader_t *reader;
    
    /* 已匹配的DataWriter数量 */
    uint32_t matched_writers;
    
    /* 数据可用回调 */
    lwdistcomm_dds_data_available_cb_t data_available_cb;
    void *data_available_arg;
    
    /* 数据样本队列 */
    lwdistcomm_dds_sample_t *samples;
    uint32_t num_samples;
    uint32_t max_samples;
    uint32_t next_sample_index;
    
    /* 互斥锁 */
    pthread_mutex_t mutex;
    
    /* 网络接收 */
    int udp_socket;
    pthread_t receive_thread;
    bool receive_thread_running;
};

/**
 * 创建DataReader内部实现
 */
lwdistcomm_dds_data_reader_impl_t *lwdistcomm_dds_data_reader_impl_create(const lwdistcomm_dds_data_reader_options_t *options, lwdistcomm_dds_subscriber_t *subscriber);

/**
 * 销毁DataReader内部实现
 */
void lwdistcomm_dds_data_reader_impl_destroy(lwdistcomm_dds_data_reader_impl_t *impl);

/**
 * 添加数据样本到DataReader
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_impl_add_sample(lwdistcomm_dds_data_reader_impl_t *impl, const void *data, uint32_t size, const lwdistcomm_dds_sample_info_t *info);

/**
 * 数据接收线程函数
 */
void *lwdistcomm_dds_data_reader_receive_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_DATA_READER_IMPL_H */
