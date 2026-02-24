#include "../include/server.h"
#include "../include/address.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * 全局变量，用于标记服务器是否崩溃
 */
static bool server_crashed = false;

/**
 * 信号处理函数，用于捕获SIGPIPE信号
 */
static void sigpipe_handler(int sig)
{
    printf("SIGPIPE signal received (signal %d)\n", sig);
    server_crashed = true;
}

/**
 * 测试服务器在客户端断开连接后的行为
 */
static bool test_server_sigpipe_handling(void)
{
    printf("\n=== Testing Server SIGPIPE Handling ===\n");
    
    // 注册SIGPIPE信号处理函数
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigpipe_handler;
    sigaction(SIGPIPE, &sa, NULL);
    
    // 创建服务器地址（使用Unix域套接字）
    lwdistcomm_address_t *server_addr = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_UNIX);
    if (!server_addr) {
        printf("Failed to create server address\n");
        return false;
    }
    
    // 设置Unix域套接字路径
    const char *socket_path = "/tmp/test_sigpipe.sock";
    if (!lwdistcomm_address_set_unix_path(server_addr, socket_path)) {
        printf("Failed to set unix socket path\n");
        lwdistcomm_address_destroy(server_addr);
        return false;
    }
    printf("Created server address: %s\n", socket_path);
    
    // 创建服务器
    lwdistcomm_server_t *server = lwdistcomm_server_create(NULL);
    if (!server) {
        printf("Failed to create server\n");
        lwdistcomm_address_destroy(server_addr);
        return false;
    }
    printf("Created server\n");
    
    // 启动服务器
    if (!lwdistcomm_server_start(server, server_addr)) {
        printf("Failed to start server\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return false;
    }
    printf("Started server on %s\n", socket_path);
    
    // 等待服务器完全启动
    sleep(1);
    
    // 检查服务器是否在运行
    if (!lwdistcomm_server_is_running(server)) {
        printf("Server is not running\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return false;
    }
    printf("Server is running\n");
    
    // 创建一个客户端套接字并连接到服务器
    int client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("socket");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return false;
    }
    
    // 连接到服务器
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    
    if (connect(client_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(client_sock);
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return false;
    }
    printf("Connected to server\n");
    
    // 立即关闭客户端套接字
    close(client_sock);
    printf("Closed client connection\n");
    
    // 等待一段时间，确保客户端连接已经关闭
    sleep(1);
    
    // 服务器处理事件
    lwdistcomm_server_process_events(server);
    printf("Processed server events\n");
    
    // 尝试向所有客户端发布数据（包括已关闭的）
    lwdistcomm_message_t msg;
    const char *data = "Test publish data";
    msg.data = (void *)data;
    msg.data_len = strlen(data);
    
    bool publish_result = lwdistcomm_server_publish(server, "/test/topic", &msg);
    printf("Published data: %s\n", publish_result ? "SUCCESS" : "FAILED");
    
    // 处理事件，尝试发送数据
    for (int i = 0; i < 5; i++) {
        lwdistcomm_server_process_events(server);
        usleep(100000); // 等待100ms
    }
    
    // 检查服务器是否崩溃
    if (server_crashed) {
        printf("Server SIGPIPE test FAILED: Server crashed\n");
        lwdistcomm_address_destroy(server_addr);
        lwdistcomm_server_destroy(server);
        return false;
    }
    
    // 清理资源
    lwdistcomm_address_destroy(server_addr);
    lwdistcomm_server_destroy(server);
    printf("Cleaned up resources\n");
    printf("Server SIGPIPE test PASSED\n");
    return true;
}

/**
 * 主函数
 */
int main(int argc, char *argv[])
{
    printf("SIGPIPE Signal Handling Test\n");
    printf("============================\n");
    
    bool server_test_passed = test_server_sigpipe_handling();
    
    printf("\n=== Test Summary ===\n");
    printf("Server SIGPIPE handling test: %s\n", server_test_passed ? "PASSED" : "FAILED");
    
    if (server_test_passed) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}
