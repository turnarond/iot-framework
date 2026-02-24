#ifndef LWDISTCOMM_DDS_H
#define LWDISTCOMM_DDS_H

#include "dds_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 前向声明
 */
typedef struct lwdistcomm_dds_domain_participant_impl_s lwdistcomm_dds_domain_participant_impl_t;
typedef struct lwdistcomm_dds_topic_impl_s lwdistcomm_dds_topic_impl_t;
typedef struct lwdistcomm_dds_publisher_impl_s lwdistcomm_dds_publisher_impl_t;
typedef struct lwdistcomm_dds_subscriber_impl_s lwdistcomm_dds_subscriber_impl_t;
typedef struct lwdistcomm_dds_data_writer_impl_s lwdistcomm_dds_data_writer_impl_t;
typedef struct lwdistcomm_dds_data_reader_impl_s lwdistcomm_dds_data_reader_impl_t;

/**
 * DomainParticipant（域参与者）
 */
typedef struct {
    lwdistcomm_dds_entity_t base;
    lwdistcomm_dds_domainid_t domain_id;
    lwdistcomm_dds_domain_participant_impl_t *impl;
} lwdistcomm_dds_domain_participant_t;

/**
 * Topic（主题）
 */
typedef struct {
    lwdistcomm_dds_entity_t base;
    char *name;
    char *type_name;
    lwdistcomm_dds_topic_impl_t *impl;
} lwdistcomm_dds_topic_t;

/**
 * Publisher（发布者）
 */
typedef struct {
    lwdistcomm_dds_entity_t base;
    lwdistcomm_dds_publisher_impl_t *impl;
} lwdistcomm_dds_publisher_t;

/**
 * Subscriber（订阅者）
 */
typedef struct {
    lwdistcomm_dds_entity_t base;
    lwdistcomm_dds_subscriber_impl_t *impl;
} lwdistcomm_dds_subscriber_t;

/**
 * DataWriter（数据写入器）
 */
typedef struct {
    lwdistcomm_dds_entity_t base;
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_data_writer_impl_t *impl;
} lwdistcomm_dds_data_writer_t;

/**
 * DataReader（数据读取器）
 */
typedef struct {
    lwdistcomm_dds_entity_t base;
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_data_reader_impl_t *impl;
} lwdistcomm_dds_data_reader_t;

/**
 * DomainParticipant创建选项
 */
typedef struct {
    lwdistcomm_dds_domainid_t domain_id;
    lwdistcomm_dds_qos_t qos;
    bool enable_automatic_discovery;
    uint32_t discovery_port;
} lwdistcomm_dds_domain_participant_options_t;

/**
 * Topic创建选项
 */
typedef struct {
    char *name;
    char *type_name;
    lwdistcomm_dds_qos_t qos;
} lwdistcomm_dds_topic_options_t;

/**
 * Publisher创建选项
 */
typedef struct {
    lwdistcomm_dds_qos_t qos;
} lwdistcomm_dds_publisher_options_t;

/**
 * Subscriber创建选项
 */
typedef struct {
    lwdistcomm_dds_qos_t qos;
} lwdistcomm_dds_subscriber_options_t;

/**
 * DataWriter创建选项
 */
typedef struct {
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_qos_t qos;
} lwdistcomm_dds_data_writer_options_t;

/**
 * DataReader创建选项
 */
typedef struct {
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_qos_t qos;
} lwdistcomm_dds_data_reader_options_t;

/**
 * 数据样本信息
 */
typedef struct {
    bool valid_data;
    lwdistcomm_dds_time_t source_timestamp;
    lwdistcomm_dds_time_t reception_timestamp;
    uint32_t instance_handle;
    uint32_t publication_handle;
} lwdistcomm_dds_sample_info_t;

/**
 * DataReader数据可用回调
 */
typedef void (*lwdistcomm_dds_data_available_cb_t)(lwdistcomm_dds_data_reader_t *reader, void *arg);

/**
 * DomainParticipant相关API
 */
