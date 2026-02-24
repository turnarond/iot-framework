#include "publisher_impl.h"
#include "domain_participant_impl.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_DATA_WRITERS 32

/**
 * 创建Publisher内部实现
 */
lwdistcomm_dds_publisher_impl_t *lwdistcomm_dds_publisher_impl_create(const lwdistcomm_dds_publisher_options_t *options, lwdistcomm_dds_domain_participant_t *participant)
{
    lwdistcomm_dds_publisher_impl_t *impl = (lwdistcomm_dds_publisher_impl_t *)malloc(sizeof(lwdistcomm_dds_publisher_impl_t));
    if (!impl) {
        return NULL;
    }
    
    memset(impl, 0, sizeof(lwdistcomm_dds_publisher_impl_t));
    
    /* 复制QoS */
    memcpy(&impl->qos, &options->qos, sizeof(lwdistcomm_dds_qos_t));
    
    impl->enabled = false;
    impl->participant = participant;
    
    /* 初始化数据写入器列表 */
    impl->data_writers = (lwdistcomm_dds_data_writer_t **)malloc(sizeof(lwdistcomm_dds_data_writer_t *) * MAX_DATA_WRITERS);
    impl->max_data_writers = MAX_DATA_WRITERS;
    
    /* 初始化互斥锁 */
    pthread_mutex_init(&impl->mutex, NULL);
    
    return impl;
}

/**
 * 销毁Publisher内部实现
 */
void lwdistcomm_dds_publisher_impl_destroy(lwdistcomm_dds_publisher_impl_t *impl)
{
    if (!impl) {
        return;
    }
    
    /* 释放数据写入器列表 */
    free(impl->data_writers);
    
    /* 销毁互斥锁 */
    pthread_mutex_destroy(&impl->mutex);
    
    free(impl);
}

/**
 * 添加DataWriter到Publisher
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_publisher_impl_add_data_writer(lwdistcomm_dds_publisher_impl_t *impl, lwdistcomm_dds_data_writer_t *writer)
{
    if (!impl || !writer) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    if (impl->num_data_writers >= impl->max_data_writers) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES;
    }
    
    impl->data_writers[impl->num_data_writers++] = writer;
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 从Publisher移除DataWriter
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_publisher_impl_remove_data_writer(lwdistcomm_dds_publisher_impl_t *impl, lwdistcomm_dds_data_writer_t *writer)
{
    if (!impl || !writer) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    for (uint32_t i = 0; i < impl->num_data_writers; i++) {
        if (impl->data_writers[i] == writer) {
            /* 移除数据写入器 */
            impl->num_data_writers--;
            if (i < impl->num_data_writers) {
                impl->data_writers[i] = impl->data_writers[impl->num_data_writers];
            }
            
            pthread_mutex_unlock(&impl->mutex);
            return LWDISTCOMM_DDS_RETCODE_OK;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 创建Publisher
 */
lwdistcomm_dds_publisher_t *lwdistcomm_dds_publisher_create(lwdistcomm_dds_domain_participant_t *participant, const lwdistcomm_dds_publisher_options_t *options)
{
    if (!participant || !options) {
        return NULL;
    }
    
    lwdistcomm_dds_publisher_t *publisher = (lwdistcomm_dds_publisher_t *)malloc(sizeof(lwdistcomm_dds_publisher_t));
    if (!publisher) {
        return NULL;
    }
    
    /* 初始化基础实体 */
    publisher->base.kind = LWDISTCOMM_DDS_ENTITY_PUBLISHER;
    publisher->base.status_mask = LWDISTCOMM_DDS_STATUS_MASK_ALL;
    
    /* 创建内部实现 */
    publisher->impl = lwdistcomm_dds_publisher_impl_create(options, participant);
    if (!publisher->impl) {
        free(publisher);
        return NULL;
    }
    
    publisher->base.impl = publisher->impl;
    
    /* 将发布者添加到DomainParticipant */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_impl_add_publisher(participant->impl, publisher);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        lwdistcomm_dds_publisher_impl_destroy(publisher->impl);
        free(publisher);
        return NULL;
    }
    
    /* 启用发布者 */
    publisher->impl->enabled = true;
    
    return publisher;
}

/**
 * 删除Publisher
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_publisher_delete(lwdistcomm_dds_publisher_t *publisher)
{
    if (!publisher || !publisher->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    lwdistcomm_dds_domain_participant_t *participant = publisher->impl->participant;
    
    /* 禁用发布者 */
    publisher->impl->enabled = false;
    
    /* 从DomainParticipant移除发布者 */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_impl_remove_publisher(participant->impl, publisher);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        return ret;
    }
    
    /* 销毁内部实现 */
    lwdistcomm_dds_publisher_impl_destroy(publisher->impl);
    
    /* 释放内存 */
    free(publisher);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
