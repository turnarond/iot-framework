#include "domain_participant_impl.h"
#include "../../include/dds/spdp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * 发现线程函数声明
 */
extern void *lwdistcomm_dds_discovery_thread_func(void *arg);

#define MAX_DISCOVERED_PARTICIPANTS 64
#define MAX_DISCOVERED_TOPICS 128
#define MAX_TOPICS 64
#define MAX_PUBLISHERS 32
#define MAX_SUBSCRIBERS 32
#define DEFAULT_DISCOVERY_PORT 7400

/**
 * 发现线程函数
 */
static void *discovery_thread_func(void *arg)
{
    lwdistcomm_dds_domain_participant_impl_t *impl = (lwdistcomm_dds_domain_participant_impl_t *)arg;
    int sockfd;
    struct sockaddr_in addr;
    char buffer[1024];
    socklen_t addr_len;
    
    /* 创建UDP套接字 */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return NULL;
    }
    
    /* 设置地址重用 */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* 绑定地址 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(impl->discovery_port);
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return NULL;
    }
    
    impl->discovery_socket = sockfd;
    impl->discovery_thread_running = true;
    
    /* 主循环 */
    while (impl->discovery_thread_running) {
        addr_len = sizeof(addr);
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
        if (n > 0) {
            /* 处理发现消息 */
            // TODO: 实现发现消息的解析和处理
        }
    }
    
    close(sockfd);
    impl->discovery_socket = -1;
    
    return NULL;
}

/**
 * 创建DomainParticipant内部实现
 */
lwdistcomm_dds_domain_participant_impl_t *lwdistcomm_dds_domain_participant_impl_create(const lwdistcomm_dds_domain_participant_options_t *options)
{
    lwdistcomm_dds_domain_participant_impl_t *impl = (lwdistcomm_dds_domain_participant_impl_t *)malloc(sizeof(lwdistcomm_dds_domain_participant_impl_t));
    if (!impl) {
        return NULL;
    }
    
    memset(impl, 0, sizeof(lwdistcomm_dds_domain_participant_impl_t));
    
    impl->domain_id = options->domain_id;
    impl->enabled = false;
    impl->automatic_discovery = options->enable_automatic_discovery;
    impl->discovery_port = options->discovery_port > 0 ? options->discovery_port : DEFAULT_DISCOVERY_PORT;
    
    /* 初始化列表 */
    impl->discovered_participants = (struct discovered_participant *)malloc(sizeof(struct discovered_participant) * MAX_DISCOVERED_PARTICIPANTS);
    impl->max_discovered_participants = MAX_DISCOVERED_PARTICIPANTS;
    
    impl->discovered_topics = (struct discovered_topic *)malloc(sizeof(struct discovered_topic) * MAX_DISCOVERED_TOPICS);
    impl->max_discovered_topics = MAX_DISCOVERED_TOPICS;
    
    impl->topics = (lwdistcomm_dds_topic_t **)malloc(sizeof(lwdistcomm_dds_topic_t *) * MAX_TOPICS);
    impl->max_topics = MAX_TOPICS;
    
    impl->publishers = (lwdistcomm_dds_publisher_t **)malloc(sizeof(lwdistcomm_dds_publisher_t *) * MAX_PUBLISHERS);
    impl->max_publishers = MAX_PUBLISHERS;
    
    impl->subscribers = (lwdistcomm_dds_subscriber_t **)malloc(sizeof(lwdistcomm_dds_subscriber_t *) * MAX_SUBSCRIBERS);
    impl->max_subscribers = MAX_SUBSCRIBERS;
    
    /* 初始化互斥锁 */
    pthread_mutex_init(&impl->mutex, NULL);
    
    /* 初始化网络相关 */
    impl->discovery_socket = -1;
    impl->discovery_thread_running = false;
    impl->spdp = NULL;
    
    return impl;
}

/**
 * 销毁DomainParticipant内部实现
 */
