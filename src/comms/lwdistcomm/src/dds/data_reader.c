#include "data_reader_impl.h"
#include "subscriber_impl.h"
#include "topic_impl.h"
#include "../../include/dds/dds.h"
#include "../../include/lwdistcomm.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_SAMPLES 128
#define UDP_RECEIVE_PORT 7402
#define MAX_BUFFER_SIZE 1024

/* Transport functions declarations */
extern int lwdistcomm_transport_create_udp_socket(lwdistcomm_addr_type_t type, bool non_blocking);
extern bool lwdistcomm_transport_bind(int sock, const lwdistcomm_address_t *addr);
extern ssize_t lwdistcomm_transport_recvfrom(int sock, void *buffer, size_t len, lwdistcomm_address_t *addr);
extern void lwdistcomm_transport_close(int sock);
extern lwdistcomm_address_t *lwdistcomm_address_create(lwdistcomm_addr_type_t type);
extern bool lwdistcomm_address_set_ipv4(lwdistcomm_address_t *addr, const char *ip, uint16_t port);
extern void lwdistcomm_address_destroy(lwdistcomm_address_t *addr);

/**
 * 数据接收线程函数
 */
void *lwdistcomm_dds_data_reader_receive_thread(void *arg)
{
    lwdistcomm_dds_data_reader_impl_t *impl = (lwdistcomm_dds_data_reader_impl_t *)arg;
    if (!impl) {
        return NULL;
    }
    
    char buffer[MAX_BUFFER_SIZE];
    lwdistcomm_address_t *addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
    if (!addr) {
        return NULL;
    }
    
    while (impl->receive_thread_running) {
        ssize_t received = lwdistcomm_transport_recvfrom(impl->udp_socket, buffer, MAX_BUFFER_SIZE, addr);
        if (received > 0) {
            printf("DataReader: Received %zd bytes of data\n", received);
            /* 创建样本信息 */
            lwdistcomm_dds_sample_info_t info;
            memset(&info, 0, sizeof(lwdistcomm_dds_sample_info_t));
            info.valid_data = true;
            
            /* 添加数据样本 */
            lwdistcomm_dds_data_reader_impl_add_sample(impl, buffer, (uint32_t)received, &info);
        }
    }
    
    lwdistcomm_address_destroy(addr);
    return NULL;
}

/**
 * 创建DataReader内部实现
 */
lwdistcomm_dds_data_reader_impl_t *lwdistcomm_dds_data_reader_impl_create(const lwdistcomm_dds_data_reader_options_t *options, lwdistcomm_dds_subscriber_t *subscriber)
{
    lwdistcomm_dds_data_reader_impl_t *impl = (lwdistcomm_dds_data_reader_impl_t *)malloc(sizeof(lwdistcomm_dds_data_reader_impl_t));
    if (!impl) {
        return NULL;
    }
    
    memset(impl, 0, sizeof(lwdistcomm_dds_data_reader_impl_t));
    
    /* 复制QoS */
    memcpy(&impl->qos, &options->qos, sizeof(lwdistcomm_dds_qos_t));
    
    impl->enabled = false;
    impl->subscriber = subscriber;
    impl->topic = options->topic;
    impl->matched_writers = 0;
    impl->data_available_cb = NULL;
    impl->data_available_arg = NULL;
    
    /* 初始化数据样本队列 */
    impl->samples = (lwdistcomm_dds_sample_t *)malloc(sizeof(lwdistcomm_dds_sample_t) * MAX_SAMPLES);
    impl->max_samples = MAX_SAMPLES;
    impl->num_samples = 0;
    impl->next_sample_index = 0;
    
    /* 初始化互斥锁 */
    pthread_mutex_init(&impl->mutex, NULL);
    
    /* 创建UDP套接字 */
    impl->udp_socket = lwdistcomm_transport_create_udp_socket(LWDISTCOMM_ADDR_TYPE_IPV4, true);
    if (impl->udp_socket >= 0) {
        /* 创建并设置地址 */
        lwdistcomm_address_t *addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
        if (addr) {
            if (lwdistcomm_address_set_ipv4(addr, "0.0.0.0", UDP_RECEIVE_PORT)) {
                /* 绑定套接字 */
                if (lwdistcomm_transport_bind(impl->udp_socket, addr)) {
                    /* 启动接收线程 */
                    impl->receive_thread_running = true;
                    pthread_create(&impl->receive_thread, NULL, lwdistcomm_dds_data_reader_receive_thread, impl);
                    printf("DataReader: UDP socket bound to port %d, receive thread started\n", UDP_RECEIVE_PORT);
                } else {
                    printf("DataReader: Failed to bind UDP socket to port %d\n", UDP_RECEIVE_PORT);
                }
            } else {
                printf("DataReader: Failed to set IPv4 address\n");
            }
            lwdistcomm_address_destroy(addr);
        } else {
            printf("DataReader: Failed to create address\n");
        }
    } else {
        printf("DataReader: Failed to create UDP socket\n");
    }
    
    return impl;
}

