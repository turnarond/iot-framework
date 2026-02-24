#include "dds/dds_simple.h"
#include "dds/dds.h"
#include "domain_participant_impl.h"
#include "topic_impl.h"
#include "publisher_impl.h"
#include "subscriber_impl.h"
#include "data_writer_impl.h"
#include "data_reader_impl.h"
#include <stdlib.h>
#include <string.h>

/**
 * 简化发布者的内部实现
 */
typedef struct {
    lwdistcomm_dds_domain_participant_t *participant;
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_publisher_t *publisher;
    lwdistcomm_dds_data_writer_t *data_writer;
} lwdistcomm_dds_simple_publisher_impl_t;

/**
 * 简化订阅者的内部实现
 */
typedef struct {
    lwdistcomm_dds_domain_participant_t *participant;
    lwdistcomm_dds_topic_t *topic;
    lwdistcomm_dds_subscriber_t *subscriber;
    lwdistcomm_dds_data_reader_t *data_reader;
    lwdistcomm_dds_simple_data_cb_t data_callback;
    void *callback_arg;
} lwdistcomm_dds_simple_subscriber_impl_t;

/**
 * 订阅者数据可用回调
 */
static void data_available_callback(lwdistcomm_dds_data_reader_t *reader, void *arg)
{
    lwdistcomm_dds_simple_subscriber_impl_t *impl = (lwdistcomm_dds_simple_subscriber_impl_t *)arg;
    if (!impl || !impl->data_callback) {
        return;
    }

    uint32_t size = 1024;
    void *buffer = malloc(size);
    if (!buffer) {
        return;
    }

    lwdistcomm_dds_sample_info_t info;
    lwdistcomm_dds_retcode_t ret;

    // 读取所有可用数据
    while (1) {
        ret = lwdistcomm_dds_data_reader_take(reader, buffer, &size, &info);
        if (ret != LWDISTCOMM_DDS_RETCODE_OK || !info.valid_data) {
            break;
        }

        // 调用用户回调
        impl->data_callback(buffer, size, impl->callback_arg);
    }

    free(buffer);
}

/**
 * 创建一个简化的DDS发布者
 */
lwdistcomm_dds_simple_publisher_t *lwdistcomm_dds_simple_publisher_create(const lwdistcomm_dds_simple_topic_options_t *options)
{
    if (!options || !options->topic_name || !options->type_name) {
        return NULL;
    }

    lwdistcomm_dds_simple_publisher_t *publisher = (lwdistcomm_dds_simple_publisher_t *)malloc(sizeof(lwdistcomm_dds_simple_publisher_t));
    if (!publisher) {
        return NULL;
    }

    lwdistcomm_dds_simple_publisher_impl_t *impl = (lwdistcomm_dds_simple_publisher_impl_t *)malloc(sizeof(lwdistcomm_dds_simple_publisher_impl_t));
    if (!impl) {
        free(publisher);
        return NULL;
    }

    memset(impl, 0, sizeof(lwdistcomm_dds_simple_publisher_impl_t));

    // 创建DomainParticipant
    lwdistcomm_dds_domain_participant_options_t dp_options;
    memset(&dp_options, 0, sizeof(dp_options));
    dp_options.domain_id = options->domain_id;
    dp_options.enable_automatic_discovery = true;
    dp_options.discovery_port = 7400;
    lwdistcomm_dds_qos_default(&dp_options.qos);

    impl->participant = lwdistcomm_dds_domain_participant_create(&dp_options);
    if (!impl->participant) {
        free(impl);
        free(publisher);
        return NULL;
    }

    // 创建Topic
    lwdistcomm_dds_topic_options_t topic_options;
    memset(&topic_options, 0, sizeof(topic_options));
    topic_options.name = options->topic_name;
    topic_options.type_name = options->type_name;
    lwdistcomm_dds_qos_default(&topic_options.qos);

    impl->topic = lwdistcomm_dds_topic_create(impl->participant, &topic_options);
    if (!impl->topic) {
        lwdistcomm_dds_domain_participant_delete(impl->participant);
        free(impl);
        free(publisher);
        return NULL;
    }

    // 创建Publisher
    lwdistcomm_dds_publisher_options_t pub_options;
    memset(&pub_options, 0, sizeof(pub_options));
    lwdistcomm_dds_qos_default(&pub_options.qos);

    impl->publisher = lwdistcomm_dds_publisher_create(impl->participant, &pub_options);
    if (!impl->publisher) {
        lwdistcomm_dds_topic_delete(impl->topic);
        lwdistcomm_dds_domain_participant_delete(impl->participant);
        free(impl);
        free(publisher);
        return NULL;
    }

    // 创建DataWriter
    lwdistcomm_dds_data_writer_options_t dw_options;
    memset(&dw_options, 0, sizeof(dw_options));
    dw_options.topic = impl->topic;
    lwdistcomm_dds_qos_default(&dw_options.qos);

    impl->data_writer = lwdistcomm_dds_data_writer_create(impl->publisher, &dw_options);
    if (!impl->data_writer) {
        lwdistcomm_dds_publisher_delete(impl->publisher);
        lwdistcomm_dds_topic_delete(impl->topic);
        lwdistcomm_dds_domain_participant_delete(impl->participant);
        free(impl);
        free(publisher);
        return NULL;
    }

    // 启用DomainParticipant
    lwdistcomm_dds_domain_participant_enable(impl->participant);

    publisher->impl = impl;
    return publisher;
}

