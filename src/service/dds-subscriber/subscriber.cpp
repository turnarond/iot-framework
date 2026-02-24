#include <lwdistcomm/client.h>
#include <lwdistcomm/address.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * 消息回调函数
 * 功能：处理接收到的订阅消息
 */
static void message_callback(void *arg, const char *url, const lwdistcomm_message_t *msg)
{
    if (!url || !msg) {
        return;
    }
    
    // 打印接收到的消息
    printf("Received message from topic %s: ", url);
    if (msg->data && msg->data_len > 0) {
        // 确保消息以null结尾
        char *msg_str = (char *)malloc(msg->data_len + 1);
        if (msg_str) {
            memcpy(msg_str, msg->data, msg->data_len);
            msg_str[msg->data_len] = '\0';
            printf("%s\n", msg_str);
            free(msg_str);
        }
    } else {
        printf("(empty message)\n");
    }
}

/**
 * DDS订阅者服务
 * 功能：连接到发布者并订阅指定主题的数据
 */

int main(int argc, char *argv[])
{
    printf("DDS Subscriber Service Starting...\n");
    
    // 创建客户端
    lwdistcomm_client_t *client = lwdistcomm_client_create(NULL);
    if (!client) {
        printf("Failed to create client\n");
        return 1;
    }
    printf("Created client\n");
    
    // 创建服务器地址（使用IPv4地址）
    lwdistcomm_address_t *server_addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
    if (!server_addr) {
        printf("Failed to create server address\n");
        lwdistcomm_client_destroy(client);
        return 1;
    }
    
    // 设置IPv4地址和端口（与发布者相同）
    const char *ip = "0.0.0.0";
    uint16_t port = 5555;
    if (!lwdistcomm_address_set_ipv4(server_addr, ip, port)) {
        printf("Failed to set IPv4 address\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_client_destroy(client);
        return 1;
    }
    printf("Connecting to publisher at: %s:%d\n", ip, port);
    
    // 连接到服务器
    printf("Attempting to connect to server...\n");
    printf("Server address: %s:%d\n", ip, port);
    
    // 尝试连接多次
    int attempts = 5;
    bool connected = false;
    
    for (int i = 0; i < attempts; i++) {
        printf("Connection attempt %d/%d...\n", i+1, attempts);
        if (lwdistcomm_client_connect(client, server_addr)) {
            connected = true;
            break;
        }
        printf("Connection attempt %d failed, retrying in 1 second...\n", i+1);
        sleep(1);
    }
    
    if (!connected) {
        printf("Failed to connect to server after %d attempts\n", attempts);
        printf("Check if publisher is running and IP address/port is correct\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_client_destroy(client);
        return 1;
    }
    printf("Connected to server\n");
    
    // 订阅主题
    const char *topic = "/dds/test/topic";
    if (!lwdistcomm_client_subscribe(client, topic, message_callback, NULL)) {
        printf("Failed to subscribe to topic: %s\n", topic);
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_client_destroy(client);
        return 1;
    }
    printf("Subscribed to topic: %s\n", topic);
    
    // 主循环，接收并处理数据
    while (true) {
        // 处理客户端事件
        if (!lwdistcomm_client_process_events(client)) {
            printf("Failed to process client events\n");
            sleep(1);
            continue;
        }
        
        // 检查是否有新数据
        // 注意：实际使用中，应该通过回调函数处理接收到的数据
        // 这里为了简化，我们只是打印消息并继续
        printf("Waiting for data...\n");
        
        // 休眠1秒
        sleep(1);
    }
    
    // 清理资源
    lwdistcomm_address_destroy(server_addr);
    lwdistcomm_client_destroy(client);
    
    return 0;
}