/**
 * 销毁DataReader内部实现
 */
void lwdistcomm_dds_data_reader_impl_destroy(lwdistcomm_dds_data_reader_impl_t *impl)
{
    if (!impl) {
        return;
    }
    
    /* 停止接收线程 */
    if (impl->receive_thread_running) {
        impl->receive_thread_running = false;
        pthread_join(impl->receive_thread, NULL);
    }
    
    /* 关闭UDP套接字 */
    if (impl->udp_socket >= 0) {
        lwdistcomm_transport_close(impl->udp_socket);
    }
    
    /* 释放数据样本 */
    for (uint32_t i = 0; i < impl->num_samples; i++) {
        free(impl->samples[i].data);
    }
    free(impl->samples);
    
    /* 销毁互斥锁 */
    pthread_mutex_destroy(&impl->mutex);
    
    free(impl);
}

/**
 * 添加数据样本到DataReader
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_impl_add_sample(lwdistcomm_dds_data_reader_impl_t *impl, const void *data, uint32_t size, const lwdistcomm_dds_sample_info_t *info)
{
    if (!impl || !data || size == 0) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 检查样本队列是否已满 */
    if (impl->num_samples >= impl->max_samples) {
        /* 如果队列已满，移除最早的样本 */
        free(impl->samples[impl->next_sample_index].data);
        impl->num_samples--;
    } else {
        /* 否则，使用下一个可用位置 */
        impl->next_sample_index = impl->num_samples;
    }
    
    /* 添加新样本 */
    impl->samples[impl->next_sample_index].data = malloc(size);
    if (!impl->samples[impl->next_sample_index].data) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES;
    }
    
    memcpy(impl->samples[impl->next_sample_index].data, data, size);
    impl->samples[impl->next_sample_index].size = size;
    
    if (info) {
        memcpy(&impl->samples[impl->next_sample_index].info, info, sizeof(lwdistcomm_dds_sample_info_t));
    } else {
        memset(&impl->samples[impl->next_sample_index].info, 0, sizeof(lwdistcomm_dds_sample_info_t));
        impl->samples[impl->next_sample_index].info.valid_data = true;
    }
    
    impl->num_samples++;
    impl->next_sample_index = (impl->next_sample_index + 1) % impl->max_samples;
    
    /* 释放互斥锁，因为回调函数可能会调用take/read等需要获取互斥锁的函数 */
    pthread_mutex_unlock(&impl->mutex);
    
    /* 调用数据可用回调 */
    if (impl->data_available_cb && impl->reader) {
        impl->data_available_cb(impl->reader, impl->data_available_arg);
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 创建DataReader
 */
lwdistcomm_dds_data_reader_t *lwdistcomm_dds_data_reader_create(lwdistcomm_dds_subscriber_t *subscriber, const lwdistcomm_dds_data_reader_options_t *options)
{
    if (!subscriber || !options || !options->topic) {
        return NULL;
    }
    
    lwdistcomm_dds_data_reader_t *reader = (lwdistcomm_dds_data_reader_t *)malloc(sizeof(lwdistcomm_dds_data_reader_t));
    if (!reader) {
        return NULL;
    }
    
    /* 初始化基础实体 */
    reader->base.kind = LWDISTCOMM_DDS_ENTITY_DATA_READER;
    reader->base.status_mask = LWDISTCOMM_DDS_STATUS_MASK_ALL;
    
    /* 创建内部实现 */
    reader->impl = lwdistcomm_dds_data_reader_impl_create(options, subscriber);
    if (!reader->impl) {
        free(reader);
        return NULL;
    }
    
    reader->base.impl = reader->impl;
    reader->topic = options->topic;
    
    /* 设置reader指针 */
    reader->impl->reader = reader;
    
    /* 将DataReader添加到Subscriber */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_subscriber_impl_add_data_reader(subscriber->impl, reader);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        lwdistcomm_dds_data_reader_impl_destroy(reader->impl);
        free(reader);
        return NULL;
    }
    
    /* 将DataReader添加到Topic */
    ret = lwdistcomm_dds_topic_impl_add_data_reader(options->topic->impl, reader);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        lwdistcomm_dds_subscriber_impl_remove_data_reader(subscriber->impl, reader);
        lwdistcomm_dds_data_reader_impl_destroy(reader->impl);
        free(reader);
        return NULL;
    }
    
    /* 启用DataReader */
    reader->impl->enabled = true;
    
    return reader;
}

