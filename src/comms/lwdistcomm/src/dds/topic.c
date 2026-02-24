#include "topic_impl.h"
#include "domain_participant_impl.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_DATA_WRITERS 16
#define MAX_DATA_READERS 16
#define DISCOVERY_MSG_MAX_SIZE 1024

/**
 * 发现消息类型
 */
typedef enum {
    DISCOVERY_MSG_PARTICIPANT_ANNOUNCE,
    DISCOVERY_MSG_PARTICIPANT_LEAVE,
    DISCOVERY_MSG_TOPIC_ANNOUNCE,
    DISCOVERY_MSG_TOPIC_REMOVE,
    DISCOVERY_MSG_ENDPOINT_ANNOUNCE,
    DISCOVERY_MSG_ENDPOINT_REMOVE
} discovery_msg_type_t;

/**
 * 发现消息结构
 */
typedef struct {
    discovery_msg_type_t type;
    uint32_t domain_id;
    uint32_t participant_id;
    char participant_name[64];
    char topic_name[64];
    char type_name[64];
    uint32_t endpoint_id;
    uint8_t is_writer;
} discovery_msg_t;

/**
 * 序列化发现消息
 */
static int serialize_discovery_msg(const discovery_msg_t *msg, char *buffer, size_t buffer_size)
{
    if (!msg || !buffer) {
        return -1;
    }
    
    /* 简单的序列化实现 */
    int offset = 0;
    
    /* 写入消息类型 */
    memcpy(buffer + offset, &msg->type, sizeof(discovery_msg_type_t));
    offset += sizeof(discovery_msg_type_t);
    
    /* 写入域ID */
    memcpy(buffer + offset, &msg->domain_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 写入参与者ID */
    memcpy(buffer + offset, &msg->participant_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 写入参与者名称 */
    strncpy(buffer + offset, msg->participant_name, sizeof(msg->participant_name));
    offset += sizeof(msg->participant_name);
    
    /* 写入主题名称 */
    strncpy(buffer + offset, msg->topic_name, sizeof(msg->topic_name));
    offset += sizeof(msg->topic_name);
    
    /* 写入类型名称 */
    strncpy(buffer + offset, msg->type_name, sizeof(msg->type_name));
    offset += sizeof(msg->type_name);
    
    /* 写入端点ID */
    memcpy(buffer + offset, &msg->endpoint_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 写入是否为写入器 */
    memcpy(buffer + offset, &msg->is_writer, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    return offset;
}

/**
 * 发送发现消息
 */
static int send_discovery_msg(int sockfd, const discovery_msg_t *msg, const struct sockaddr_in *addr)
{
    if (!msg || !addr) {
        return -1;
    }
    
    char buffer[DISCOVERY_MSG_MAX_SIZE];
    int msg_size = serialize_discovery_msg(msg, buffer, sizeof(buffer));
    if (msg_size < 0) {
        return -1;
    }
    
    return sendto(sockfd, buffer, msg_size, 0, (struct sockaddr *)addr, sizeof(*addr));
}

/**
 * 发送主题宣告消息
 */
static void send_topic_announce(lwdistcomm_dds_topic_t *topic)
{
    if (!topic || !topic->impl || !topic->impl->participant || !topic->impl->participant->impl) {
        return;
    }
    
    lwdistcomm_dds_domain_participant_impl_t *dp_impl = topic->impl->participant->impl;
    
    /* 等待发现套接字创建 */
    int retries = 0;
    while (dp_impl->discovery_socket < 0 && retries < 5) {
        usleep(100000); /* 等待100ms */
        retries++;
    }
    
    if (dp_impl->discovery_socket < 0) {
        return;
    }
    
    /* 创建主题宣告消息 */
    discovery_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = DISCOVERY_MSG_TOPIC_ANNOUNCE;
    msg.domain_id = dp_impl->domain_id;
    msg.participant_id = dp_impl->domain_id; /* 使用域ID作为临时参与者ID */
    strncpy(msg.participant_name, "Participant", sizeof(msg.participant_name));
    strncpy(msg.topic_name, topic->name, sizeof(msg.topic_name));
    strncpy(msg.type_name, topic->type_name, sizeof(msg.type_name));
    
    /* 创建广播地址 */
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(dp_impl->discovery_port);
    
    /* 发送广播消息 */
    send_discovery_msg(dp_impl->discovery_socket, &msg, &broadcast_addr);
    
    /* 再次发送，确保消息被接收 */
    usleep(100000); /* 等待100ms */
    send_discovery_msg(dp_impl->discovery_socket, &msg, &broadcast_addr);
}

/**
 * 发送主题移除消息
 */
static void send_topic_remove(lwdistcomm_dds_topic_t *topic)
{
    if (!topic || !topic->impl || !topic->impl->participant || !topic->impl->participant->impl) {
        return;
    }
    
    lwdistcomm_dds_domain_participant_impl_t *dp_impl = topic->impl->participant->impl;
    if (dp_impl->discovery_socket < 0) {
        return;
    }
    
    /* 创建主题移除消息 */
    discovery_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = DISCOVERY_MSG_TOPIC_REMOVE;
    msg.domain_id = dp_impl->domain_id;
    msg.participant_id = dp_impl->domain_id; /* 使用域ID作为临时参与者ID */
    strncpy(msg.participant_name, "Participant", sizeof(msg.participant_name));
    strncpy(msg.topic_name, topic->name, sizeof(msg.topic_name));
    strncpy(msg.type_name, topic->type_name, sizeof(msg.type_name));
    
    /* 创建广播地址 */
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(dp_impl->discovery_port);
    
    /* 发送广播消息 */
    send_discovery_msg(dp_impl->discovery_socket, &msg, &broadcast_addr);
}

/**
 * 创建Topic内部实现
 */
lwdistcomm_dds_topic_impl_t *lwdistcomm_dds_topic_impl_create(const lwdistcomm_dds_topic_options_t *options, lwdistcomm_dds_domain_participant_t *participant)
{
    lwdistcomm_dds_topic_impl_t *impl = (lwdistcomm_dds_topic_impl_t *)malloc(sizeof(lwdistcomm_dds_topic_impl_t));
    if (!impl) {
        return NULL;
    }
    
    memset(impl, 0, sizeof(lwdistcomm_dds_topic_impl_t));
    
    /* 复制主题名称和类型名称 */
    impl->name = strdup(options->name);
    if (!impl->name) {
        free(impl);
        return NULL;
    }
    
    impl->type_name = strdup(options->type_name);
    if (!impl->type_name) {
        free(impl->name);
        free(impl);
        return NULL;
    }
    
    /* 复制QoS */
    memcpy(&impl->qos, &options->qos, sizeof(lwdistcomm_dds_qos_t));
    
    impl->enabled = false;
    impl->participant = participant;
    
    /* 初始化数据写入器列表 */
    impl->data_writers = (lwdistcomm_dds_data_writer_t **)malloc(sizeof(lwdistcomm_dds_data_writer_t *) * MAX_DATA_WRITERS);
    impl->max_data_writers = MAX_DATA_WRITERS;
    
    /* 初始化数据读取器列表 */
    impl->data_readers = (lwdistcomm_dds_data_reader_t **)malloc(sizeof(lwdistcomm_dds_data_reader_t *) * MAX_DATA_READERS);
    impl->max_data_readers = MAX_DATA_READERS;
    
    /* 初始化互斥锁 */
    pthread_mutex_init(&impl->mutex, NULL);
    
    return impl;
}

/**
 * 销毁Topic内部实现
 */
void lwdistcomm_dds_topic_impl_destroy(lwdistcomm_dds_topic_impl_t *impl)
{
    if (!impl) {
        return;
    }
    
    /* 释放主题名称和类型名称 */
    free(impl->name);
    free(impl->type_name);
    
    /* 释放数据写入器列表 */
    free(impl->data_writers);
    
    /* 释放数据读取器列表 */
    free(impl->data_readers);
    
    /* 销毁互斥锁 */
    pthread_mutex_destroy(&impl->mutex);
    
    free(impl);
}

/**
 * 添加DataWriter到Topic
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_topic_impl_add_data_writer(lwdistcomm_dds_topic_impl_t *impl, lwdistcomm_dds_data_writer_t *writer)
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
 * 从Topic移除DataWriter
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_topic_impl_remove_data_writer(lwdistcomm_dds_topic_impl_t *impl, lwdistcomm_dds_data_writer_t *writer)
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
 * 添加DataReader到Topic
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_topic_impl_add_data_reader(lwdistcomm_dds_topic_impl_t *impl, lwdistcomm_dds_data_reader_t *reader)
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
 * 从Topic移除DataReader
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_topic_impl_remove_data_reader(lwdistcomm_dds_topic_impl_t *impl, lwdistcomm_dds_data_reader_t *reader)
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
 * 创建Topic
 */
lwdistcomm_dds_topic_t *lwdistcomm_dds_topic_create(lwdistcomm_dds_domain_participant_t *participant, const lwdistcomm_dds_topic_options_t *options)
{
    if (!participant || !options || !options->name || !options->type_name) {
        return NULL;
    }
    
    lwdistcomm_dds_topic_t *topic = (lwdistcomm_dds_topic_t *)malloc(sizeof(lwdistcomm_dds_topic_t));
    if (!topic) {
        return NULL;
    }
    
    /* 初始化基础实体 */
    topic->base.kind = LWDISTCOMM_DDS_ENTITY_TOPIC;
    topic->base.status_mask = LWDISTCOMM_DDS_STATUS_MASK_ALL;
    
    /* 创建内部实现 */
    topic->impl = lwdistcomm_dds_topic_impl_create(options, participant);
    if (!topic->impl) {
        free(topic);
        return NULL;
    }
    
    topic->base.impl = topic->impl;
    topic->name = topic->impl->name;
    topic->type_name = topic->impl->type_name;
    
    /* 将主题添加到DomainParticipant */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_impl_add_topic(participant->impl, topic);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        lwdistcomm_dds_topic_impl_destroy(topic->impl);
        free(topic);
        return NULL;
    }
    
    /* 启用主题 */
    topic->impl->enabled = true;
    
    /* 发送主题宣告消息 */
    send_topic_announce(topic);
    
    return topic;
}

/**
 * 删除Topic
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_topic_delete(lwdistcomm_dds_topic_t *topic)
{
    if (!topic || !topic->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    lwdistcomm_dds_domain_participant_t *participant = topic->impl->participant;
    
    /* 禁用主题 */
    topic->impl->enabled = false;
    
    /* 发送主题移除消息 */
    send_topic_remove(topic);
    
    /* 从DomainParticipant移除主题 */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_impl_remove_topic(participant->impl, topic);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        return ret;
    }
    
    /* 销毁内部实现 */
    lwdistcomm_dds_topic_impl_destroy(topic->impl);
    
    /* 释放内存 */
    free(topic);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
