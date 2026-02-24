/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: config_manager.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include "config_types.h"
#include <string>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "Poco/Data/Session.h"

using namespace Poco::Data;
namespace edge {
namespace core {

// 配置来源类型
enum class ConfigSource {
    SYSTEM_DEFAULT,  // 系统默认配置
    CLOUD,           // 云端下发配置
    EDGE,            // 本地边缘侧配置 (通过edge_server)
    FACTORY_RESET    // 工厂重置配置
};

// 配置分区
enum class ConfigPartition {
    SYSTEM, // 系统配置
    REGISTRATION, // 注册配置
    CLOUD_SERVICE, // 云服务配置
    NETWORK, // 网络配置
    RESOURCE, // 资源配置
    LOGGING, // 日志配置
    FIRMWARE, // 固件更新配置
    CUSTOM // 自定义配置
};

// 配置变更事件
struct ConfigChangeEvent {
    std::vector<ConfigPartition> affectedPartitions;
    std::vector<ModuleId> targetModules; // 需要通知的模块
    ConfigSource source;
    std::string reason;
    uint64_t timestamp;
    bool requiresRestart;
};

// 配置监听器接口
class IConfigListener {
public:
    virtual ~IConfigListener() = default;
    virtual void onConfigChanged(const ConfigChangeEvent& event) = 0;
};

// 配置管理器
class ConfigManager {
public:
    static ConfigManager *GetInstance();
    
    // 初始化配置管理器
    bool Init();
    
    // 获取配置
    const SystemConfig& GetSystemConfig() const;
    const CloudConfig& GetCloudConfig() const;
    const CloudConfig& GetRegisterConfig() const;
    const DeviceInfoConfig& GetDeviceInfoConfig() const;
    const ResourceMonitoringConfig& GetResourceConfig() const;
    const LoggingConfig& GetLoggingConfig() const;
    
    // 更新配置
    bool SetConfig(const std::string& configType, const std::string& jsonValue);
    void SetSystemConfig(const SystemConfig& config);
    void SetCloudConfig(const CloudConfig& config);
    void SetRegistrationConfig(const CloudConfig& config);
    void SetDeviceInfoConfig(const DeviceInfoConfig& config);
    void SetResourceConfig(const ResourceMonitoringConfig& config);
    void SetLoggingConfig(const LoggingConfig& config);
    
    // 注册模块监听器 (特定模块只监听相关配置)
    void RegisterModuleListener(ModuleId moduleId, IConfigListener* listener);

    // 注销模块监听器
    void UnregisterModuleListener(ModuleId moduleId);

    // 重新加载配置
    bool Reload();
    
    // 监控配置变化
    void StartMonitoring();
    void StopMonitoring();

private:
    bool LoadSystemConfig(Session& session);
    bool LoadDeviceConfig(Session& session);
    void SetupVsoaEndpoints();
    void NotifyListeners(const std::vector<ConfigPartition>& affectedPartitions,
        bool requiresRestart);

private:
    ConfigManager();
    ~ConfigManager();
    
    mutable std::mutex mutex_;
    std::string config_db_path_;
    SystemConfig config_;
    std::unordered_map<ModuleId, IConfigListener*> moduleListeners_; // 按模块注册的监听器
    bool monitoringActive_;
    std::thread monitorThread_;

private:
    std::string device_id_;     // 设备ID
    int device_type_;           // 设备类型
    int registration_phase_;    // 注册阶段
    
};
#define CONFIG_MANAGER ConfigManager::GetInstance()

// 便捷访问宏
#define GET_CLOUD_CONFIG() edge::core::ConfigManager::getInstance().getCloudConfig()
#define GET_REGISTRATION_CONFIG() edge::core::ConfigManager::getInstance().getRegistrationConfig()
#define GET_RESOURCE_CONFIG() edge::core::ConfigManager::getInstance().getResourceConfig()
#define GET_LOGGING_CONFIG() edge::core::ConfigManager::getInstance().getLoggingConfig()

} // namespace core
} // namespace edge
