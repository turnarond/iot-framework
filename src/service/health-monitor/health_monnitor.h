// services/health_monitor.h
#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace edge {
namespace services {

// 健康状态
enum class HealthStatus {
    UNKNOWN,
    HEALTHY,
    WARNING,
    CRITICAL,
    OFFLINE
};

// 检查类型
enum class CheckType {
    PROCESS_EXISTENCE,
    CPU_USAGE,
    MEMORY_USAGE,
    RESPONSE_TIME,
    NETWORK_CONNECTIVITY,
    STORAGE_SPACE,
    CUSTOM
};

// 健康检查配置
struct HealthCheckConfig {
    std::string name;
    std::string target; // service name, app id, or resource path
    CheckType type;
    uint32_t intervalSec;
    uint32_t timeoutSec;
    uint32_t maxFailuresBeforeAlert;
    
    // 阈值配置
    struct Thresholds {
        float cpuPercentWarning;
        float cpuPercentCritical;
        float memoryPercentWarning;
        float memoryPercentCritical;
        uint32_t responseTimeWarningMs;
        uint32_t responseTimeCriticalMs;
        uint64_t storageSpaceWarningKB;
        uint64_t storageSpaceCriticalKB;
    } thresholds;
    
    // 恢复动作
    std::vector<std::string> recoveryActions; // "restart", "notify", "failover", etc.
};

// 健康报告
struct HealthReport {
    std::string entityId; // service name or app id
    HealthStatus status;
    std::string message;
    uint64_t timestamp;
    std::map<std::string, float> metrics;
    uint32_t failureCount;
};

class HealthMonitor {
public:
    HealthMonitor();
    ~HealthMonitor();
    
    // 检查管理
    bool loadHealthCheckConfig(const std::string& configPath);
    bool registerHealthCheck(const HealthCheckConfig& config);
    bool unregisterHealthCheck(const std::string& checkName);
    
    // 手动触发检查
    HealthReport runHealthCheck(const std::string& checkName);
    std::vector<HealthReport> runAllHealthChecks();
    
    // 状态查询
    HealthStatus getOverallHealth() const;
    HealthReport getLatestReport(const std::string& entityId) const;
    std::vector<HealthReport> getAllReports() const;
    
    // 事件订阅
    void registerHealthStatusListener(
        std::function<void(const HealthReport&)> listener
    );
    
    // 恢复操作
    bool triggerRecoveryAction(const std::string& entityId, const std::string& action);
    bool autoRecover(const std::string& entityId);
    
    // 资源监控
    struct SystemResourceUsage {
        float cpuLoadPercent;
        uint32_t freeMemoryKB;
        uint32_t totalMemoryKB;
        uint64_t freeStorageKB;
        uint64_t totalStorageKB;
        uint32_t activeConnections;
    };
    
    SystemResourceUsage getSystemResourceUsage() const;
    
    // 云端报告
    bool enableCloudReporting(bool enable, uint32_t intervalSec = 60);
    bool sendHealthReportToCloud(const HealthReport& report);
    
    // 初始化
    bool init();
    bool start();
    void stop();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace services
} // namespace edge

#endif // HEALTH_MONITOR_H