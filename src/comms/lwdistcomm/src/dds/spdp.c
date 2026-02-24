#include "../../include/dds/spdp.h"
#include "domain_participant_impl.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define SPDP_MAGIC "SPDP"
#define SPDP_VERSION 1
#define SPDP_DEFAULT_MULTICAST_ADDRESS "239.255.0.1"
#define SPDP_DEFAULT_MULTICAST_PORT 7400
#define SPDP_DEFAULT_ANNOUNCE_INTERVAL_SEC 3
#define SPDP_DEFAULT_LEASE_DURATION_SEC 10
#define SPDP_MAX_MESSAGE_SIZE 4096
#define SPDP_MAX_DISCOVERED_PARTICIPANTS 128
#define SPDP_MAX_DISCOVERED_TOPICS 256

/**
 * SPDP内部实现结构
 */
struct lwdistcomm_dds_spdp_impl_s {
    lwdistcomm_dds_spdp_config_t config;
    lwdistcomm_dds_domain_participant_t *participant;
    int socket_fd;
    struct sockaddr_in multicast_addr;
    struct sockaddr_in local_addr;
    pthread_t thread;
    bool running;
    bool thread_running;
    pthread_mutex_t mutex;
    
    /* 已发现的参与者 */
    struct spdp_discovered_participant {
        lwdistcomm_dds_spdp_participant_info_t info;
        time_t last_seen;
        char address[128];
        uint16_t port;
    } discovered_participants[SPDP_MAX_DISCOVERED_PARTICIPANTS];
    uint32_t num_discovered_participants;
    
    /* 已发现的主题 */
    struct spdp_discovered_topic {
        lwdistcomm_dds_spdp_topic_info_t info;
        time_t last_seen;
        uint32_t participant_id;
    } discovered_topics[SPDP_MAX_DISCOVERED_TOPICS];
    uint32_t num_discovered_topics;
    
    /* 回调函数 */
    lwdistcomm_dds_spdp_participant_callback_t participant_callback;
    void *participant_callback_arg;
    lwdistcomm_dds_spdp_topic_callback_t topic_callback;
    void *topic_callback_arg;
};

/**
 * 获取当前时间戳
 */
static void get_current_timestamp(uint32_t *sec, uint32_t *nsec)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    *sec = (uint32_t)ts.tv_sec;
    *nsec = (uint32_t)ts.tv_nsec;
}

/**
 * 序列化SPDP消息
 */
