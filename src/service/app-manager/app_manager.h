// services/app_manager.h
#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace edge {
namespace services {

// APP状态
enum class AppState {
    NOT_INSTALLED,
    INSTALLED,
    STARTING,
    RUNNING,
    STOPPING,
    STOPPED,
    UPDATING,
    ERROR
};

// APP信息
struct AppInfo {
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::string path;
    AppState state;
    uint32_t pid;
    uint64_t installTime;
    uint64_t startTime;
    uint32_t memoryUsageKB;
    float cpuUsagePercent;
    std::map<std::string, std::string> metadata;
};

// APP配置
struct AppConfig {
    std::string id;
    bool autoStart;
    uint32_t restartPolicy; // 0: never, 1: on-failure, 2: always
    uint32_t maxRestartCount;
    std::map<std::string, std::string> environment;
    std::map<std::string, std::string> volumes; // host_path:container_path
    std::vector<std::string> networkPermissions;
    std::vector<std::string> devicePermissions;
};

class AppManager {
public:
    AppManager();
    ~AppManager();
    
    // APP生命周期
    bool installApp(const std::string& packagePath, const std::string& appId = "");
    bool uninstallApp(const std::string& appId);
    bool startApp(const std::string& appId);
    bool stopApp(const std::string& appId);
    bool restartApp(const std::string& appId);
    bool updateApp(const std::string& appId, const std::string& newPackagePath);
    
    // APP信息查询
    AppInfo getAppInfo(const std::string& appId) const;
    std::vector<AppInfo> getAllApps() const;
    std::vector<std::string> getRunningApps() const;
    
    // APP配置管理
    bool loadAppConfig(const std::string& configPath);
    bool saveAppConfig(const std::string& appId, const AppConfig& config);
    AppConfig getAppConfig(const std::string& appId) const;
    bool setAppAutoStart(const std::string& appId, bool autoStart);
    
    // APP资源限制
    bool setAppResourceLimits(const std::string& appId, uint32_t maxMemoryKB, 
                            uint32_t maxCpuPercent);
    
    // APP通信
    bool sendAppMessage(const std::string& appId, const std::string& topic, 
                      const std::string& payload);
    bool subscribeAppEvents(const std::string& appId, 
                          std::function<void(const std::string&, const std::string&)> callback);
    
    // 云端集成
    bool syncAppListToCloud();
    bool downloadAppFromCloud(const std::string& appId, const std::string& version,
                            std::function<void(float)> onProgress = nullptr);
    
    // 事件监听
    void registerAppStateListener(
        std::function<void(const std::string&, AppState, AppState)> listener
    );
    
    // 健康检查
    bool isAppHealthy(const std::string& appId) const;
    void triggerAppHealthCheck(const std::string& appId);
    
    // 初始化
    bool init();
    bool start();
    void shutdown();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace services
} // namespace edge

#endif // APP_MANAGER_H