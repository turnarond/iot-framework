#include "subscriber_impl.h"
#include "domain_participant_impl.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_DATA_READERS 32

/**
 * 创建Subscriber内部实现
 */
lwdistcomm_dds_subscriber_impl_t *lwdistcomm_dds_subscriber_impl_create(const lwdistcomm_dds_subscriber_options_t *options, lwdistcomm_dds_domain_participant_t *participant)
{
    lwdistcomm_dds_subscriber_impl_t *impl = (lwdistcomm_dds_subscriber_impl_t *)malloc(sizeof(lwdistcomm_dds_subscriber_impl_t));
    if (!impl) {
        return NULL;
    }
    
    memset(impl, 0, sizeof(lwdistcomm_dds_subscriber_impl_t));
    
    /* 复制QoS */
    memcpy(&impl->qos, &options->qos, sizeof(lwdistcomm_dds_qos_t));
    
    impl->enabled = false;
    impl->participant = participant;
    
    /* 初始化数据读取器列表 */
    impl->data_readers = (lwdistcomm_dds_data_reader_t **)malloc(sizeof(lwdistcomm_dds_data_reader_t *) * MAX_DATA_READERS);
    impl->max_data_readers = MAX_DATA_READERS;
    
    /* 初始化互斥锁 */
    pthread_mutex_init(&impl->mutex, NULL);
    
    return impl;
}

/**
 * 销毁Subscriber内部实现
 */
void lwdistcomm_dds_subscriber_impl_destroy(lwdistcomm_dds_subscriber_impl_t *impl)
{
    if (!impl) {
        return;
    }
    
    /* 释放数据读取器列表 */
    free(impl->data_readers);
    
    /* 销毁互斥锁 */
    pthread_mutex_destroy(&impl->mutex);
    
    free(impl);
}

/**
 * 添加DataReader到Subscriber
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_subscriber_impl_add_data_reader(lwdistcomm_dds_subscriber_impl_t *impl, lwdistcomm_dds_data_reader_t *reader)
{
    if (!impl || !reader) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    if (impl->num_data_readers >= impl->max_data_readers) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES;
    }
    
    impl->data_readers[impl->num_data_readers++] = reader;
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 从Subscriber移除DataReader
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_subscriber_impl_remove_data_reader(lwdistcomm_dds_subscriber_impl_t *impl, lwdistcomm_dds_data_reader_t *reader)
{
    if (!impl || !reader) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    for (uint32_t i = 0; i < impl->num_data_readers; i++) {
        if (impl->data_readers[i] == reader) {
            /* 移除数据读取器 */
            impl->num_data_readers--;
            if (i < impl->num_data_readers) {
                impl->data_readers[i] = impl->data_readers[impl->num_data_readers];
            }
            
            pthread_mutex_unlock(&impl->mutex);
            return LWDISTCOMM_DDS_RETCODE_OK;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 创建Subscriber
 */
lwdistcomm_dds_subscriber_t *lwdistcomm_dds_subscriber_create(lwdistcomm_dds_domain_participant_t *participant, const lwdistcomm_dds_subscriber_options_t *options)
{
    if (!participant || !options) {
        return NULL;
    }
    
    lwdistcomm_dds_subscriber_t *subscriber = (lwdistcomm_dds_subscriber_t *)malloc(sizeof(lwdistcomm_dds_subscriber_t));
    if (!subscriber) {
        return NULL;
    }
    
    /* 初始化基础实体 */
    subscriber->base.kind = LWDISTCOMM_DDS_ENTITY_SUBSCRIBER;
    subscriber->base.status_mask = LWDISTCOMM_DDS_STATUS_MASK_ALL;
    
    /* 创建内部实现 */
    subscriber->impl = lwdistcomm_dds_subscriber_impl_create(options, participant);
    if (!subscriber->impl) {
        free(subscriber);
        return NULL;
    }
    
    subscriber->base.impl = subscriber->impl;
    
    /* 将订阅者添加到DomainParticipant */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_impl_add_subscriber(participant->impl, subscriber);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        lwdistcomm_dds_subscriber_impl_destroy(subscriber->impl);
        free(subscriber);
        return NULL;
    }
    
    /* 启用订阅者 */
    subscriber->impl->enabled = true;
    
    return subscriber;
}

/**
 * 删除Subscriber
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_subscriber_delete(lwdistcomm_dds_subscriber_t *subscriber)
{
    if (!subscriber || !subscriber->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    lwdistcomm_dds_domain_participant_t *participant = subscriber->impl->participant;
    
    /* 禁用订阅者 */
    subscriber->impl->enabled = false;
    
    /* 从DomainParticipant移除订阅者 */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_impl_remove_subscriber(participant->impl, subscriber);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        return ret;
    }
    
    /* 销毁内部实现 */
    lwdistcomm_dds_subscriber_impl_destroy(subscriber->impl);
    
    /* 释放内存 */
    free(subscriber);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
