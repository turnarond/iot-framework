#ifndef LWDISTCOMM_DDS_TYPES_H
#define LWDISTCOMM_DDS_TYPES_H

#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DDS域ID类型
 */
typedef uint32_t lwdistcomm_dds_domainid_t;

/**
 * DDS状态掩码类型
 */
typedef uint32_t lwdistcomm_dds_statusmask_t;

/**
 * DDS时间类型
 */
typedef struct {
    int64_t sec;
    uint32_t nanosec;
} lwdistcomm_dds_time_t;

/**
 * DDS持续时间类型
 */
typedef struct {
    int64_t sec;
    uint32_t nanosec;
} lwdistcomm_dds_duration_t;

/**
 * DDS返回代码
 */
typedef enum {
    LWDISTCOMM_DDS_RETCODE_OK = 0,
    LWDISTCOMM_DDS_RETCODE_ERROR = -1,
    LWDISTCOMM_DDS_RETCODE_UNSUPPORTED = -2,
    LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER = -3,
    LWDISTCOMM_DDS_RETCODE_PRECONDITION_NOT_MET = -4,
    LWDISTCOMM_DDS_RETCODE_OUT_OF_RESOURCES = -5,
    LWDISTCOMM_DDS_RETCODE_NOT_ENABLED = -6,
    LWDISTCOMM_DDS_RETCODE_IMMUTABLE_POLICY = -7,
    LWDISTCOMM_DDS_RETCODE_INCONSISTENT_POLICY = -8,
    LWDISTCOMM_DDS_RETCODE_ALREADY_DELETED = -9,
    LWDISTCOMM_DDS_RETCODE_TIMEOUT = -10,
    LWDISTCOMM_DDS_RETCODE_NO_DATA = -11,
    LWDISTCOMM_DDS_RETCODE_ILLEGAL_OPERATION = -12
} lwdistcomm_dds_retcode_t;

/**
 * DDS实体状态
 */
typedef enum {
    LWDISTCOMM_DDS_STATUS_MASK_NONE = 0,
    LWDISTCOMM_DDS_STATUS_MASK_ALL = 0xFFFFFFFF,
    LWDISTCOMM_DDS_STATUS_PUBLICATION_MATCHED = (1 << 0),
    LWDISTCOMM_DDS_STATUS_SUBSCRIPTION_MATCHED = (1 << 1),
    LWDISTCOMM_DDS_STATUS_SAMPLE_LOST = (1 << 2),
    LWDISTCOMM_DDS_STATUS_SAMPLE_REJECTED = (1 << 3),
    LWDISTCOMM_DDS_STATUS_DATA_AVAILABLE = (1 << 4),
    LWDISTCOMM_DDS_STATUS_LIVELINESS_LOST = (1 << 5),
    LWDISTCOMM_DDS_STATUS_LIVELINESS_CHANGED = (1 << 6),
    LWDISTCOMM_DDS_STATUS_OFFERED_DEADLINE_MISSED = (1 << 7),
    LWDISTCOMM_DDS_STATUS_REQUESTED_DEADLINE_MISSED = (1 << 8),
    LWDISTCOMM_DDS_STATUS_OFFERED_INCOMPATIBLE_QOS = (1 << 9),
    LWDISTCOMM_DDS_STATUS_REQUESTED_INCOMPATIBLE_QOS = (1 << 10),
    LWDISTCOMM_DDS_STATUS_ALL_DATA_DISPOSED = (1 << 11)
} lwdistcomm_dds_status_t;

/**
 * DDS可靠性策略
 */
typedef enum {
    LWDISTCOMM_DDS_RELIABILITY_BEST_EFFORT,
    LWDISTCOMM_DDS_RELIABILITY_RELIABLE
} lwdistcomm_dds_reliability_kind_t;

/**
 * DDS可靠性策略结构体
 */
typedef struct {
    lwdistcomm_dds_reliability_kind_t kind;
    lwdistcomm_dds_duration_t max_blocking_time;
} lwdistcomm_dds_reliability_qos_t;

/**
 * DDS持久性策略
 */
typedef enum {
    LWDISTCOMM_DDS_DURABILITY_VOLATILE,
    LWDISTCOMM_DDS_DURABILITY_TRANSIENT_LOCAL,
    LWDISTCOMM_DDS_DURABILITY_TRANSIENT,
    LWDISTCOMM_DDS_DURABILITY_PERSISTENT
} lwdistcomm_dds_durability_kind_t;