static int serialize_spdp_message(const lwdistcomm_dds_spdp_message_t *msg, char *buffer, size_t buffer_size)
{
    if (!msg || !buffer) {
        return -1;
    }
    
    int offset = 0;
    
    /* 序列化魔术字 */
    memcpy(buffer + offset, SPDP_MAGIC, 4);
    offset += 4;
    
    /* 序列化版本号 */
    uint16_t version = htons(SPDP_VERSION);
    memcpy(buffer + offset, &version, sizeof(version));
    offset += sizeof(version);
    
    /* 序列化消息长度（暂时设为0，后续会更新） */
    uint16_t message_length = 0;
    memcpy(buffer + offset, &message_length, sizeof(message_length));
    offset += sizeof(message_length);
    
    /* 序列化消息类型 */
    uint16_t message_type = htons(msg->header.message_type);
    memcpy(buffer + offset, &message_type, sizeof(message_type));
    offset += sizeof(message_type);
    
    /* 序列化域ID */
    uint32_t domain_id = htonl(msg->header.domain_id);
    memcpy(buffer + offset, &domain_id, sizeof(domain_id));
    offset += sizeof(domain_id);
    
    /* 序列化参与者ID */
    uint32_t participant_id = htonl(msg->header.participant_id);
    memcpy(buffer + offset, &participant_id, sizeof(participant_id));
    offset += sizeof(participant_id);
    
    /* 序列化时间戳 */
    uint32_t timestamp_sec = htonl(msg->header.timestamp_sec);
    memcpy(buffer + offset, &timestamp_sec, sizeof(timestamp_sec));
    offset += sizeof(timestamp_sec);
    
    uint32_t timestamp_nsec = htonl(msg->header.timestamp_nsec);
    memcpy(buffer + offset, &timestamp_nsec, sizeof(timestamp_nsec));
    offset += sizeof(timestamp_nsec);
    
    /* 序列化参与者信息 */
    memcpy(buffer + offset, msg->participant.participant_name, 64);
    offset += 64;
    
    uint32_t protocol_version = htonl(msg->participant.protocol_version);
    memcpy(buffer + offset, &protocol_version, sizeof(protocol_version));
    offset += sizeof(protocol_version);
    
    uint32_t vendor_id = htonl(msg->participant.vendor_id);
    memcpy(buffer + offset, &vendor_id, sizeof(vendor_id));
    offset += sizeof(vendor_id);
    
    uint32_t product_id = htonl(msg->participant.product_id);
    memcpy(buffer + offset, &product_id, sizeof(product_id));
    offset += sizeof(product_id);
    
    uint32_t host_id = htonl(msg->participant.host_id);
    memcpy(buffer + offset, &host_id, sizeof(host_id));
    offset += sizeof(host_id);
    
    uint32_t application_id = htonl(msg->participant.application_id);
    memcpy(buffer + offset, &application_id, sizeof(application_id));
    offset += sizeof(application_id);
    
    uint32_t instance_id = htonl(msg->participant.instance_id);
    memcpy(buffer + offset, &instance_id, sizeof(instance_id));
    offset += sizeof(instance_id);
    
    uint32_t lease_duration_sec = htonl(msg->participant.lease_duration_sec);
    memcpy(buffer + offset, &lease_duration_sec, sizeof(lease_duration_sec));
    offset += sizeof(lease_duration_sec);
    
    uint32_t lease_duration_nsec = htonl(msg->participant.lease_duration_nsec);
    memcpy(buffer + offset, &lease_duration_nsec, sizeof(lease_duration_nsec));
    offset += sizeof(lease_duration_nsec);
    
    /* 序列化主题信息 */
    memcpy(buffer + offset, msg->topic.topic_name, 64);
    offset += 64;
    
    memcpy(buffer + offset, msg->topic.type_name, 64);
    offset += 64;
    
    uint32_t topic_id = htonl(msg->topic.topic_id);
    memcpy(buffer + offset, &topic_id, sizeof(topic_id));
    offset += sizeof(topic_id);
    
    /* 序列化端点信息 */
    uint32_t endpoint_id = htonl(msg->endpoint.endpoint_id);
    memcpy(buffer + offset, &endpoint_id, sizeof(endpoint_id));
    offset += sizeof(endpoint_id);
    
    memcpy(buffer + offset, &msg->endpoint.is_writer, sizeof(msg->endpoint.is_writer));
    offset += sizeof(msg->endpoint.is_writer);
    
    memcpy(buffer + offset, msg->endpoint.transport_address, 128);
    offset += 128;
    
    uint16_t port = htons(msg->endpoint.port);
    memcpy(buffer + offset, &port, sizeof(port));
    offset += sizeof(port);
    
    /* 序列化主题数量 */
    uint32_t num_topics = htonl(msg->num_topics);
    memcpy(buffer + offset, &num_topics, sizeof(num_topics));
    offset += sizeof(num_topics);
    
    /* 序列化主题列表 */
    for (uint32_t i = 0; i < msg->num_topics && i < 16; i++) {
        memcpy(buffer + offset, msg->topics[i].topic_name, 64);
        offset += 64;
        
        memcpy(buffer + offset, msg->topics[i].type_name, 64);
        offset += 64;
        
        uint32_t topic_id = htonl(msg->topics[i].topic_id);
        memcpy(buffer + offset, &topic_id, sizeof(topic_id));
        offset += sizeof(topic_id);
    }
    
    /* 更新消息长度 */
    message_length = htons(offset);
    memcpy(buffer + 6, &message_length, sizeof(message_length));
    
    return offset;
}

/**
 * 反序列化SPDP消息
 */