/**
 * 删除DataReader
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_delete(lwdistcomm_dds_data_reader_t *reader)
{
    if (!reader || !reader->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    lwdistcomm_dds_subscriber_t *subscriber = reader->impl->subscriber;
    lwdistcomm_dds_topic_t *topic = reader->topic;
    
    /* 禁用DataReader */
    reader->impl->enabled = false;
    
    /* 从Topic移除DataReader */
    lwdistcomm_dds_topic_impl_remove_data_reader(topic->impl, reader);
    
    /* 从Subscriber移除DataReader */
    lwdistcomm_dds_subscriber_impl_remove_data_reader(subscriber->impl, reader);
    
    /* 销毁内部实现 */
    lwdistcomm_dds_data_reader_impl_destroy(reader->impl);
    
    /* 释放内存 */
    free(reader);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 读取数据（非破坏性）
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_read(lwdistcomm_dds_data_reader_t *reader, void *data, uint32_t *size, lwdistcomm_dds_sample_info_t *info)
{
    if (!reader || !reader->impl || !data || !size) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    if (!reader->impl->enabled) {
        return LWDISTCOMM_DDS_RETCODE_NOT_ENABLED;
    }
    
    pthread_mutex_lock(&reader->impl->mutex);
    
    if (reader->impl->num_samples == 0) {
        pthread_mutex_unlock(&reader->impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_NO_DATA;
    }
    
    /* 计算最早样本的索引 */
    uint32_t oldest_index = (reader->impl->next_sample_index - reader->impl->num_samples + reader->impl->max_samples) % reader->impl->max_samples;
    
    /* 检查数据缓冲区大小 */
    if (*size < reader->impl->samples[oldest_index].size) {
        *size = reader->impl->samples[oldest_index].size;
        pthread_mutex_unlock(&reader->impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    /* 复制数据 */
    memcpy(data, reader->impl->samples[oldest_index].data, reader->impl->samples[oldest_index].size);
    *size = reader->impl->samples[oldest_index].size;
    
    /* 复制样本信息 */
    if (info) {
        memcpy(info, &reader->impl->samples[oldest_index].info, sizeof(lwdistcomm_dds_sample_info_t));
    }
    
    pthread_mutex_unlock(&reader->impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 读取数据（破坏性）
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_take(lwdistcomm_dds_data_reader_t *reader, void *data, uint32_t *size, lwdistcomm_dds_sample_info_t *info)
{
    if (!reader || !reader->impl || !data || !size) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    if (!reader->impl->enabled) {
        return LWDISTCOMM_DDS_RETCODE_NOT_ENABLED;
    }
    
    pthread_mutex_lock(&reader->impl->mutex);
    
    if (reader->impl->num_samples == 0) {
        pthread_mutex_unlock(&reader->impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_NO_DATA;
    }
    
    /* 计算最早样本的索引 */
    uint32_t oldest_index = (reader->impl->next_sample_index - reader->impl->num_samples + reader->impl->max_samples) % reader->impl->max_samples;
    
    /* 检查数据缓冲区大小 */
    if (*size < reader->impl->samples[oldest_index].size) {
        *size = reader->impl->samples[oldest_index].size;
        pthread_mutex_unlock(&reader->impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    /* 复制数据 */
    memcpy(data, reader->impl->samples[oldest_index].data, reader->impl->samples[oldest_index].size);
    *size = reader->impl->samples[oldest_index].size;
    
    /* 复制样本信息 */
    if (info) {
        memcpy(info, &reader->impl->samples[oldest_index].info, sizeof(lwdistcomm_dds_sample_info_t));
    }
    
    /* 移除样本 */
    free(reader->impl->samples[oldest_index].data);
    reader->impl->num_samples--;
    
    pthread_mutex_unlock(&reader->impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置数据可用回调
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_set_data_available_callback(lwdistcomm_dds_data_reader_t *reader, lwdistcomm_dds_data_available_cb_t callback, void *arg)
{
    if (!reader || !reader->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&reader->impl->mutex);
    
    reader->impl->data_available_cb = callback;
    reader->impl->data_available_arg = arg;
    
    pthread_mutex_unlock(&reader->impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