void lwdistcomm_dds_domain_participant_impl_destroy(lwdistcomm_dds_domain_participant_impl_t *impl)
{
    if (!impl) {
        return;
    }
    
    /* 停止发现线程 */
    if (impl->discovery_thread_running) {
        lwdistcomm_dds_domain_participant_impl_stop_discovery(impl);
    }
    
    /* 清理SPDP */
    if (impl->spdp) {
        lwdistcomm_dds_spdp_delete(impl->spdp);
        impl->spdp = NULL;
    }
    
    /* 释放已发现的参与者 */
    for (uint32_t i = 0; i < impl->num_discovered_participants; i++) {
        free(impl->discovered_participants[i].name);
        free(impl->discovered_participants[i].address);
    }
    free(impl->discovered_participants);
    
    /* 释放已发现的主题 */
    for (uint32_t i = 0; i < impl->num_discovered_topics; i++) {
        free(impl->discovered_topics[i].name);
        free(impl->discovered_topics[i].type_name);
    }
    free(impl->discovered_topics);
    
    /* 释放主题列表 */
    free(impl->topics);
    
    /* 释放发布者列表 */
    free(impl->publishers);
    
    /* 释放订阅者列表 */
    free(impl->subscribers);
    
    /* 销毁互斥锁 */
    pthread_mutex_destroy(&impl->mutex);
    
    free(impl);
}

