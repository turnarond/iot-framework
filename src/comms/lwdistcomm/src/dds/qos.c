#include "../../include/dds/dds.h"
#include <stdlib.h>
#include <string.h>

/**
 * 设置QoS为默认值
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_default(lwdistcomm_dds_qos_t *qos)
{
    if (!qos) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    /* 设置可靠性策略默认值 */
    qos->reliability.kind = LWDISTCOMM_DDS_RELIABILITY_BEST_EFFORT;
    qos->reliability.max_blocking_time.sec = LWDISTCOMM_DDS_DEFAULT_MAX_BLOCKING_TIME_SEC;
    qos->reliability.max_blocking_time.nanosec = LWDISTCOMM_DDS_DEFAULT_MAX_BLOCKING_TIME_NANOSEC;
    
    /* 设置持久性策略默认值 */
    qos->durability.kind = LWDISTCOMM_DDS_DURABILITY_VOLATILE;
    
    /* 设置存活策略默认值 */
    qos->liveliness.kind = LWDISTCOMM_DDS_LIVELINESS_AUTOMATIC;
    qos->liveliness.lease_duration.sec = LWDISTCOMM_DDS_DEFAULT_LEASE_DURATION_SEC;
    qos->liveliness.lease_duration.nanosec = LWDISTCOMM_DDS_DEFAULT_LEASE_DURATION_NANOSEC;
    
    /* 设置截止期限策略默认值 */
    qos->deadline.period.sec = LWDISTCOMM_DDS_DEFAULT_DEADLINE_PERIOD_SEC;
    qos->deadline.period.nanosec = LWDISTCOMM_DDS_DEFAULT_DEADLINE_PERIOD_NANOSEC;
    
    /* 设置历史记录策略默认值 */
    qos->history.kind = LWDISTCOMM_DDS_HISTORY_KEEP_LAST;
    qos->history.depth = LWDISTCOMM_DDS_DEFAULT_HISTORY_DEPTH;
    
    /* 设置资源限制策略默认值 */
    qos->resource_limits.max_samples = LWDISTCOMM_DDS_DEFAULT_MAX_SAMPLES;
    qos->resource_limits.max_instances = LWDISTCOMM_DDS_DEFAULT_MAX_INSTANCES;
    qos->resource_limits.max_samples_per_instance = LWDISTCOMM_DDS_DEFAULT_MAX_SAMPLES_PER_INSTANCE;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 复制QoS策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_copy(const lwdistcomm_dds_qos_t *src, lwdistcomm_dds_qos_t *dst)
{
    if (!src || !dst) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    /* 复制可靠性策略 */
    dst->reliability.kind = src->reliability.kind;
    dst->reliability.max_blocking_time.sec = src->reliability.max_blocking_time.sec;
    dst->reliability.max_blocking_time.nanosec = src->reliability.max_blocking_time.nanosec;
    
    /* 复制持久性策略 */
    dst->durability.kind = src->durability.kind;
    
    /* 复制存活策略 */
    dst->liveliness.kind = src->liveliness.kind;
    dst->liveliness.lease_duration.sec = src->liveliness.lease_duration.sec;
    dst->liveliness.lease_duration.nanosec = src->liveliness.lease_duration.nanosec;
    
    /* 复制截止期限策略 */
    dst->deadline.period.sec = src->deadline.period.sec;
    dst->deadline.period.nanosec = src->deadline.period.nanosec;
    
    /* 复制历史记录策略 */
    dst->history.kind = src->history.kind;
    dst->history.depth = src->history.depth;
    
    /* 复制资源限制策略 */
    dst->resource_limits.max_samples = src->resource_limits.max_samples;
    dst->resource_limits.max_instances = src->resource_limits.max_instances;
    dst->resource_limits.max_samples_per_instance = src->resource_limits.max_samples_per_instance;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置可靠性策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_reliability(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_reliability_kind_t kind, const lwdistcomm_dds_duration_t *max_blocking_time)
{
    if (!qos) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    qos->reliability.kind = kind;
    if (max_blocking_time) {
        qos->reliability.max_blocking_time.sec = max_blocking_time->sec;
        qos->reliability.max_blocking_time.nanosec = max_blocking_time->nanosec;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置持久性策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_durability(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_durability_kind_t kind)
{
    if (!qos) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    qos->durability.kind = kind;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置存活策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_liveliness(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_liveliness_kind_t kind, const lwdistcomm_dds_duration_t *lease_duration)
{
    if (!qos) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    qos->liveliness.kind = kind;
    if (lease_duration) {
        qos->liveliness.lease_duration.sec = lease_duration->sec;
        qos->liveliness.lease_duration.nanosec = lease_duration->nanosec;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置截止期限策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_deadline(lwdistcomm_dds_qos_t *qos, const lwdistcomm_dds_duration_t *period)
{
    if (!qos || !period) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    qos->deadline.period.sec = period->sec;
    qos->deadline.period.nanosec = period->nanosec;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置历史记录策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_history(lwdistcomm_dds_qos_t *qos, lwdistcomm_dds_history_kind_t kind, uint32_t depth)
{
    if (!qos) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    qos->history.kind = kind;
    qos->history.depth = depth;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置资源限制策略
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_qos_set_resource_limits(lwdistcomm_dds_qos_t *qos, uint32_t max_samples, uint32_t max_instances, uint32_t max_samples_per_instance)
{
    if (!qos) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    qos->resource_limits.max_samples = max_samples;
    qos->resource_limits.max_instances = max_instances;
    qos->resource_limits.max_samples_per_instance = max_samples_per_instance;
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