lwdistcomm_dds_domain_participant_t *lwdistcomm_dds_domain_participant_create(const lwdistcomm_dds_domain_participant_options_t *options);
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_delete(lwdistcomm_dds_domain_participant_t *participant);
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_enable(lwdistcomm_dds_domain_participant_t *participant);
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_disable(lwdistcomm_dds_domain_participant_t *participant);

/**
 * Topic相关API
 */
lwdistcomm_dds_topic_t *lwdistcomm_dds_topic_create(lwdistcomm_dds_domain_participant_t *participant, const lwdistcomm_dds_topic_options_t *options);
lwdistcomm_dds_retcode_t lwdistcomm_dds_topic_delete(lwdistcomm_dds_topic_t *topic);

/**
 * Publisher相关API
 */
lwdistcomm_dds_publisher_t *lwdistcomm_dds_publisher_create(lwdistcomm_dds_domain_participant_t *participant, const lwdistcomm_dds_publisher_options_t *options);
lwdistcomm_dds_retcode_t lwdistcomm_dds_publisher_delete(lwdistcomm_dds_publisher_t *publisher);

/**
 * Subscriber相关API
 */
lwdistcomm_dds_subscriber_t *lwdistcomm_dds_subscriber_create(lwdistcomm_dds_domain_participant_t *participant, const lwdistcomm_dds_subscriber_options_t *options);
lwdistcomm_dds_retcode_t lwdistcomm_dds_subscriber_delete(lwdistcomm_dds_subscriber_t *subscriber);

/**
 * DataWriter相关API
 */
lwdistcomm_dds_data_writer_t *lwdistcomm_dds_data_writer_create(lwdistcomm_dds_publisher_t *publisher, const lwdistcomm_dds_data_writer_options_t *options);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_writer_delete(lwdistcomm_dds_data_writer_t *writer);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_writer_write(lwdistcomm_dds_data_writer_t *writer, const void *data, uint32_t size);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_writer_dispose(lwdistcomm_dds_data_writer_t *writer, const void *data, uint32_t size);

/**
 * DataReader相关API
 */
lwdistcomm_dds_data_reader_t *lwdistcomm_dds_data_reader_create(lwdistcomm_dds_subscriber_t *subscriber, const lwdistcomm_dds_data_reader_options_t *options);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_delete(lwdistcomm_dds_data_reader_t *reader);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_read(lwdistcomm_dds_data_reader_t *reader, void *data, uint32_t *size, lwdistcomm_dds_sample_info_t *info);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_take(lwdistcomm_dds_data_reader_t *reader, void *data, uint32_t *size, lwdistcomm_dds_sample_info_t *info);
lwdistcomm_dds_retcode_t lwdistcomm_dds_data_reader_set_data_available_callback(lwdistcomm_dds_data_reader_t *reader, lwdistcomm_dds_data_available_cb_t callback, void *arg);

/**
 * QoS相关API
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_default(lwdistcomm_dds_qos_t *qos);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_copy(const lwdistcomm_dds_qos_t *src, lwdistcomm_dds_qos_t *dst);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_reliability(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_reliability_kind_t kind, const lwdistcomm_dds_duration_t *max_blocking_time);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_durability(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_durability_kind_t kind);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_liveliness(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_liveliness_kind_t kind, const lwdistcomm_dds_duration_t *lease_duration);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_deadline(lwdistcomm_dds_qos_t *qos, const lwdistcomm_dds_duration_t *period);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_history(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_history_kind_t kind, uint32_t depth);
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_resource_limits(lwdistcomm_dds_qos_t *qos, uint32_t max_samples, uint32_t max_instances, uint32_t max_samples_per_instance);

/**
 * 发现机制相关API
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_get_discovered_participants(lwdistcomm_dds_domain_participant_t *participant, uint32_t *count);
lwdistcomm_dds_retcode_t lwdistcomm_dds_domain_participant_get_discovered_topics(lwdistcomm_dds_domain_participant_t *participant, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_H */