/**
 * 添加主题到DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_add_topic(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_topic_t *topic)
{
    if (!impl || !topic) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    if (impl->num_topics >= impl->max_topics) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES;
    }
    
    impl->topics[impl->num_topics++] = topic;
    
    pthread_mutex_unlock(&impl->mutex);
    
    /* 发送SPDP主题宣告消息 */
    if (impl->spdp && impl->enabled) {
        lwdistcomm_dds_spdp_send_topic_announce(impl->spdp, topic);
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 从DomainParticipant移除主题
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_remove_topic(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_topic_t *topic)
{
    if (!impl || !topic) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    for (uint32_t i = 0; i < impl->num_topics; i++) {
        if (impl->topics[i] == topic) {
            /* 移除主题 */
            impl->num_topics--;
            if (i < impl->num_topics) {
                impl->topics[i] = impl->topics[impl->num_topics];
            }
            
            pthread_mutex_unlock(&impl->mutex);
            return LWDISTCOMM_DDS_RETCODE_OK;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 添加发布者到DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_add_publisher(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_publisher_t *publisher)
{
    if (!impl || !publisher) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    if (impl->num_publishers >= impl->max_publishers) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES;
    }
    
    impl->publishers[impl->num_publishers++] = publisher;
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 从DomainParticipant移除发布者
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_remove_publisher(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_publisher_t *publisher)
{
    if (!impl || !publisher) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    for (uint32_t i = 0; i < impl->num_publishers; i++) {
        if (impl->publishers[i] == publisher) {
            /* 移除发布者 */
            impl->num_publishers--;
            if (i < impl->num_publishers) {
                impl->publishers[i] = impl->publishers[impl->num_publishers];
            }
            
            pthread_mutex_unlock(&impl->mutex);
            return LWDISTCOMM_DDS_RETCODE_OK;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 添加订阅者到DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_add_subscriber(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_subscriber_t *subscriber)
{
    if (!impl || !subscriber) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    if (impl->num_subscribers >= impl->max_subscribers) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES;
    }
    
    impl->subscribers[impl->num_subscribers++] = subscriber;
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 从DomainParticipant移除订阅者
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_remove_subscriber(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_subscriber_t *subscriber)
{
    if (!impl || !subscriber) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    for (uint32_t i = 0; i < impl->num_subscribers; i++) {
        if (impl->subscribers[i] == subscriber) {
            /* 移除订阅者 */
            impl->num_subscribers--;
            if (i < impl->num_subscribers) {
                impl->subscribers[i] = impl->subscribers[impl->num_subscribers];
            }
            
            pthread_mutex_unlock(&impl->mutex);
            return LWDISTCOMM_DDS_RETCODE_OK;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 启动发现线程
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_start_discovery(lwdistcomm_dds_domain_participant_impl_t *impl)
{
    if (!impl || !impl->automatic_discovery) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    if (impl->discovery_thread_running) {
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    int ret = pthread_create(&impl->discovery_thread, NULL, lwdistcomm_dds_discovery_thread_func, impl);
    if (ret != 0) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 停止发现线程
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_stop_discovery(lwdistcomm_dds_domain_participant_impl_t *impl)
{
    if (!impl || !impl->discovery_thread_running) {
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    impl->discovery_thread_running = false;
    pthread_join(impl->discovery_thread, NULL);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 创建DomainParticipant
 */
lwdistcomm_dds_domain_participant_t *lwdistcomm_dds_domain_participant_create(const lwdistcomm_dds_domain_participant_options_t *options)
{
    if (!options) {
        return NULL;
    }
    
    lwdistcomm_dds_domain_participant_t *participant = (lwdistcomm_dds_domain_participant_t *)malloc(sizeof(lwdistcomm_dds_domain_participant_t));
    if (!participant) {
        return NULL;
    }
    
    /* 初始化基础实体 */
    participant->base.kind = LWDISTCOMM_DDS_ENTITY_DOMAIN_PARTICIPANT;
    participant->base.status_mask = LWDISTCOMM_DDS_STATUS_MASK_ALL;
    
    /* 创建内部实现 */
    participant->domain_id = options->domain_id;
    participant->impl = lwdistcomm_dds_domain_participant_impl_create(options);
    if (!participant->impl) {
        free(participant);
        return NULL;
    }
    
    participant->base.impl = participant->impl;
    
    return participant;
}

/**
 * 删除DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_delete(lwdistcomm_dds_domain_participant_t *participant)
{
    if (!participant) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    /* 禁用DomainParticipant */
    lwdistcomm_dds_domain_participant_disable(participant);
    
    /* 销毁内部实现 */
    lwdistcomm_dds_domain_participant_impl_destroy(participant->impl);
    
    /* 释放内存 */
    free(participant);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 启用DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_enable(lwdistcomm_dds_domain_participant_t *participant)
{
    if (!participant || !participant->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    participant->impl->enabled = true;
    
    /* 初始化并启动SPDP */
    if (participant->impl->automatic_discovery && !participant->impl->spdp) {
        lwdistcomm_dds_spdp_config_t spdp_config;
        memset(&spdp_config, 0, sizeof(spdp_config));
        spdp_config.domain_id = participant->domain_id;
        spdp_config.participant_id = (uint32_t)time(NULL);
        snprintf(spdp_config.participant_name, sizeof(spdp_config.participant_name), "Participant_%u", spdp_config.participant_id);
        strcpy(spdp_config.multicast_address, "239.255.0.1");
        spdp_config.multicast_port = participant->impl->discovery_port;
        spdp_config.announce_interval_sec = 3;
        spdp_config.lease_duration_sec = 10;
        
        participant->impl->spdp = lwdistcomm_dds_spdp_create(&spdp_config, participant);
        if (participant->impl->spdp) {
            lwdistcomm_dds_spdp_start(participant->impl->spdp);
        }
    }
    
    /* 启动发现机制 */
    if (participant->impl->automatic_discovery) {
        lwdistcomm_dds_domain_participant_impl_start_discovery(participant->impl);
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 禁用DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_disable(lwdistcomm_dds_domain_participant_t *participant)
{
    if (!participant || !participant->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    participant->impl->enabled = false;
    
    /* 停止SPDP */
    if (participant->impl->spdp) {
        lwdistcomm_dds_spdp_stop(participant->impl->spdp);
    }
    
    /* 停止发现机制 */
    if (participant->impl->automatic_discovery) {
        lwdistcomm_dds_domain_participant_impl_stop_discovery(participant->impl);
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 获取已发现的参与者数量
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_get_discovered_participants(lwdistcomm_dds_domain_participant_t *participant, uint32_t *count)
{
    if (!participant || !participant->impl || !count) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&participant->impl->mutex);
    *count = participant->impl->num_discovered_participants;
    pthread_mutex_unlock(&participant->impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 获取已发现的主题数量
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_get_discovered_topics(lwdistcomm_dds_domain_participant_t *participant, uint32_t *count)
{
    if (!participant || !participant->impl || !count) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    pthread_mutex_lock(&participant->impl->mutex);
    *count = participant->impl->num_discovered_topics;
    pthread_mutex_unlock(&participant->impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
