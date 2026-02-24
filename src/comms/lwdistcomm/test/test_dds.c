#include "../include/dds/dds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * 测试数据结构
 */
typedef struct {
    int id;
    char message[64];
} test_data_t;

/**
 * 数据可用回调函数
 */
static void data_available_callback(lwdistcomm_dds_data_reader_t *reader, void *arg)
{
    if (!reader || !arg) {
        return;
    }
    
    test_data_t data;
    uint32_t size = sizeof(test_data_t);
    lwdistcomm_dds_sample_info_t info;
    
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_data_reader_take(reader, &data, &size, &info);
    if (ret == LWDISTCOMM_DDS_RETCODE_OK && info.valid_data) {
        printf("Received data: id=%d, message=%s\n", data.id, data.message);
        *((bool *)arg) = true;
    }
}

/**
 * 测试DDS基本功能
 */
static bool test_dds_basic_functionality(void)
{
    printf("\n=== Testing DDS Basic Functionality ===\n");
    
    /* 初始化QoS */
    lwdistcomm_dds_qos_t qos;
    lwdistcomm_dds_qos_default(&qos);
    
    /* 创建DomainParticipant选项 */
    lwdistcomm_dds_domain_participant_options_t dp_options = {
        .domain_id = 0,
        .qos = qos,
        .enable_automatic_discovery = true,
        .discovery_port = 7400
    };
    
    /* 创建DomainParticipant */
    lwdistcomm_dds_domain_participant_t *participant = lwdistcomm_dds_domain_participant_create(&dp_options);
    if (!participant) {
        printf("Failed to create DomainParticipant\n");
        return false;
    }
    printf("Created DomainParticipant\n");
    
    /* 启用DomainParticipant */
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_domain_participant_enable(participant);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to enable DomainParticipant\n");
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Enabled DomainParticipant\n");
    
    /* 创建Topic选项 */
    lwdistcomm_dds_topic_options_t topic_options = {
        .name = "TestTopic",
        .type_name = "test_data_t",
        .qos = qos
    };
    
    /* 创建Topic */
    lwdistcomm_dds_topic_t *topic = lwdistcomm_dds_topic_create(participant, &topic_options);
    if (!topic) {
        printf("Failed to create Topic\n");
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Created Topic: %s\n", topic->name);
    
    /* 创建Publisher选项 */
    lwdistcomm_dds_publisher_options_t publisher_options = {
        .qos = qos
    };
    
    /* 创建Publisher */
    lwdistcomm_dds_publisher_t *publisher = lwdistcomm_dds_publisher_create(participant, &publisher_options);
    if (!publisher) {
        printf("Failed to create Publisher\n");
        lwdistcomm_dds_topic_delete(topic);
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Created Publisher\n");
    
    /* 创建Subscriber选项 */
    lwdistcomm_dds_subscriber_options_t subscriber_options = {
        .qos = qos
    };
    
    /* 创建Subscriber */
    lwdistcomm_dds_subscriber_t *subscriber = lwdistcomm_dds_subscriber_create(participant, &subscriber_options);
    if (!subscriber) {
        printf("Failed to create Subscriber\n");
        lwdistcomm_dds_publisher_delete(publisher);
        lwdistcomm_dds_topic_delete(topic);
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Created Subscriber\n");
    
    /* 创建DataWriter选项 */
    lwdistcomm_dds_data_writer_options_t dw_options = {
        .topic = topic,
        .qos = qos
    };
    
    /* 创建DataWriter */
    lwdistcomm_dds_data_writer_t *data_writer = lwdistcomm_dds_data_writer_create(publisher, &dw_options);
    if (!data_writer) {
        printf("Failed to create DataWriter\n");
        lwdistcomm_dds_subscriber_delete(subscriber);
        lwdistcomm_dds_publisher_delete(publisher);
        lwdistcomm_dds_topic_delete(topic);
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Created DataWriter\n");
    
    /* 创建DataReader选项 */
    lwdistcomm_dds_data_reader_options_t dr_options = {
        .topic = topic,
        .qos = qos
    };
    
    /* 创建DataReader */
    lwdistcomm_dds_data_reader_t *data_reader = lwdistcomm_dds_data_reader_create(subscriber, &dr_options);
    if (!data_reader) {
        printf("Failed to create DataReader\n");
        lwdistcomm_dds_data_writer_delete(data_writer);
        lwdistcomm_dds_subscriber_delete(subscriber);
        lwdistcomm_dds_publisher_delete(publisher);
        lwdistcomm_dds_topic_delete(topic);
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Created DataReader\n");
    
    /* 设置数据可用回调 */
    bool data_received = false;
    ret = lwdistcomm_dds_data_reader_set_data_available_callback(data_reader, data_available_callback, &data_received);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to set data available callback\n");
        lwdistcomm_dds_data_reader_delete(data_reader);
        lwdistcomm_dds_data_writer_delete(data_writer);
        lwdistcomm_dds_subscriber_delete(subscriber);
        lwdistcomm_dds_publisher_delete(publisher);
        lwdistcomm_dds_topic_delete(topic);
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Set data available callback\n");
    
    /* 准备测试数据 */
    test_data_t test_data = {
        .id = 1,
        .message = "Hello DDS!"
    };
    
    /* 发送数据 */
    ret = lwdistcomm_dds_data_writer_write(data_writer, &test_data, sizeof(test_data_t));
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to write data\n");
        lwdistcomm_dds_data_reader_delete(data_reader);
        lwdistcomm_dds_data_writer_delete(data_writer);
        lwdistcomm_dds_subscriber_delete(subscriber);
        lwdistcomm_dds_publisher_delete(publisher);
        lwdistcomm_dds_topic_delete(topic);
        lwdistcomm_dds_domain_participant_delete(participant);
        return false;
    }
    printf("Sent data: id=%d, message=%s\n", test_data.id, test_data.message);
    
    /* 等待数据接收 */
    sleep(1);
    
    /* 等待发现机制完成 */
    sleep(1);
    
    /* 测试发现机制 */
    uint32_t discovered_participants = 0;
    ret = lwdistcomm_dds_domain_participant_get_discovered_participants(participant, &discovered_participants);
    if (ret == LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Discovered participants: %u\n", discovered_participants);
    }
    
    uint32_t discovered_topics = 0;
    ret = lwdistcomm_dds_domain_participant_get_discovered_topics(participant, &discovered_topics);
    if (ret == LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Discovered topics: %u\n", discovered_topics);
    }
    
    /* 清理资源 */
    lwdistcomm_dds_data_reader_delete(data_reader);
    lwdistcomm_dds_data_writer_delete(data_writer);
    lwdistcomm_dds_subscriber_delete(subscriber);
    lwdistcomm_dds_publisher_delete(publisher);
    lwdistcomm_dds_topic_delete(topic);
    lwdistcomm_dds_domain_participant_delete(participant);
    
    printf("Cleaned up resources\n");
    printf("DDS basic functionality test %s\n", data_received ? "PASSED" : "FAILED");
    
    return data_received;
}

/**
 * 测试DDS QoS策略
 */
static bool test_dds_qos(void)
{
    printf("\n=== Testing DDS QoS Policies ===\n");
    
    /* 初始化QoS */
    lwdistcomm_dds_qos_t qos;
    lwdistcomm_dds_qos_default(&qos);
    
    /* 测试可靠性策略 */
    lwdistcomm_dds_duration_t max_blocking_time = {1, 0};
    lwdistcomm_dds_retcode_t ret = lwdistcomm_dds_qos_set_reliability(&qos, LWDISTCOMM_DDS_RELIABILITY_RELIABLE, &max_blocking_time);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to set reliability QoS\n");
        return false;
    }
    printf("Set reliability QoS to RELIABLE\n");
    
    /* 测试持久性策略 */
    ret = lwdistcomm_dds_qos_set_durability(&qos, LWDISTCOMM_DDS_DURABILITY_TRANSIENT_LOCAL);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to set durability QoS\n");
        return false;
    }
    printf("Set durability QoS to TRANSIENT_LOCAL\n");
    
    /* 测试历史记录策略 */
    ret = lwdistcomm_dds_qos_set_history(&qos, LWDISTCOMM_DDS_HISTORY_KEEP_LAST, 10);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to set history QoS\n");
        return false;
    }
    printf("Set history QoS to KEEP_LAST with depth 10\n");
    
    /* 测试资源限制策略 */
    ret = lwdistcomm_dds_qos_set_resource_limits(&qos, 100, 10, 10);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to set resource limits QoS\n");
        return false;
    }
    printf("Set resource limits QoS\n");
    
    /* 测试QoS复制 */
    lwdistcomm_dds_qos_t qos_copy;
    ret = lwdistcomm_dds_qos_copy(&qos, &qos_copy);
    if (ret != LWDISTCOMM_DDS_RETCODE_OK) {
        printf("Failed to copy QoS\n");
        return false;
    }
    printf("Copied QoS\n");
    
    printf("DDS QoS policies test PASSED\n");
    
    return true;
}

/**
 * 主函数
 */
int main(int argc, char *argv[])
{
    printf("DDS Functionality Test\n");
    printf("====================\n");
    
    bool basic_test_passed = test_dds_basic_functionality();
    bool qos_test_passed = test_dds_qos();
    
    printf("\n=== Test Summary ===\n");
    printf("Basic functionality test: %s\n", basic_test_passed ? "PASSED" : "FAILED");
    printf("QoS policies test: %s\n", qos_test_passed ? "PASSED" : "FAILED");
    
    if (basic_test_passed && qos_test_passed) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}
