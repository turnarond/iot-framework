/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: resource_monitor.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include "resource_types.h"
#include "../cfgm/config_types.h"
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

namespace edge {
namespace core {

class ResourceMonitor {
public:
    static ResourceMonitor *GetInstance();
    
    // 初始化
    bool Init();
    
    // 开始监控
    void StartMonitoring();
    
    // 停止监控
    void StopMonitoring();
    
    // // 手动触发资源采集
    // ResourceSnapshot captureSnapshot();
    
    // // 获取最新资源快照
    // ResourceSnapshot getLatestSnapshot() const;
    
    // // 获取历史数据
    // std::vector<ResourceSnapshot> getHistoryData(
    //     std::chrono::system_clock::time_point startTime,
    //     std::chrono::system_clock::time_point endTime) const;
    
    // // 添加告警监听器
    // using AlertListener = std::function<void(const ResourceAlert& alert)>;
    // void addAlertListener(AlertListener listener);
    
    // // 设置上报回调
    // using ReportCallback = std::function<void(const ResourceSnapshot& snapshot)>;
    // void setReportCallback(ReportCallback callback);
    
    // // 检查资源状态
    // void checkResources();
    
    // 清理资源
    void cleanup();
    
private:
    ResourceMonitor() = default;
    ~ResourceMonitor() = default;
    
    mutable std::mutex mutex_;
    ResourceSnapshot latestSnapshot_;
    std::vector<ResourceSnapshot> history_;
    ResourceMonitoringConfig config_;
    bool monitoringActive_;
    std::thread monitorThread_;
    
    // // 采集CPU使用率
    // CpuUsage collectCpuUsage();
    
    // // 采集内存使用率
    // MemoryUsage collectMemoryUsage();
    
    // // 采集磁盘使用率
    // std::vector<DiskUsage> collectDiskUsage();
    
    // // 采集网络使用率
    // std::vector<NetworkUsage> collectNetworkUsage();
    
    // // 采集系统负载
    // SystemLoad collectSystemLoad();
    
    // // 检查资源阈值
    // void checkThresholds(const ResourceSnapshot& snapshot);
    
    // // 触发告警
    // void triggerAlert(const std::string& resourceType, 
    //                  const std::string& resourceName,
    //                  float currentValue,
    //                  float threshold,
    //                  const std::string& message);
    
    // // 监控线程主循环
    // void monitoringLoop();
    
    // // 限制历史数据大小
    // void trimHistory();

private:
    static ResourceMonitor instance_;
};
#define RESOURCE_MONITOR ResourceMonitor::GetInstance()
} // namespace core
} // namespace edge