/**
 * 销毁一个简化的DDS发布者
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_publisher_destroy(lwdistcomm_dds_simple_publisher_t *publisher)
{
    if (!publisher || !publisher->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }

    lwdistcomm_dds_simple_publisher_impl_t *impl = (lwdistcomm_dds_simple_publisher_impl_t *)publisher->impl;

    // 销毁数据写入器
    if (impl->data_writer) {
        lwdistcomm_dds_data_writer_delete(impl->data_writer);
    }

    // 销毁发布者
    if (impl->publisher) {
        lwdistcomm_dds_publisher_delete(impl->publisher);
    }

    // 销毁主题
    if (impl->topic) {
        lwdistcomm_dds_topic_delete(impl->topic);
    }

    // 销毁域参与者
    if (impl->participant) {
        lwdistcomm_dds_domain_participant_delete(impl->participant);
    }

    free(impl);
    free(publisher);

    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 发布数据到主题
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_publish(lwdistcomm_dds_simple_publisher_t *publisher, const void *data, uint32_t size)
{
    if (!publisher || !publisher->impl || !data) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }

    lwdistcomm_dds_simple_publisher_impl_t *impl = (lwdistcomm_dds_simple_publisher_impl_t *)publisher->impl;
    if (!impl->data_writer) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }

    return lwdistcomm_dds_data_writer_write(impl->data_writer, data, size);
}

/**
 * 创建一个简化的DDS订阅者
 */
