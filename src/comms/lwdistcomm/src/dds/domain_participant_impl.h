#ifndef LWDISTCOMM_DDS_DOMAIN_PARTICIPANT_IMPL_H
#define LWDISTCOMM_DDS_DOMAIN_PARTICIPANT_IMPL_H

#include "../../include/dds/dds.h"
#include "../../include/dds/spdp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DomainParticipant内部实现结构
 */
struct lwdistcomm_dds_domain_participant_impl_s {
    lwdistcomm_dds_domainid_t domain_id;
    bool enabled;
    bool automatic_discovery;
    uint32_t discovery_port;
    
    /* 已发现的参与者列表 */
    struct discovered_participant {
        uint32_t handle;
        char *name;
        char *address;
    } *discovered_participants;
    uint32_t num_discovered_participants;
    uint32_t max_discovered_participants;
    
    /* 已发现的主题列表 */
    struct discovered_topic {
        uint32_t handle;
        char *name;
        char *type_name;
    } *discovered_topics;
    uint32_t num_discovered_topics;
    uint32_t max_discovered_topics;
    
    /* 主题列表 */
    lwdistcomm_dds_topic_t **topics;
    uint32_t num_topics;
    uint32_t max_topics;
    
    /* 发布者列表 */
    lwdistcomm_dds_publisher_t **publishers;
    uint32_t num_publishers;
    uint32_t max_publishers;
    
    /* 订阅者列表 */
    lwdistcomm_dds_subscriber_t **subscribers;
    uint32_t num_subscribers;
    uint32_t max_subscribers;
    
    /* 网络相关 */
    int discovery_socket;
    pthread_t discovery_thread;
    bool discovery_thread_running;
    
    /* SPDP相关 */
    lwdistcomm_dds_spdp_t *spdp;
    
    /* 互斥锁 */
    pthread_mutex_t mutex;
};

/**
 * 创建DomainParticipant内部实现
 */
lwdistcomm_dds_domain_participant_impl_t *lwdistcomm_dds_domain_participant_impl_create(const lwdistcomm_dds_domain_participant_options_t *options);

/**
 * 销毁DomainParticipant内部实现
 */
void lwdistcomm_dds_domain_participant_impl_destroy(lwdistcomm_dds_domain_participant_impl_t *impl);

/**
 * 添加主题到DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_add_topic(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_topic_t *topic);

/**
 * 从DomainParticipant移除主题
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_remove_topic(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_topic_t *topic);

/**
 * 添加发布者到DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_add_publisher(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_publisher_t *publisher);

/**
 * 从DomainParticipant移除发布者
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_remove_publisher(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_publisher_t *publisher);

/**
 * 添加订阅者到DomainParticipant
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_add_subscriber(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_subscriber_t *subscriber);

/**
 * 从DomainParticipant移除订阅者
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_remove_subscriber(lwdistcomm_dds_domain_participant_impl_t *impl, lwdistcomm_dds_subscriber_t *subscriber);

/**
 * 启动发现线程
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_start_discovery(lwdistcomm_dds_domain_participant_impl_t *impl);

/**
 * 停止发现线程
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_impl_stop_discovery(lwdistcomm_dds_domain_participant_impl_t *impl);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_DOMAIN_PARTICIPANT_IMPL_H */
