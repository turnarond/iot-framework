/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: resource_types.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include <string>
#include <vector>
#include <chrono>

namespace edge {
namespace core {

// CPU使用情况
struct CpuUsage {
    float totalUsage;           // 总CPU使用率(%)
    float userUsage;            // 用户态使用率(%)
    float systemUsage;          // 系统态使用率(%)
    float idleTime;             // 空闲时间(%)
    uint32_t coreCount;         // CPU核心数
};

// 内存使用情况
struct MemoryUsage {
    uint64_t total;             // 总内存(bytes)
    uint64_t used;              // 已使用内存(bytes)
    uint64_t free;              // 空闲内存(bytes)
    uint64_t cached;            // 缓存内存(bytes)
    float usagePercent;         // 使用率(%)
};

// 磁盘使用情况
struct DiskUsage {
    std::string mountPoint;     // 挂载点
    uint64_t total;             // 总空间(bytes)
    uint64_t used;              // 已使用空间(bytes)
    uint64_t free;              // 空闲空间(bytes)
    float usagePercent;         // 使用率(%)
};

// 网络使用情况
struct NetworkUsage {
    std::string interface;      // 网络接口
    uint64_t rxBytes;           // 接收字节数
    uint64_t txBytes;           // 发送字节数
    uint64_t rxPackets;         // 接收包数
    uint64_t txPackets;         // 发送包数
    float rxSpeed;              // 接收速度(bytes/s)
    float txSpeed;              // 发送速度(bytes/s)
};

// 系统负载
struct SystemLoad {
    float load1;                // 1分钟平均负载
    float load5;                // 5分钟平均负载
    float load15;               // 15分钟平均负载
    float cpuTemperature;       // CPU温度(°C)
};

// 资源快照
struct ResourceSnapshot {
    std::chrono::system_clock::time_point timestamp;
    CpuUsage cpu;
    MemoryUsage memory;
    std::vector<DiskUsage> disks;
    std::vector<NetworkUsage> networks;
    SystemLoad load;
    
    // 转换为JSON
    std::string toJson() const;
};

// 资源告警
struct ResourceAlert {
    std::string resourceType;   // 资源类型 (cpu/memory/disk/network)
    std::string resourceName;   // 资源名称 (如磁盘挂载点、网络接口)
    float currentValue;         // 当前值
    float threshold;            // 阈值
    std::string message;        // 告警消息
    std::chrono::system_clock::time_point timestamp;
};

} // namespace core
} // namespace edge