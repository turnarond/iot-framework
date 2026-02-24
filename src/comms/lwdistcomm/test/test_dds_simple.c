#include "dds/dds_simple.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/**
 * 测试数据结构
 */
typedef struct {
    int id;
    char message[256];
} TestData;

/**
 * 订阅者数据回调函数
 */
static void data_callback(const void *data, uint32_t size, void *arg)
{
    if (size != sizeof(TestData)) {
        printf("Received data with incorrect size\n");
        return;
    }

    TestData *test_data = (TestData *)data;
    printf("Subscriber received: id=%d, message=%s\n", test_data->id, test_data->message);
}

/**
 * 发布者线程函数
 */
static void *publisher_thread(void *arg)
{
    lwdistcomm_dds_simple_topic_options_t options;
    options.topic_name = "TestTopic";
    options.type_name = "TestData";
    options.domain_id = 0;

    // 创建发布者
    lwdistcomm_dds_simple_publisher_t *publisher = lwdistcomm_dds_simple_publisher_create(&options);
    if (!publisher) {
        printf("Failed to create publisher\n");
        return NULL;
    }

    printf("Publisher created successfully\n");

    // 发布数据
    TestData data;
    for (int i = 0; i < 10; i++) {
        data.id = i;
        snprintf(data.message, sizeof(data.message), "Hello DDS from publisher! Count: %d", i);

        printf("Publishing: id=%d, message=%s\n", data.id, data.message);
        lwdistcomm_dds_simple_publish(publisher, &data, sizeof(data));

        sleep(1);
    }

    // 销毁发布者
    lwdistcomm_dds_simple_publisher_destroy(publisher);
    printf("Publisher destroyed\n");

    return NULL;
}

int main()
{
    printf("=== Simple DDS Test ===\n");

    // 创建订阅者
    lwdistcomm_dds_simple_topic_options_t options;
    options.topic_name = "TestTopic";
    options.type_name = "TestData";
    options.domain_id = 0;

    lwdistcomm_dds_simple_subscriber_t *subscriber = lwdistcomm_dds_simple_subscriber_create(&options, data_callback, NULL);
    if (!subscriber) {
        printf("Failed to create subscriber\n");
        return 1;
    }

    printf("Subscriber created successfully\n");

    // 创建并启动发布者线程
    pthread_t pub_thread;
    pthread_create(&pub_thread, NULL, publisher_thread, NULL);

    // 等待发布者完成
    pthread_join(pub_thread, NULL);

    // 等待一段时间以确保所有数据都被接收
    sleep(2);

    // 销毁订阅者
    lwdistcomm_dds_simple_subscriber_destroy(subscriber);
    printf("Subscriber destroyed\n");

    printf("=== Test completed ===\n");
    return 0;
}