static int deserialize_spdp_message(const char *buffer, size_t buffer_size, lwdistcomm_dds_spdp_message_t *msg)
{
    if (!buffer || !msg || buffer_size < sizeof(lwdistcomm_dds_spdp_header_t)) {
        return -1;
    }
    
    int offset = 0;
    
    /* 验证魔术字 */
    if (memcmp(buffer + offset, SPDP_MAGIC, 4) != 0) {
        return -1;
    }
    offset += 4;
    
    /* 反序列化版本号 */
    uint16_t version;
    memcpy(&version, buffer + offset, sizeof(version));
    version = ntohs(version);
    if (version != SPDP_VERSION) {
        return -1;
    }
    msg->header.version = version;
    offset += sizeof(version);
    
    /* 反序列化消息长度 */
    uint16_t message_length;
    memcpy(&message_length, buffer + offset, sizeof(message_length));
    msg->header.message_length = ntohs(message_length);
    offset += sizeof(message_length);
    
    /* 反序列化消息类型 */
    uint16_t message_type;
    memcpy(&message_type, buffer + offset, sizeof(message_type));
    msg->header.message_type = (lwdistcomm_dds_spdp_message_type_t)ntohs(message_type);
    offset += sizeof(message_type);
    
    /* 反序列化域ID */
    uint32_t domain_id;
    memcpy(&domain_id, buffer + offset, sizeof(domain_id));
    msg->header.domain_id = ntohl(domain_id);
    offset += sizeof(domain_id);
    
    /* 反序列化参与者ID */
    uint32_t participant_id;
    memcpy(&participant_id, buffer + offset, sizeof(participant_id));
    msg->header.participant_id = ntohl(participant_id);
    offset += sizeof(participant_id);
    
    /* 反序列化时间戳 */
    uint32_t timestamp_sec;
    memcpy(&timestamp_sec, buffer + offset, sizeof(timestamp_sec));
    msg->header.timestamp_sec = ntohl(timestamp_sec);
    offset += sizeof(timestamp_sec);
    
    uint32_t timestamp_nsec;
    memcpy(&timestamp_nsec, buffer + offset, sizeof(timestamp_nsec));
    msg->header.timestamp_nsec = ntohl(timestamp_nsec);
    offset += sizeof(timestamp_nsec);
    
    /* 反序列化参与者信息 */
    memcpy(msg->participant.participant_name, buffer + offset, 64);
    offset += 64;
    
    uint32_t protocol_version;
    memcpy(&protocol_version, buffer + offset, sizeof(protocol_version));
    msg->participant.protocol_version = ntohl(protocol_version);
    offset += sizeof(protocol_version);
    
    uint32_t vendor_id;
    memcpy(&vendor_id, buffer + offset, sizeof(vendor_id));
    msg->participant.vendor_id = ntohl(vendor_id);
    offset += sizeof(vendor_id);
    
    uint32_t product_id;
    memcpy(&product_id, buffer + offset, sizeof(product_id));
    msg->participant.product_id = ntohl(product_id);
    offset += sizeof(product_id);
    
    uint32_t host_id;
    memcpy(&host_id, buffer + offset, sizeof(host_id));
    msg->participant.host_id = ntohl(host_id);
    offset += sizeof(host_id);
    
    uint32_t application_id;
    memcpy(&application_id, buffer + offset, sizeof(application_id));
    msg->participant.application_id = ntohl(application_id);
    offset += sizeof(application_id);
    
    uint32_t instance_id;
    memcpy(&instance_id, buffer + offset, sizeof(instance_id));
    msg->participant.instance_id = ntohl(instance_id);
    offset += sizeof(instance_id);
    
    uint32_t lease_duration_sec;
    memcpy(&lease_duration_sec, buffer + offset, sizeof(lease_duration_sec));
    msg->participant.lease_duration_sec = ntohl(lease_duration_sec);
    offset += sizeof(lease_duration_sec);
    
    uint32_t lease_duration_nsec;
    memcpy(&lease_duration_nsec, buffer + offset, sizeof(lease_duration_nsec));
    msg->participant.lease_duration_nsec = ntohl(lease_duration_nsec);
    offset += sizeof(lease_duration_nsec);
    
    /* 反序列化主题信息 */
    memcpy(msg->topic.topic_name, buffer + offset, 64);
    offset += 64;
    
    memcpy(msg->topic.type_name, buffer + offset, 64);
    offset += 64;
    
    uint32_t topic_id;
    memcpy(&topic_id, buffer + offset, sizeof(topic_id));
    msg->topic.topic_id = ntohl(topic_id);
    offset += sizeof(topic_id);
    
    /* 反序列化端点信息 */
    uint32_t endpoint_id;
    memcpy(&endpoint_id, buffer + offset, sizeof(endpoint_id));
    msg->endpoint.endpoint_id = ntohl(endpoint_id);
    offset += sizeof(endpoint_id);
    
    memcpy(&msg->endpoint.is_writer, buffer + offset, sizeof(msg->endpoint.is_writer));
    offset += sizeof(msg->endpoint.is_writer);
    
    memcpy(msg->endpoint.transport_address, buffer + offset, 128);
    offset += 128;
    
    uint16_t port;
    memcpy(&port, buffer + offset, sizeof(port));
    msg->endpoint.port = ntohs(port);
    offset += sizeof(port);
    
    /* 反序列化主题数量 */
    uint32_t num_topics;
    memcpy(&num_topics, buffer + offset, sizeof(num_topics));
    msg->num_topics = ntohl(num_topics);
    offset += sizeof(num_topics);
    
    /* 反序列化主题列表 */
    if (msg->num_topics > 0) {
        msg->topics = (lwdistcomm_dds_spdp_topic_info_t *)malloc(sizeof(lwdistcomm_dds_spdp_topic_info_t) * msg->num_topics);
        if (!msg->topics) {
            return -1;
        }
        
        for (uint32_t i = 0; i < msg->num_topics && i < 16; i++) {
            memcpy(msg->topics[i].topic_name, buffer + offset, 64);
            offset += 64;
            
            memcpy(msg->topics[i].type_name, buffer + offset, 64);
            offset += 64;
            
            uint32_t t_id;
            memcpy(&t_id, buffer + offset, sizeof(t_id));
            msg->topics[i].topic_id = ntohl(t_id);
            offset += sizeof(t_id);
        }
    }
    
    return offset;
}