/**
 * DDS持久性策略结构体
 */
typedef struct {
    lwdistcomm_dds_durability_kind_t kind;
} lwdistcomm_dds_durability_qos_t;

/**
 * DDS存活策略
 */
typedef enum {
    LWDISTCOMM_DDS_LIVELINESS_AUTOMATIC,
    LWDISTCOMM_DDS_LIVELINESS_MANUAL_BY_PARTICIPANT,
    LWDISTCOMM_DDS_LIVELINESS_MANUAL_BY_TOPIC
} lwdistcomm_dds_liveliness_kind_t;

/**
 * DDS存活策略结构体
 */
typedef struct {
    lwdistcomm_dds_liveliness_kind_t kind;
    lwdistcomm_dds_duration_t lease_duration;
} lwdistcomm_dds_liveliness_qos_t;

/**
 * DDS截止期限策略结构体
 */
typedef struct {
    lwdistcomm_dds_duration_t period;
} lwdistcomm_dds_deadline_qos_t;

/**
 * DDS历史记录策略
 */
typedef enum {
    LWDISTCOMM_DDS_HISTORY_KEEP_LAST,
    LWDISTCOMM_DDS_HISTORY_KEEP_ALL
} lwdistcomm_dds_history_kind_t;

/**
 * DDS历史记录策略结构体
 */
typedef struct {
    lwdistcomm_dds_history_kind_t kind;
    uint32_t depth;
} lwdistcomm_dds_history_qos_t;

/**
 * DDS资源限制策略结构体
 */
typedef struct {
    uint32_t max_samples;
    uint32_t max_instances;
    uint32_t max_samples_per_instance;
} lwdistcomm_dds_resource_limits_qos_t;

/**
 * DDS服务质量策略集合
 */
typedef struct {
    lwdistcomm_dds_reliability_qos_t reliability;
    lwdistcomm_dds_durability_qos_t durability;
    lwdistcomm_dds_liveliness_qos_t liveliness;
    lwdistcomm_dds_deadline_qos_t deadline;
    lwdistcomm_dds_history_qos_t history;
    lwdistcomm_dds_resource_limits_qos_t resource_limits;
} lwdistcomm_dds_qos_t;

/**
 * DDS实体类型
 */
typedef enum {
    LWDISTCOMM_DDS_ENTITY_DOMAIN_PARTICIPANT,
    LWDISTCOMM_DDS_ENTITY_TOPIC,
    LWDISTCOMM_DDS_ENTITY_PUBLISHER,
    LWDISTCOMM_DDS_ENTITY_SUBSCRIBER,
    LWDISTCOMM_DDS_ENTITY_DATA_WRITER,
    LWDISTCOMM_DDS_ENTITY_DATA_READER
} lwdistcomm_dds_entity_kind_t;

/**
 * DDS实体基类
 */
typedef struct {
    lwdistcomm_dds_entity_kind_t kind;
    lwdistcomm_dds_statusmask_t status_mask;
    void *impl;
} lwdistcomm_dds_entity_t;

/**
 * DDS默认值
 */
#define LWDISTCOMM_DDS_DEFAULT_DOMAIN_ID 0
#define LWDISTCOMM_DDS_DEFAULT_MAX_BLOCKING_TIME_SEC 1
#define LWDISTCOMM_DDS_DEFAULT_MAX_BLOCKING_TIME_NANOSEC 0
#define LWDISTCOMM_DDS_DEFAULT_LEASE_DURATION_SEC 10
#define LWDISTCOMM_DDS_DEFAULT_LEASE_DURATION_NANOSEC 0
#define LWDISTCOMM_DDS_DEFAULT_DEADLINE_PERIOD_SEC 1
#define LWDISTCOMM_DDS_DEFAULT_DEADLINE_PERIOD_NANOSEC 0
#define LWDISTCOMM_DDS_DEFAULT_HISTORY_DEPTH 1
#define LWDISTCOMM_DDS_DEFAULT_MAX_SAMPLES 1024
#define LWDISTCOMM_DDS_DEFAULT_MAX_INSTANCES 1024
#define LWDISTCOMM_DDS_DEFAULT_MAX_SAMPLES_PER_INSTANCE 1

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_TYPES_H */
