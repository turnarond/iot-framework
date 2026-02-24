/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: config_types.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once

#include <string>
#include <vector>

#include "../base/base.h"

namespace edge {
namespace core {

enum class InterFaceType {
    ETHERNET = 1,
    WIFI,
    CELLULAR,
    VIRTUAL,
    LOOPBACK,
    BLUETOOTH,
    USB,
    UNKNOWN
};

// 注册状态
enum class RegistrationPhase {
    UNCONFIGURED = 0,   // 未配置注册信息
    CONFIGURED = 1,     // 已配置注册信息
    CONNECTING = 2,     // 正在连接到注册broker
    CONNECTED = 3,      // 已连接到注册broker
    DISCONNECTED = 4,   // 已断开注册broker连接
    REGISTERING = 5,    // 正在注册设备
    VERIFYING = 6,      // 正在验证注册信息
    REGISTERED = 7,     // 已完成注册，获取到服务凭证
    EXPIRED = 8,        // 服务凭证已过期
    FAILED = 9          // 注册失败
};

enum class DeviceType {
    UNKNOWN = 1,
    EDGE_DEVICE = 2,
    GATEWAY = 3,
    SENSOR = 4,
    CONTROLLER = 5,
    END_DEVICE = 6,
};

// 设备注册配置
struct DeviceInfoConfig {
    std::string deviceId;        // 设备唯一ID
    std::string deviceName;      // 设备名称
    std::string deviceType;      // 设备类型
    std::string deviceModel;     // 设备型号
    std::string manufacturer;    // 制造商
    std::string projectId;       // 项目ID
    std::string location;        // 位置
    std::string firmwareVersion; // 固件版本
    RegistrationPhase registrationPhase; // 注册阶段
    std::vector<std::string> capabilities; // 设备能力列表
    
    bool isValid() const {
        return !deviceId.empty() && !deviceName.empty() && !deviceType.empty();
    }

    bool operator == (const DeviceInfoConfig& cfg) { 
        return deviceId == cfg.deviceId && deviceName == cfg.deviceName &&
            deviceType == cfg.deviceType && manufacturer == cfg.manufacturer &&
            firmwareVersion == cfg.firmwareVersion && capabilities == cfg.capabilities;
    }

    bool operator != (const DeviceInfoConfig& cfg) {
        return !(*this == cfg);
    }
};

// 云端连接配置
struct CloudConfig {
    std::string endpoint;       // 云端地址
    std::string protocol;       // 协议类型 (mqtt/vsoa)
    uint16_t port;              // 端口
    std::string clientId;       // 客户端ID
    std::string username;       // 用户名
    std::string password;       // 密码/Token
    bool sslEnable;            // 是否使用TLS/SSL
    std::string caFile;         // CA文件路径
    std::string certFile;       // 证书文件路径
    std::string keyFile;        // 私钥文件路径
    uint32_t keepalive = 60;          // 保活时间(秒)
    uint32_t connectTimeout = 10;    // 连接超时时间(秒)
    uint32_t reconnectInterval = 5; // 重连间隔(秒)
    
    // 验证配置有效性
    bool isValid() const {
        return !endpoint.empty() && port > 0 && !clientId.empty();
    }
    bool operator == (const CloudConfig& cfg) {
        return endpoint == cfg.endpoint && protocol == cfg.protocol && port == cfg.port &&
            clientId == cfg.clientId && username == cfg.username && password == cfg.password &&
            sslEnable == cfg.sslEnable && keepalive == cfg.keepalive &&
            connectTimeout == cfg.connectTimeout && reconnectInterval == cfg.reconnectInterval;
    }

    bool operator != (const CloudConfig& cfg) {
        return !(*this == cfg);
    }
};

// 网络接口配置
struct NetworkInterfaceConfig {
    std::string if_name;        // 接口名称
    InterFaceType type;         // 接口类型
    bool enabled;               // 是否启用
    std::string mac_address;    // MAC地址
    int mtu;                    // MTU
    std::string description;    // 描述
    bool operator == (const NetworkInterfaceConfig& cfg) {
        return if_name == cfg.if_name && type == cfg.type &&
            mac_address == cfg.mac_address && mtu == cfg.mtu;
    }

    bool operator != (const NetworkInterfaceConfig& cfg) {
        return !(*this == cfg);
    }
};

// IP地址配置
struct IpConfig {
    std::string if_name;         // 接口名称
    std::string ip_address;     // IP地址
    int  ip_version;            // IP版本
    std::string subnet_mask;    // 子网掩码
    std::string gateway;        // 默认网关
    std::string dns_server;     // DNS服务器
    std::string search_domain;  // 搜索域
    int metric;                 // 路由Metric

    bool operator == (const IpConfig& cfg) {
        return ip_address == cfg.ip_address && subnet_mask == cfg.subnet_mask &&
            gateway == cfg.gateway && dns_server == cfg.dns_server &&
            search_domain == cfg.search_domain && metric == cfg.metric;
    }

    bool operator != (const IpConfig& cfg) {
        return !(*this == cfg);
    }
};

// 资源监控配置
struct ResourceMonitoringConfig {
    bool enabled = true;               // 是否启用
    uint32_t interval = 60;          // 监控间隔(秒)
    float cpuThreshold = 80.f;         // CPU使用率阈值(%)
    float memoryThreshold = 85.f;      // 内存使用率阈值(%)
    float diskThreshold = 90.f;        // 磁盘使用率阈值(%)
    bool reportChangesOnly = true;     // 仅上报变化
    
    bool operator == (const ResourceMonitoringConfig& cfg) {
        return enabled == cfg.enabled && interval == cfg.interval &&
            cpuThreshold == cfg.cpuThreshold && memoryThreshold == cfg.memoryThreshold &&
            diskThreshold == cfg.diskThreshold && reportChangesOnly == cfg.reportChangesOnly;
    }

    bool operator != (const ResourceMonitoringConfig& cfg) {
        return !(*this == cfg);
    }
};

// 日志配置
struct LoggingConfig {
    std::string level;          // 日志级别 (debug/info/warning/error)
    uint32_t maxFileSize;       // 单个日志文件最大大小(MB)
    uint32_t maxFiles;          // 最大保留文件数
    bool uploadEnabled;         // 是否启用日志上传
    uint32_t uploadInterval;    // 上传间隔(秒)
    std::string uploadEndpoint; // 上传端点

    bool operator == (const LoggingConfig& cfg) {
        return level == cfg.level && maxFileSize == cfg.maxFileSize &&
            maxFiles == cfg.maxFiles && uploadEnabled == cfg.uploadEnabled &&
            uploadInterval == cfg.uploadInterval && uploadEndpoint == cfg.uploadEndpoint;
    }

    bool operator != (const LoggingConfig& cfg) {
        return !(*this == cfg);
    }
};

// 系统配置
struct SystemConfig {
    CloudConfig cloud_endpoint;
    CloudConfig register_endpoint;
    DeviceInfoConfig device_info;
    ResourceMonitoringConfig resourceMonitoring;
    LoggingConfig logging;
    std::string databaseName;   // 配置数据库路径
    std::string redisAddress;   // Redis地址
};

} // namespace core
} // namespace edge