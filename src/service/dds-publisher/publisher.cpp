#include <lwdistcomm/server.h>
#include <lwdistcomm/address.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctime>

/**
 * DDS发布者服务
 * 功能：定期发布数据到指定主题
 */

int main(int argc, char *argv[])
{
    printf("DDS Publisher Service Starting...\n");
    
    // 创建服务器地址（使用IPv4地址）
    lwdistcomm_address_t *server_addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
    if (!server_addr) {
        printf("Failed to create server address\n");
        return 1;
    }
    
    // 设置IPv4地址和端口
    const char *ip = "127.0.0.1";
    uint16_t port = 5555;
    if (!lwdistcomm_address_set_ipv4(server_addr, ip, port)) {
        printf("Failed to set IPv4 address\n");
        lwdistcomm_address_destroy(server_addr);
        return 1;
    }
    printf("Created server address: %s:%d\n", ip, port);
    
    // 创建服务器
    lwdistcomm_server_t *server = lwdistcomm_server_create(NULL);
    if (!server) {
        printf("Failed to create server\n");
        lwdistcomm_address_destroy(server_addr);
        return 1;
    }
    printf("Created server\n");
    
    // 启动服务器
    if (!lwdistcomm_server_start(server, server_addr)) {
        printf("Failed to start server\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return 1;
    }
    printf("Started server on %s:%d\n", ip, port);
    
    // 检查服务器是否正在运行
    if (!lwdistcomm_server_is_running(server)) {
        printf("Server is not running\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return 1;
    }
    printf("Server is running\n");
    
    // 等待一段时间，确保服务器完全启动并准备好处理连接请求
    printf("Waiting for server to be ready...\n");
    sleep(2);
    
    // 再次检查服务器是否正在运行
    if (!lwdistcomm_server_is_running(server)) {
        printf("Server stopped running\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return 1;
    }
    printf("Server is ready to accept connections\n");
    
    // 发布计数器
    int counter = 0;
    
    // 定期发布数据
    time_t last_publish_time = time(NULL);
    while (true) {
        // 持续处理服务器事件
        if (!lwdistcomm_server_process_events(server)) {
            printf("Failed to process server events\n");
        }
        
        // 每1秒发布一次数据
        time_t current_time = time(NULL);
        if (current_time - last_publish_time >= 1) {
            // 准备发布数据
            lwdistcomm_message_t msg;
            char data[100];
            snprintf(data, sizeof(data), "Hello from publisher! Counter: %d", counter);
            msg.data = (void *)data;
            msg.data_len = strlen(data);
            
            // 发布数据到主题
            bool publish_result = lwdistcomm_server_publish(server, "/dds/test/topic", &msg);
            if (publish_result) {
                printf("Published data: %s\n", data);
            } else {
                printf("Failed to publish data\n");
            }
            
            // 增加计数器
            counter++;
            
            // 更新上次发布时间
            last_publish_time = current_time;
        }
        
        // 短暂休眠，避免CPU占用过高
        usleep(10000); // 10ms
    }
    
    // 清理资源
    lwdistcomm_address_destroy(server_addr);
    lwdistcomm_server_destroy(server);
    
    return 0;
}
