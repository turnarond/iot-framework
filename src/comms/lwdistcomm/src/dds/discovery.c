#include "domain_participant_impl.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define DISCOVERY_MSG_MAX_SIZE 1024
#define DISCOVERY_INTERVAL_MS 1000
#define DISCOVERY_PORT 7400

/**
 * 发现消息类型
 */
typedef enum {
    DISCOVERY_MSG_PARTICIPANT_ANNOUNCE,
    DISCOVERY_MSG_PARTICIPANT_LEAVE,
    DISCOVERY_MSG_TOPIC_ANNOUNCE,
    DISCOVERY_MSG_TOPIC_REMOVE,
    DISCOVERY_MSG_ENDPOINT_ANNOUNCE,
    DISCOVERY_MSG_ENDPOINT_REMOVE
} discovery_msg_type_t;

/**
 * 发现消息结构
 */
typedef struct {
    discovery_msg_type_t type;
    uint32_t domain_id;
    uint32_t participant_id;
    char participant_name[64];
    char topic_name[64];
    char type_name[64];
    uint32_t endpoint_id;
    uint8_t is_writer;
} discovery_msg_t;

/**
 * 序列化发现消息
 */
static int serialize_discovery_msg(const discovery_msg_t *msg, char *buffer, size_t buffer_size)
{
    if (!msg || !buffer) {
        return -1;
    }
    
    /* 简单的序列化实现，实际应用中可能需要更复杂的序列化方法 */
    int offset = 0;
    
    /* 写入消息类型 */
    memcpy(buffer + offset, &msg->type, sizeof(discovery_msg_type_t));
    offset += sizeof(discovery_msg_type_t);
    
    /* 写入域ID */
    memcpy(buffer + offset, &msg->domain_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 写入参与者ID */
    memcpy(buffer + offset, &msg->participant_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 写入参与者名称 */
    strncpy(buffer + offset, msg->participant_name, sizeof(msg->participant_name));
    offset += sizeof(msg->participant_name);
    
    /* 写入主题名称 */
    strncpy(buffer + offset, msg->topic_name, sizeof(msg->topic_name));
    offset += sizeof(msg->topic_name);
    
    /* 写入类型名称 */
    strncpy(buffer + offset, msg->type_name, sizeof(msg->type_name));
    offset += sizeof(msg->type_name);
    
    /* 写入端点ID */
    memcpy(buffer + offset, &msg->endpoint_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 写入是否为写入器 */
    memcpy(buffer + offset, &msg->is_writer, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    return offset;
}

/**
 * 反序列化发现消息
 */
static int deserialize_discovery_msg(const char *buffer, size_t buffer_size, discovery_msg_t *msg)
{
    if (!buffer || !msg) {
        return -1;
    }
    
    /* 简单的反序列化实现 */
    int offset = 0;
    
    /* 读取消息类型 */
    memcpy(&msg->type, buffer + offset, sizeof(discovery_msg_type_t));
    offset += sizeof(discovery_msg_type_t);
    
    /* 读取域ID */
    memcpy(&msg->domain_id, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 读取参与者ID */
    memcpy(&msg->participant_id, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 读取参与者名称 */
    strncpy(msg->participant_name, buffer + offset, sizeof(msg->participant_name));
    offset += sizeof(msg->participant_name);
    
    /* 读取主题名称 */
    strncpy(msg->topic_name, buffer + offset, sizeof(msg->topic_name));
    offset += sizeof(msg->topic_name);
    
    /* 读取类型名称 */
    strncpy(msg->type_name, buffer + offset, sizeof(msg->type_name));
    offset += sizeof(msg->type_name);
    
    /* 读取端点ID */
    memcpy(&msg->endpoint_id, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    /* 读取是否为写入器 */
    memcpy(&msg->is_writer, buffer + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    return offset;
}

/**
 * 发送发现消息
 */
static int send_discovery_msg(int sockfd, const discovery_msg_t *msg, const struct sockaddr_in *addr)
{
    if (!msg || !addr) {
        return -1;
    }
    
    char buffer[DISCOVERY_MSG_MAX_SIZE];
    int msg_size = serialize_discovery_msg(msg, buffer, sizeof(buffer));
    if (msg_size < 0) {
        return -1;
    }
    
    return sendto(sockfd, buffer, msg_size, 0, (struct sockaddr *)addr, sizeof(*addr));
}

/**
 * 处理参与者宣告消息
 */
static void handle_participant_announce(lwdistcomm_dds_domain_participant_impl_t *impl, const discovery_msg_t *msg)
{
    if (!impl || !msg) {
        return;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 检查是否已经发现过该参与者 */
    for (uint32_t i = 0; i < impl->num_discovered_participants; i++) {
        if (impl->discovered_participants[i].handle == msg->participant_id) {
            /* 已经发现过，更新信息 */
            free(impl->discovered_participants[i].name);
            impl->discovered_participants[i].name = strdup(msg->participant_name);
            pthread_mutex_unlock(&impl->mutex);
            return;
        }
    }
    
    /* 检查是否有足够的空间 */
    if (impl->num_discovered_participants >= impl->max_discovered_participants) {
        pthread_mutex_unlock(&impl->mutex);
        return;
    }
    
    /* 添加新发现的参与者 */
    impl->discovered_participants[impl->num_discovered_participants].handle = msg->participant_id;
    impl->discovered_participants[impl->num_discovered_participants].name = strdup(msg->participant_name);
    impl->discovered_participants[impl->num_discovered_participants].address = NULL; /* 可以从消息中获取地址信息 */
    impl->num_discovered_participants++;
    
    pthread_mutex_unlock(&impl->mutex);
}

/**
 * 处理参与者离开消息
 */
static void handle_participant_leave(lwdistcomm_dds_domain_participant_impl_t *impl, const discovery_msg_t *msg)
{
    if (!impl || !msg) {
        return;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 查找并移除离开的参与者 */
    for (uint32_t i = 0; i < impl->num_discovered_participants; i++) {
        if (impl->discovered_participants[i].handle == msg->participant_id) {
            /* 释放资源 */
            free(impl->discovered_participants[i].name);
            if (impl->discovered_participants[i].address) {
                free(impl->discovered_participants[i].address);
            }
            
            /* 移除参与者 */
            impl->num_discovered_participants--;
            if (i < impl->num_discovered_participants) {
                impl->discovered_participants[i] = impl->discovered_participants[impl->num_discovered_participants];
            }
            pthread_mutex_unlock(&impl->mutex);
            return;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
}

/**
 * 处理主题宣告消息
 */
static void handle_topic_announce(lwdistcomm_dds_domain_participant_impl_t *impl, const discovery_msg_t *msg)
{
    if (!impl || !msg) {
        return;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 检查是否已经发现过该主题 */
    for (uint32_t i = 0; i < impl->num_discovered_topics; i++) {
        if (strcmp(impl->discovered_topics[i].name, msg->topic_name) == 0) {
            /* 已经发现过，更新信息 */
            free(impl->discovered_topics[i].type_name);
            impl->discovered_topics[i].type_name = strdup(msg->type_name);
            pthread_mutex_unlock(&impl->mutex);
            return;
        }
    }
    
    /* 检查是否有足够的空间 */
    if (impl->num_discovered_topics >= impl->max_discovered_topics) {
        pthread_mutex_unlock(&impl->mutex);
        return;
    }
    
    /* 添加新发现的主题 */
    impl->discovered_topics[impl->num_discovered_topics].handle = msg->participant_id;
    impl->discovered_topics[impl->num_discovered_topics].name = strdup(msg->topic_name);
    impl->discovered_topics[impl->num_discovered_topics].type_name = strdup(msg->type_name);
    impl->num_discovered_topics++;
    
    pthread_mutex_unlock(&impl->mutex);
}

/**
 * 处理主题移除消息
 */
static void handle_topic_remove(lwdistcomm_dds_domain_participant_impl_t *impl, const discovery_msg_t *msg)
{
    if (!impl || !msg) {
        return;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 查找并移除主题 */
    for (uint32_t i = 0; i < impl->num_discovered_topics; i++) {
        if (strcmp(impl->discovered_topics[i].name, msg->topic_name) == 0) {
            /* 释放资源 */
            free(impl->discovered_topics[i].name);
            free(impl->discovered_topics[i].type_name);
            
            /* 移除主题 */
            impl->num_discovered_topics--;
            if (i < impl->num_discovered_topics) {
                impl->discovered_topics[i] = impl->discovered_topics[impl->num_discovered_topics];
            }
            pthread_mutex_unlock(&impl->mutex);
            return;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
}

/**
 * 发现消息处理函数
 */
static void process_discovery_msg(lwdistcomm_dds_domain_participant_impl_t *impl, const char *buffer, size_t size, const struct sockaddr_in *addr)
{
    if (!impl || !buffer) {
        return;
    }
    
    discovery_msg_t msg;
    if (deserialize_discovery_msg(buffer, size, &msg) < 0) {
        return;
    }
    
    /* 检查域ID是否匹配 */
    if (msg.domain_id != impl->domain_id) {
        return;
    }
    
    /* 根据消息类型处理 */
    switch (msg.type) {
        case DISCOVERY_MSG_PARTICIPANT_ANNOUNCE:
            handle_participant_announce(impl, &msg);
            break;
        case DISCOVERY_MSG_PARTICIPANT_LEAVE:
            handle_participant_leave(impl, &msg);
            break;
        case DISCOVERY_MSG_TOPIC_ANNOUNCE:
            handle_topic_announce(impl, &msg);
            break;
        case DISCOVERY_MSG_TOPIC_REMOVE:
            handle_topic_remove(impl, &msg);
            break;
        case DISCOVERY_MSG_ENDPOINT_ANNOUNCE:
        case DISCOVERY_MSG_ENDPOINT_REMOVE:
            /* TODO: 处理端点宣告和移除消息 */
            break;
        default:
            break;
    }
}

/**
 * 发现线程函数（更新版本）
 */
void *lwdistcomm_dds_discovery_thread_func(void *arg)
{
    lwdistcomm_dds_domain_participant_impl_t *impl = (lwdistcomm_dds_domain_participant_impl_t *)arg;
    int sockfd;
    struct sockaddr_in addr;
    char buffer[DISCOVERY_MSG_MAX_SIZE];
    socklen_t addr_len;
    
    /* 创建UDP套接字 */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return NULL;
    }
    
    /* 设置地址重用 */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* 设置广播权限 */
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    
    /* 绑定地址 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(impl->discovery_port);
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return NULL;
    }
    
    impl->discovery_socket = sockfd;
    impl->discovery_thread_running = true;
    
    /* 发送参与者宣告消息 */
    discovery_msg_t announce_msg;
    memset(&announce_msg, 0, sizeof(announce_msg));
    announce_msg.type = DISCOVERY_MSG_PARTICIPANT_ANNOUNCE;
    announce_msg.domain_id = impl->domain_id;
    announce_msg.participant_id = impl->domain_id; /* 使用域ID作为临时参与者ID */
    strncpy(announce_msg.participant_name, "Participant", sizeof(announce_msg.participant_name));
    
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(impl->discovery_port);
    
    /* 发送广播消息 */
    send_discovery_msg(sockfd, &announce_msg, &broadcast_addr);
    
    /* 主循环 */
    while (impl->discovery_thread_running) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sockfd, &read_set);
        
        struct timeval timeout;
        timeout.tv_sec = DISCOVERY_INTERVAL_MS / 1000;
        timeout.tv_usec = (DISCOVERY_INTERVAL_MS % 1000) * 1000;
        
        int ret = select(sockfd + 1, &read_set, NULL, NULL, &timeout);
        if (ret < 0) {
            continue;
        }
        
        if (ret > 0 && FD_ISSET(sockfd, &read_set)) {
            addr_len = sizeof(addr);
            int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
            if (n > 0) {
                process_discovery_msg(impl, buffer, n, &addr);
            }
        } else {
            /* 定期发送参与者宣告消息 */
            send_discovery_msg(sockfd, &announce_msg, &broadcast_addr);
        }
    }
    
    /* 发送参与者离开消息 */
    discovery_msg_t leave_msg;
    memset(&leave_msg, 0, sizeof(leave_msg));
    leave_msg.type = DISCOVERY_MSG_PARTICIPANT_LEAVE;
    leave_msg.domain_id = impl->domain_id;
    leave_msg.participant_id = impl->domain_id;
    send_discovery_msg(sockfd, &leave_msg, &broadcast_addr);
    
    close(sockfd);
    impl->discovery_socket = -1;
    
    return NULL;
}

/**
 * 启动发现机制
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_start_discovery(lwdistcomm_dds_domain_participant_t *participant)
{
    if (!participant || !participant->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    lwdistcomm_dds_domain_participant_impl_t *impl = participant->impl;
    
    if (!impl->automatic_discovery) {
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    if (impl->discovery_thread_running) {
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    int ret = pthread_create(&impl->discovery_thread, NULL, lwdistcomm_dds_discovery_thread_func, impl);
    if (ret != 0) {
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 停止发现机制
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_stop_discovery(lwdistcomm_dds_domain_participant_t *participant)
{
    if (!participant || !participant->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    lwdistcomm_dds_domain_participant_impl_t *impl = participant->impl;
    
    if (!impl->discovery_thread_running) {
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    impl->discovery_thread_running = false;
    pthread_join(impl->discovery_thread, NULL);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