lwdistcomm_dds_simple_subscriber_t *lwdistcomm_dds_simple_subscriber_create(const lwdistcomm_dds_simple_topic_options_t *options, lwdistcomm_dds_simple_data_cb_t callback, void *arg)
{
    if (!options || !options->topic_name || !options->type_name || !callback) {
        return NULL;
    }

    lwdistcomm_dds_simple_subscriber_t *subscriber = (lwdistcomm_dds_simple_subscriber_t *)malloc(sizeof(lwdistcomm_dds_simple_subscriber_t));
    if (!subscriber) {
        return NULL;
    }

    lwdistcomm_dds_simple_subscriber_impl_t *impl = (lwdistcomm_dds_simple_subscriber_impl_t *)malloc(sizeof(lwdistcomm_dds_simple_subscriber_impl_t));
    if (!impl) {
        free(subscriber);
        return NULL;
    }

    memset(impl, 0, sizeof(lwdistcomm_dds_simple_subscriber_impl_t));
    impl->data_callback = callback;
    impl->callback_arg = arg;

    // 创建DomainParticipant
    lwdistcomm_dds_domain_participant_options_t dp_options;
    memset(&dp_options, 0, sizeof(dp_options));
    dp_options.domain_id = options->domain_id;
    dp_options.enable_automatic_discovery = true;
    dp_options.discovery_port = 7400;
    lwdistcomm_dds_qos_default(&dp_options.qos);

    impl->participant = lwdistcomm_dds_domain_participant_create(&dp_options);
    if (!impl->participant) {
        free(impl);
        free(subscriber);
        return NULL;
    }

    // 创建Topic
    lwdistcomm_dds_topic_options_t topic_options;
    memset(&topic_options, 0, sizeof(topic_options));
    topic_options.name = options->topic_name;
    topic_options.type_name = options->type_name;
    lwdistcomm_dds_qos_default(&topic_options.qos);

    impl->topic = lwdistcomm_dds_topic_create(impl->participant, &topic_options);
    if (!impl->topic) {
        lwdistcomm_dds_domain_participant_delete(impl->participant);
        free(impl);
        free(subscriber);
        return NULL;
    }

    // 创建Subscriber
    lwdistcomm_dds_subscriber_options_t sub_options;
    memset(&sub_options, 0, sizeof(sub_options));
    lwdistcomm_dds_qos_default(&sub_options.qos);

    impl->subscriber = lwdistcomm_dds_subscriber_create(impl->participant, &sub_options);
    if (!impl->subscriber) {
        lwdistcomm_dds_topic_delete(impl->topic);
        lwdistcomm_dds_domain_participant_delete(impl->participant);
        free(impl);
        free(subscriber);
        return NULL;
    }

    // 创建DataReader
    lwdistcomm_dds_data_reader_options_t dr_options;
    memset(&dr_options, 0, sizeof(dr_options));
    dr_options.topic = impl->topic;
    lwdistcomm_dds_qos_default(&dr_options.qos);

    impl->data_reader = lwdistcomm_dds_data_reader_create(impl->subscriber, &dr_options);
    if (!impl->data_reader) {
        lwdistcomm_dds_subscriber_delete(impl->subscriber);
        lwdistcomm_dds_topic_delete(impl->topic);
        lwdistcomm_dds_domain_participant_delete(impl->participant);
        free(impl);
        free(subscriber);
        return NULL;
    }

    // 设置数据可用回调
    lwdistcomm_dds_data_reader_set_data_available_callback(impl->data_reader, data_available_callback, impl);

    // 启用DomainParticipant
    lwdistcomm_dds_domain_participant_enable(impl->participant);

    subscriber->impl = impl;
    return subscriber;
}

/**
 * 销毁一个简化的DDS订阅者
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_subscriber_destroy(lwdistcomm_dds_simple_subscriber_t *subscriber)
{
    if (!subscriber || !subscriber->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }

    lwdistcomm_dds_simple_subscriber_impl_t *impl = (lwdistcomm_dds_simple_subscriber_impl_t *)subscriber->impl;

    // 销毁数据读取器
    if (impl->data_reader) {
        lwdistcomm_dds_data_reader_delete(impl->data_reader);
    }

    // 销毁订阅者
    if (impl->subscriber) {
        lwdistcomm_dds_subscriber_delete(impl->subscriber);
    }

    // 销毁主题
    if (impl->topic) {
        lwdistcomm_dds_topic_delete(impl->topic);
    }

    // 销毁域参与者
    if (impl->participant) {
        lwdistcomm_dds_domain_participant_delete(impl->participant);
    }

    free(impl);
    free(subscriber);

    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 处理订阅者事件（非阻塞）
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_simple_subscriber_process_events(lwdistcomm_dds_simple_subscriber_t *subscriber)
{
    if (!subscriber || !subscriber->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }

    // 这里可以添加事件处理逻辑
    // 由于我们使用了回调机制，这里可能不需要做什么

    return LWDISTCOMM_DDS_RETCODE_OK;
}
