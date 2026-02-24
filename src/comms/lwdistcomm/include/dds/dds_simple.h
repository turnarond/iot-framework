#ifndef LWDISTCOMM_DDS_SIMPLE_H
#define LWDISTCOMM_DDS_SIMPLE_H

#include "dds_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 简化的DDS发布者接口
 */
typedef struct {
    void *impl;
} lwdistcomm_dds_simple_publisher_t;

/**
 * 简化的DDS订阅者接口
 */
typedef struct {
    void *impl;
} lwdistcomm_dds_simple_subscriber_t;

/**
 * 订阅者数据回调函数
 */
typedef void (*lwdistcomm_dds_simple_data_cb_t)(const void *data, uint32_t size, void *arg);

/**
 * 简化的DDS主题选项
 */
typedef struct {
    char *topic_name;
    char *type_name;
    lwdistcomm_dds_domainid_t domain_id;
} lwdistcomm_dds_simple_topic_options_t;

/**
 * 创建一个简化的DDS发布者
 * 
 * @param options 主题选项
 * @return 发布者实例，失败返回NULL
 */
lwdistcomm_dds_simple_publisher_t *lwdistcomm_dds_simple_publisher_create(const lwdistcomm_dds_simple_topic_options_t *options);

/**
 * 销毁一个简化的DDS发布者
 * 
 * @param publisher 发布者实例
 * @return 成功返回LWDISTCOMM_DDS_RETCODE_OK，失败返回错误码
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_publisher_destroy(lwdistcomm_dds_simple_publisher_t *publisher);

/**
 * 发布数据到主题
 * 
 * @param publisher 发布者实例
 * @param data 数据指针
 * @param size 数据大小
 * @return 成功返回LWDISTCOMM_DDS_RETCODE_OK，失败返回错误码
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_publish(lwdistcomm_dds_simple_publisher_t *publisher, const void *data, uint32_t size);

/**
 * 创建一个简化的DDS订阅者
 * 
 * @param options 主题选项
 * @param callback 数据回调函数
 * @param arg 回调参数
 * @return 订阅者实例，失败返回NULL
 */
lwdistcomm_dds_simple_subscriber_t *lwdistcomm_dds_simple_subscriber_create(const lwdistcomm_dds_simple_topic_options_t *options, lwdistcomm_dds_simple_data_cb_t callback, void *arg);

/**
 * 销毁一个简化的DDS订阅者
 * 
 * @param subscriber 订阅者实例
 * @return 成功返回LWDISTCOMM_DDS_RETCODE_OK，失败返回错误码
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_subscriber_destroy(lwdistcomm_dds_simple_subscriber_t *subscriber);

/**
 * 处理订阅者事件（非阻塞）
 * 
 * @param subscriber 订阅者实例
 * @return 成功返回LWDISTCOMM_DDS_RETCODE_OK，失败返回错误码
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_subscriber_process_events(lwdistcomm_dds_simple_subscriber_t *subscriber);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_SIMPLE_H */