/**
 * 发送SPDP消息
 */
static int send_spdp_message(struct lwdistcomm_dds_spdp_impl_s *impl, const lwdistcomm_dds_spdp_message_t *msg)
{
    char buffer[SPDP_MAX_MESSAGE_SIZE];
    int message_length = serialize_spdp_message(msg, buffer, sizeof(buffer));
    if (message_length < 0) {
        return -1;
    }
    
    int ret = sendto(
        impl->socket_fd,
        buffer,
        message_length,
        0,
        (struct sockaddr *)&impl->multicast_addr,
        sizeof(impl->multicast_addr)
    );
    
    return ret;
}

/**
 * 处理接收到的SPDP消息
 */
static void process_spdp_message(struct lwdistcomm_dds_spdp_impl_s *impl, const char *buffer, size_t size, const struct sockaddr_in *sender_addr)
{
    lwdistcomm_dds_spdp_message_t msg;
    memset(&msg, 0, sizeof(msg));
    
    int ret = deserialize_spdp_message(buffer, size, &msg);
    if (ret < 0) {
        return;
    }
    
    /* 检查域ID是否匹配 */
    if (msg.header.domain_id != impl->config.domain_id) {
        goto cleanup;
    }
    
    /* 检查是否是自己发送的消息 */
    if (msg.header.participant_id == impl->config.participant_id) {
        goto cleanup;
    }
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 处理参与者宣告消息 */
    if (msg.header.message_type == LWDISTCOMM_DDS_SPDP_MESSAGE_PARTICIPANT_ANNOUNCE) {
        /* 检查是否已经发现过该参与者 */
        bool found = false;
        for (uint32_t i = 0; i < impl->num_discovered_participants; i++) {
            if (impl->discovered_participants[i].info.protocol_version == msg.participant.protocol_version &&
                impl->discovered_participants[i].info.host_id == msg.participant.host_id &&
                impl->discovered_participants[i].info.application_id == msg.participant.application_id &&
                impl->discovered_participants[i].info.instance_id == msg.participant.instance_id) {
                /* 更新参与者信息 */
                memcpy(&impl->discovered_participants[i].info, &msg.participant, sizeof(msg.participant));
                impl->discovered_participants[i].last_seen = time(NULL);
                inet_ntop(AF_INET, &sender_addr->sin_addr, impl->discovered_participants[i].address, sizeof(impl->discovered_participants[i].address));
                impl->discovered_participants[i].port = msg.endpoint.port;
                found = true;
                break;
            }
        }
        
        if (!found && impl->num_discovered_participants < SPDP_MAX_DISCOVERED_PARTICIPANTS) {
            /* 添加新参与者 */
            memcpy(&impl->discovered_participants[impl->num_discovered_participants].info, &msg.participant, sizeof(msg.participant));
            impl->discovered_participants[impl->num_discovered_participants].last_seen = time(NULL);
            inet_ntop(AF_INET, &sender_addr->sin_addr, impl->discovered_participants[impl->num_discovered_participants].address, sizeof(impl->discovered_participants[impl->num_discovered_participants].address));
            impl->discovered_participants[impl->num_discovered_participants].port = msg.endpoint.port;
            impl->num_discovered_participants++;
            
            /* 调用参与者回调 */
            if (impl->participant_callback) {
                impl->participant_callback(impl->participant, &msg.participant, true, impl->participant_callback_arg);
            }
        }
    }
    
    /* 处理主题宣告消息 */
    if (msg.header.message_type == LWDISTCOMM_DDS_SPDP_MESSAGE_TOPIC_ANNOUNCE) {
        /* 检查是否已经发现过该主题 */
        bool found = false;
        for (uint32_t i = 0; i < impl->num_discovered_topics; i++) {
            if (strcmp(impl->discovered_topics[i].info.topic_name, msg.topic.topic_name) == 0 &&
                impl->discovered_topics[i].participant_id == msg.header.participant_id) {
                /* 更新主题信息 */
                memcpy(&impl->discovered_topics[i].info, &msg.topic, sizeof(msg.topic));
                impl->discovered_topics[i].last_seen = time(NULL);
                found = true;
                break;
            }
        }
        
        if (!found && impl->num_discovered_topics < SPDP_MAX_DISCOVERED_TOPICS) {
            /* 添加新主题 */
            memcpy(&impl->discovered_topics[impl->num_discovered_topics].info, &msg.topic, sizeof(msg.topic));
            impl->discovered_topics[impl->num_discovered_topics].last_seen = time(NULL);
            impl->discovered_topics[impl->num_discovered_topics].participant_id = msg.header.participant_id;
            impl->num_discovered_topics++;
            
            /* 调用主题回调 */
            if (impl->topic_callback) {
                impl->topic_callback(impl->participant, &msg.topic, true, impl->topic_callback_arg);
            }
        }
        
        /* 处理主题列表 */
        for (uint32_t i = 0; i < msg.num_topics && msg.topics; i++) {
            bool topic_found = false;
            for (uint32_t j = 0; j < impl->num_discovered_topics; j++) {
                if (strcmp(impl->discovered_topics[j].info.topic_name, msg.topics[i].topic_name) == 0 &&
                    impl->discovered_topics[j].participant_id == msg.header.participant_id) {
                    /* 更新主题信息 */
                    memcpy(&impl->discovered_topics[j].info, &msg.topics[i], sizeof(msg.topics[i]));
                    impl->discovered_topics[j].last_seen = time(NULL);
                    topic_found = true;
                    break;
                }
            }
            
            if (!topic_found && impl->num_discovered_topics < SPDP_MAX_DISCOVERED_TOPICS) {
                /* 添加新主题 */
                memcpy(&impl->discovered_topics[impl->num_discovered_topics].info, &msg.topics[i], sizeof(msg.topics[i]));
                impl->discovered_topics[impl->num_discovered_topics].last_seen = time(NULL);
                impl->discovered_topics[impl->num_discovered_topics].participant_id = msg.header.participant_id;
                impl->num_discovered_topics++;
                
                /* 调用主题回调 */
                if (impl->topic_callback) {
                    impl->topic_callback(impl->participant, &msg.topics[i], true, impl->topic_callback_arg);
                }
            }
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
cleanup:
    if (msg.topics) {
        free(msg.topics);
    }
}

/**
 * 清理过期的参与者和主题
 */
static void cleanup_expired_entries(struct lwdistcomm_dds_spdp_impl_s *impl)
{
    time_t now = time(NULL);
    
    pthread_mutex_lock(&impl->mutex);
    
    /* 清理过期的参与者 */
    for (uint32_t i = 0; i < impl->num_discovered_participants; i++) {
        if (now - impl->discovered_participants[i].last_seen > SPDP_DEFAULT_LEASE_DURATION_SEC) {
            /* 移除过期的参与者 */
            if (i < impl->num_discovered_participants - 1) {
                memcpy(&impl->discovered_participants[i], &impl->discovered_participants[impl->num_discovered_participants - 1], sizeof(impl->discovered_participants[0]));
            }
            impl->num_discovered_participants--;
            i--;
        }
    }
    
    /* 清理过期的主题 */
    for (uint32_t i = 0; i < impl->num_discovered_topics; i++) {
        if (now - impl->discovered_topics[i].last_seen > SPDP_DEFAULT_LEASE_DURATION_SEC) {
            /* 移除过期的主题 */
            if (i < impl->num_discovered_topics - 1) {
                memcpy(&impl->discovered_topics[i], &impl->discovered_topics[impl->num_discovered_topics - 1], sizeof(impl->discovered_topics[0]));
            }
            impl->num_discovered_topics--;
            i--;
        }
    }
    
    pthread_mutex_unlock(&impl->mutex);
}

/**
 * 构建参与者宣告消息
 */
static void build_participant_announce_message(struct lwdistcomm_dds_spdp_impl_s *impl, lwdistcomm_dds_spdp_message_t *msg)
{
    memset(msg, 0, sizeof(*msg));
    
    /* 填充消息头部 */
    memcpy(msg->header.magic, SPDP_MAGIC, 4);
    msg->header.version = SPDP_VERSION;
    msg->header.message_type = LWDISTCOMM_DDS_SPDP_MESSAGE_PARTICIPANT_ANNOUNCE;
    msg->header.domain_id = impl->config.domain_id;
    msg->header.participant_id = impl->config.participant_id;
    
    /* 设置时间戳 */
    get_current_timestamp(&msg->header.timestamp_sec, &msg->header.timestamp_nsec);
    
    /* 填充参与者信息 */
    strncpy(msg->participant.participant_name, impl->config.participant_name, sizeof(msg->participant.participant_name) - 1);
    msg->participant.protocol_version = SPDP_VERSION;
    msg->participant.vendor_id = 0xAC00; /* ACOAUTO */
    msg->participant.product_id = 0x0001;
    msg->participant.host_id = (uint32_t)getpid();
    msg->participant.application_id = 0x0001;
    msg->participant.instance_id = (uint32_t)time(NULL);
    msg->participant.lease_duration_sec = impl->config.lease_duration_sec;
    msg->participant.lease_duration_nsec = 0;
    
    /* 填充端点信息 */
    msg->endpoint.endpoint_id = impl->config.participant_id;
    msg->endpoint.is_writer = 1;
    
    /* 获取本地IP地址 */
    char local_ip[16];
    inet_ntop(AF_INET, &impl->local_addr.sin_addr, local_ip, sizeof(local_ip));
    strncpy(msg->endpoint.transport_address, local_ip, sizeof(msg->endpoint.transport_address) - 1);
    msg->endpoint.port = impl->config.multicast_port;
    
    /* 填充主题信息 */
    if (impl->participant && impl->participant->impl) {
        lwdistcomm_dds_domain_participant_impl_t *participant_impl = impl->participant->impl;
        
        msg->num_topics = participant_impl->num_topics;
        if (msg->num_topics > 0) {
            msg->topics = (lwdistcomm_dds_spdp_topic_info_t *)malloc(sizeof(lwdistcomm_dds_spdp_topic_info_t) * msg->num_topics);
            if (msg->topics) {
                for (uint32_t i = 0; i < msg->num_topics; i++) {
                    strncpy(msg->topics[i].topic_name, participant_impl->topics[i]->name, sizeof(msg->topics[i].topic_name) - 1);
                    strncpy(msg->topics[i].type_name, participant_impl->topics[i]->type_name, sizeof(msg->topics[i].type_name) - 1);
                    msg->topics[i].topic_id = i + 1;
                }
            }
        }
    }
}

/**
 * SPDP线程函数
 */
static void *spdp_thread_func(void *arg)
{
    struct lwdistcomm_dds_spdp_impl_s *impl = (struct lwdistcomm_dds_spdp_impl_s *)arg;
    char buffer[SPDP_MAX_MESSAGE_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len;
    fd_set read_fds;
    struct timeval timeout;
    time_t last_announce = 0;
    time_t last_cleanup = 0;
    
    impl->thread_running = true;
    
    while (impl->running) {
        time_t now = time(NULL);
        
        /* 定期发送宣告消息 */
        if (now - last_announce >= impl->config.announce_interval_sec) {
            lwdistcomm_dds_spdp_message_t msg;
            build_participant_announce_message(impl, &msg);
            send_spdp_message(impl, &msg);
            if (msg.topics) {
                free(msg.topics);
            }
            last_announce = now;
        }
        
        /* 定期清理过期条目 */
        if (now - last_cleanup >= 1) {
            cleanup_expired_entries(impl);
            last_cleanup = now;
        }
        
        /* 等待消息 */
        FD_ZERO(&read_fds);
        FD_SET(impl->socket_fd, &read_fds);
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ret = select(impl->socket_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ret < 0) {
            continue;
        }
        
        if (ret > 0 && FD_ISSET(impl->socket_fd, &read_fds)) {
            sender_addr_len = sizeof(sender_addr);
            ssize_t n = recvfrom(
                impl->socket_fd,
                buffer,
                sizeof(buffer),
                0,
                (struct sockaddr *)&sender_addr,
                &sender_addr_len
            );
            
            if (n > 0) {
                process_spdp_message(impl, buffer, n, &sender_addr);
            }
        }
    }
    
    impl->thread_running = false;
    return NULL;
}

/**
 * 创建SPDP实例
 */
lwdistcomm_dds_spdp_t *lwdistcomm_dds_spdp_create(
    const lwdistcomm_dds_spdp_config_t *config,
    lwdistcomm_dds_domain_participant_t *participant
)
{
    if (!config || !participant) {
        return NULL;
    }
    
    lwdistcomm_dds_spdp_t *spdp = (lwdistcomm_dds_spdp_t *)malloc(sizeof(lwdistcomm_dds_spdp_t));
    if (!spdp) {
        return NULL;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = (struct lwdistcomm_dds_spdp_impl_s *)malloc(sizeof(struct lwdistcomm_dds_spdp_impl_s));
    if (!impl) {
        free(spdp);
        return NULL;
    }
    
    /* 初始化配置 */
    memcpy(&impl->config, config, sizeof(lwdistcomm_dds_spdp_config_t));
    
    /* 设置默认值 */
    if (strlen(impl->config.multicast_address) == 0) {
        strcpy(impl->config.multicast_address, SPDP_DEFAULT_MULTICAST_ADDRESS);
    }
    if (impl->config.multicast_port == 0) {
        impl->config.multicast_port = SPDP_DEFAULT_MULTICAST_PORT;
    }
    if (impl->config.announce_interval_sec == 0) {
        impl->config.announce_interval_sec = SPDP_DEFAULT_ANNOUNCE_INTERVAL_SEC;
    }
    if (impl->config.lease_duration_sec == 0) {
        impl->config.lease_duration_sec = SPDP_DEFAULT_LEASE_DURATION_SEC;
    }
    
    impl->participant = participant;
    impl->running = false;
    impl->thread_running = false;
    impl->num_discovered_participants = 0;
    impl->num_discovered_topics = 0;
    impl->participant_callback = NULL;
    impl->participant_callback_arg = NULL;
    impl->topic_callback = NULL;
    impl->topic_callback_arg = NULL;
    
    pthread_mutex_init(&impl->mutex, NULL);
    
    /* 创建套接字 */
    impl->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (impl->socket_fd < 0) {
        pthread_mutex_destroy(&impl->mutex);
        free(impl);
        free(spdp);
        return NULL;
    }
    
    /* 设置套接字选项 */
    int opt = 1;
    setsockopt(impl->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* 绑定到本地地址 */
    memset(&impl->local_addr, 0, sizeof(impl->local_addr));
    impl->local_addr.sin_family = AF_INET;
    impl->local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    impl->local_addr.sin_port = htons(impl->config.multicast_port);
    
    if (bind(impl->socket_fd, (struct sockaddr *)&impl->local_addr, sizeof(impl->local_addr)) < 0) {
        close(impl->socket_fd);
        pthread_mutex_destroy(&impl->mutex);
        free(impl);
        free(spdp);
        return NULL;
    }
    
    /* 设置多播地址 */
    memset(&impl->multicast_addr, 0, sizeof(impl->multicast_addr));
    impl->multicast_addr.sin_family = AF_INET;
    impl->multicast_addr.sin_addr.s_addr = inet_addr(impl->config.multicast_address);
    impl->multicast_addr.sin_port = htons(impl->config.multicast_port);
    
    /* 加入多播组 */
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(impl->config.multicast_address);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(impl->socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        close(impl->socket_fd);
        pthread_mutex_destroy(&impl->mutex);
        free(impl);
        free(spdp);
        return NULL;
    }
    
    spdp->config = impl->config;
    spdp->impl = impl;
    
    return spdp;
}

/**
 * 销毁SPDP实例
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_delete(
    lwdistcomm_dds_spdp_t *spdp
)
{
    if (!spdp || !spdp->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    /* 停止SPDP */
    lwdistcomm_dds_spdp_stop(spdp);
    
    /* 关闭套接字 */
    if (impl->socket_fd >= 0) {
        close(impl->socket_fd);
    }
    
    /* 清理资源 */
    pthread_mutex_destroy(&impl->mutex);
    free(impl);
    free(spdp);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 启动SPDP
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_start(
    lwdistcomm_dds_spdp_t *spdp
)
{
    if (!spdp || !spdp->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    pthread_mutex_lock(&impl->mutex);
    
    if (impl->running) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    impl->running = true;
    
    /* 创建并启动线程 */
    int ret = pthread_create(&impl->thread, NULL, spdp_thread_func, impl);
    if (ret != 0) {
        impl->running = false;
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_ERROR;
    }
    
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 停止SPDP
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_stop(
    lwdistcomm_dds_spdp_t *spdp
)
{
    if (!spdp || !spdp->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    pthread_mutex_lock(&impl->mutex);
    
    if (!impl->running) {
        pthread_mutex_unlock(&impl->mutex);
        return LWDISTCOMM_DDS_RETCODE_OK;
    }
    
    impl->running = false;
    pthread_mutex_unlock(&impl->mutex);
    
    /* 等待线程结束 */
    if (impl->thread_running) {
        pthread_join(impl->thread, NULL);
    }
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 发送参与者宣告消息
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_send_participant_announce(
    lwdistcomm_dds_spdp_t *spdp
)
{
    if (!spdp || !spdp->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    lwdistcomm_dds_spdp_message_t msg;
    build_participant_announce_message(impl, &msg);
    
    int ret = send_spdp_message(impl, &msg);
    if (msg.topics) {
        free(msg.topics);
    }
    
    return ret >= 0 ? LWDISTCOMM_DDS_RETCODE_OK : LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 发送主题宣告消息
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_send_topic_announce(
    lwdistcomm_dds_spdp_t *spdp,
    const lwdistcomm_dds_topic_t *topic
)
{
    if (!spdp || !spdp->impl || !topic) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    lwdistcomm_dds_spdp_message_t msg;
    memset(&msg, 0, sizeof(msg));
    
    /* 填充消息头部 */
    memcpy(msg.header.magic, SPDP_MAGIC, 4);
    msg.header.version = SPDP_VERSION;
    msg.header.message_type = LWDISTCOMM_DDS_SPDP_MESSAGE_TOPIC_ANNOUNCE;
    msg.header.domain_id = impl->config.domain_id;
    msg.header.participant_id = impl->config.participant_id;
    
    /* 设置时间戳 */
    get_current_timestamp(&msg.header.timestamp_sec, &msg.header.timestamp_nsec);
    
    /* 填充参与者信息 */
    strncpy(msg.participant.participant_name, impl->config.participant_name, sizeof(msg.participant.participant_name) - 1);
    
    /* 填充主题信息 */
    strncpy(msg.topic.topic_name, topic->name, sizeof(msg.topic.topic_name) - 1);
    strncpy(msg.topic.type_name, topic->type_name, sizeof(msg.topic.type_name) - 1);
    msg.topic.topic_id = 1;
    
    /* 填充端点信息 */
    msg.endpoint.endpoint_id = impl->config.participant_id;
    msg.endpoint.is_writer = 1;
    
    /* 获取本地IP地址 */
    char local_ip[16];
    inet_ntop(AF_INET, &impl->local_addr.sin_addr, local_ip, sizeof(local_ip));
    strncpy(msg.endpoint.transport_address, local_ip, sizeof(msg.endpoint.transport_address) - 1);
    msg.endpoint.port = impl->config.multicast_port;
    
    int ret = send_spdp_message(impl, &msg);
    
    return ret >= 0 ? LWDISTCOMM_DDS_RETCODE_OK : LWDISTCOMM_DDS_RETCODE_ERROR;
}

/**
 * 设置参与者回调函数
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_set_participant_callback(
    lwdistcomm_dds_spdp_t *spdp,
    lwdistcomm_dds_spdp_participant_callback_t callback,
    void *arg
)
{
    if (!spdp || !spdp->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    pthread_mutex_lock(&impl->mutex);
    impl->participant_callback = callback;
    impl->participant_callback_arg = arg;
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 设置主题回调函数
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_set_topic_callback(
    lwdistcomm_dds_spdp_t *spdp,
    lwdistcomm_dds_spdp_topic_callback_t callback,
    void *arg
)
{
    if (!spdp || !spdp->impl) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    pthread_mutex_lock(&impl->mutex);
    impl->topic_callback = callback;
    impl->topic_callback_arg = arg;
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 获取已发现的参与者数量
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_get_discovered_participants_count(
    lwdistcomm_dds_spdp_t *spdp,
    uint32_t *count
)
{
    if (!spdp || !spdp->impl || !count) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    pthread_mutex_lock(&impl->mutex);
    *count = impl->num_discovered_participants;
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}

/**
 * 获取已发现的主题数量
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_get_discovered_topics_count(
    lwdistcomm_dds_spdp_t *spdp,
    uint32_t *count
)
{
    if (!spdp || !spdp->impl || !count) {
        return LWDISTCOMM_DDS_RETCODE_BAD_PARAMETER;
    }
    
    struct lwdistcomm_dds_spdp_impl_s *impl = spdp->impl;
    
    pthread_mutex_lock(&impl->mutex);
    *count = impl->num_discovered_topics;
    pthread_mutex_unlock(&impl->mutex);
    
    return LWDISTCOMM_DDS_RETCODE_OK;
}
