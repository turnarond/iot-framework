#ifndef LWDISTCOMM_DDS_SPDP_H
#define LWDISTCOMM_DDS_SPDP_H

#include "dds.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SPDP消息类型
 */
typedef enum {
    LWDISTCOMM_DDS_SPDP_MESSAGE_PARTICIPANT_ANNOUNCE,
    LWDISTCOMM_DDS_SPDP_MESSAGE_PARTICIPANT_LEAVE,
    LWDISTCOMM_DDS_SPDP_MESSAGE_TOPIC_ANNOUNCE,
    LWDISTCOMM_DDS_SPDP_MESSAGE_TOPIC_REMOVE
} lwdistcomm_dds_spdp_message_type_t;

/**
 * SPDP消息头部
 */
typedef struct {
    uint8_t magic[4];           /* 魔术字: "SPDP" */
    uint16_t version;            /* 版本号 */
    uint16_t message_length;     /* 消息长度 */
    lwdistcomm_dds_spdp_message_type_t message_type; /* 消息类型 */
    uint32_t domain_id;          /* 域ID */
    uint32_t participant_id;     /* 参与者ID */
    uint32_t timestamp_sec;      /* 时间戳(秒) */
    uint32_t timestamp_nsec;     /* 时间戳(纳秒) */
} lwdistcomm_dds_spdp_header_t;

/**
 * SPDP参与者信息
 */
typedef struct {
    char participant_name[64];   /* 参与者名称 */
    uint32_t protocol_version;   /* 协议版本 */
    uint32_t vendor_id;          /* 厂商ID */
    uint32_t product_id;         /* 产品ID */
    uint32_t host_id;            /* 主机ID */
    uint32_t application_id;     /* 应用ID */
    uint32_t instance_id;        /* 实例ID */
    uint32_t lease_duration_sec; /* 租约持续时间(秒) */
    uint32_t lease_duration_nsec; /* 租约持续时间(纳秒) */
} lwdistcomm_dds_spdp_participant_info_t;

/**
 * SPDP主题信息
 */
typedef struct {
    char topic_name[64];         /* 主题名称 */
    char type_name[64];          /* 类型名称 */
    uint32_t topic_id;           /* 主题ID */
} lwdistcomm_dds_spdp_topic_info_t;

/**
 * SPDP端点信息
 */
typedef struct {
    uint32_t endpoint_id;        /* 端点ID */
    uint8_t is_writer;           /* 是否为写入器 */
    char transport_address[128]; /* 传输地址 */
    uint16_t port;               /* 端口号 */
} lwdistcomm_dds_spdp_endpoint_info_t;

/**
 * SPDP消息
 */
typedef struct {
    lwdistcomm_dds_spdp_header_t header;                  /* 消息头部 */
    lwdistcomm_dds_spdp_participant_info_t participant;   /* 参与者信息 */
    lwdistcomm_dds_spdp_topic_info_t topic;               /* 主题信息 */
    lwdistcomm_dds_spdp_endpoint_info_t endpoint;         /* 端点信息 */
    uint32_t num_topics;                                  /* 主题数量 */
    lwdistcomm_dds_spdp_topic_info_t *topics;             /* 主题列表 */
} lwdistcomm_dds_spdp_message_t;

/**
 * SPDP配置
 */
typedef struct {
    uint32_t domain_id;          /* 域ID */
    uint32_t participant_id;     /* 参与者ID */
    char participant_name[64];   /* 参与者名称 */
    char multicast_address[16];  /* 组播地址 */
    uint16_t multicast_port;     /* 组播端口 */
    uint32_t announce_interval_sec; /* 宣告间隔(秒) */
    uint32_t lease_duration_sec; /* 租约持续时间(秒) */
    bool enable_ipv6;            /* 是否启用IPv6 */
} lwdistcomm_dds_spdp_config_t;

/**
 * SPDP回调函数类型
 */
typedef void (*lwdistcomm_dds_spdp_participant_callback_t)(
    lwdistcomm_dds_domain_participant_t *participant,
    const lwdistcomm_dds_spdp_participant_info_t *info,
    bool is_new,
    void *arg
);

typedef void (*lwdistcomm_dds_spdp_topic_callback_t)(
    lwdistcomm_dds_domain_participant_t *participant,
    const lwdistcomm_dds_spdp_topic_info_t *info,
    bool is_new,
    void *arg
);

/**
 * 创建SPDP实例
 */
typedef struct lwdistcomm_dds_spdp_impl_s lwdistcomm_dds_spdp_impl_t;
typedef struct {
    lwdistcomm_dds_spdp_config_t config;
    lwdistcomm_dds_spdp_impl_t *impl;
} lwdistcomm_dds_spdp_t;

/**
 * 创建SPDP实例
 */
lwdistcomm_dds_spdp_t *lwdistcomm_dds_spdp_create(
    const lwdistcomm_dds_spdp_config_t *config,
    lwdistcomm_dds_domain_participant_t *participant
);

/**
 * 销毁SPDP实例
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_delete(
    lwdistcomm_dds_spdp_t *spdp
);

/**
 * 启动SPDP
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_start(
    lwdistcomm_dds_spdp_t *spdp
);

/**
 * 停止SPDP
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_stop(
    lwdistcomm_dds_spdp_t *spdp
);

/**
 * 发送参与者宣告消息
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_send_participant_announce(
    lwdistcomm_dds_spdp_t *spdp
);

/**
 * 发送主题宣告消息
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_send_topic_announce(
    lwdistcomm_dds_spdp_t *spdp,
    const lwdistcomm_dds_topic_t *topic
);

/**
 * 设置回调函数
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_set_participant_callback(
    lwdistcomm_dds_spdp_t *spdp,
    lwdistcomm_dds_spdp_participant_callback_t callback,
    void *arg
);

lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_set_topic_callback(
    lwdistcomm_dds_spdp_t *spdp,
    lwdistcomm_dds_spdp_topic_callback_t callback,
    void *arg
);

/**
 * 获取已发现的参与者数量
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_get_discovered_participants_count(
    lwdistcomm_dds_spdp_t *spdp,
    uint32_t *count
);

/**
 * 获取已发现的主题数量
 */
lwdistcomm_dds_retcode_t lwdistcomm_dds_spdp_get_discovered_topics_count(
    lwdistcomm_dds_spdp_t *spdp,
    uint32_t *count
);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_DDS_SPDP_H */
