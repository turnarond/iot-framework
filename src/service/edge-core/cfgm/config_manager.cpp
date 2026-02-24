/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: config_manager.cpp .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "config_manager.h"
#include "cfgm/config_types.h"
#include "cfgm_service.h"
#include "../../../comms/edge-util/util.h"
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/SQLite/SQLiteException.h>
#include <Poco/Data/Session.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

// VSOA 头文件
#include "lwcomm/lwcomm.h"
#include "lwlog/lwlog.h"

extern CLWLog g_logger;

using namespace Poco::Data;
using namespace Poco::Data::SQLite;
using namespace Poco::Data::Keywords;

using namespace edge::core;

// 默认配置
const SystemConfig kDefaultSystemConfig = {
    // CloudConfig cloud_endpoint
    {
        "mqtt.acoinfo-cloud.com", // endpoint
        "mqtt",                   // protocol
        1883,                     // port
        "",                       // clientId (运行时生成)
        "default_user",           // username
        "default_password",       // password
        false,                    // ssl_enable
        "",                       // caFile
        "",                       // certFile
        "",                       // keyFile
        60,                       // keepalive
        5                         // reconnectInterval
    },
    // CloudConfig register_endpoint
    {
        "mqtt.acoinfo-cloud.com", // endpoint
        "mqtt",                   // protocol
        1883,                     // port
        "",                       // clientId (运行时生成)
        "",                       // username
        "",                       // password
        false,                    // ssl_enable
        "",                       // caFile
        "",                       // certFile
        "",                       // keyFile
        60,                       // keepalive
        5                         // reconnectInterval
    },
    // DeviceInfoConfig
    {
        "SN12345678", // deviceId (运行时设置)
        "EdgeDevice", // deviceName
        "general",    // deviceType
        "EDGE1010",   // deviceModel           
        "ACOINFO",    // manufacturer
        "CloudTest",  // projectId
        "nanjing",    // location
        "1.0.0",      // firmwareVersion
        RegistrationPhase::UNCONFIGURED, // registrationPhase
        {"monitoring", "logging", "control"} // capabilities
    },
    // ResourceMonitoringConfig
    {},
    // LoggingConfig
    {},
    // databaseName
    "edge_system.db",
    // redisAddress
    "localhost:6379"
};

const std::string kConfigTableName = "config_log";
const std::string kDeviceTableName = "device_identity";

// 从JSON对象解析CloudConfig
CloudConfig parseCloudConfig(const Poco::JSON::Object::Ptr& obj) {
    CloudConfig config;
    if (obj->has("endpoint")) config.endpoint = obj->getValue<std::string>("endpoint");
    if (obj->has("protocol")) config.protocol = obj->getValue<std::string>("protocol");
    if (obj->has("port")) config.port = static_cast<uint16_t>(obj->getValue<int>("port"));
    if (obj->has("clientId")) config.clientId = obj->getValue<std::string>("clientId");
    if (obj->has("username")) config.username = obj->getValue<std::string>("username");
    if (obj->has("password")) config.password = obj->getValue<std::string>("password");
    if (obj->has("sslEnable")) config.sslEnable = obj->getValue<bool>("sslEnable");
    if (obj->has("keepalive")) config.keepalive = obj->getValue<uint32_t>("keepalive");
    if (obj->has("reconnectInterval")) config.reconnectInterval = obj->getValue<uint32_t>("reconnectInterval");
    return config;
}

// 从JSON对象解析RegistrationConfig
DeviceInfoConfig parseRegistrationConfig(const Poco::JSON::Object::Ptr& obj) 
{
    DeviceInfoConfig config;
    if (obj->has("deviceId")) config.deviceId = obj->getValue<std::string>("deviceId");
    if (obj->has("deviceName")) config.deviceName = obj->getValue<std::string>("deviceName");
    if (obj->has("deviceType")) config.deviceType = obj->getValue<std::string>("deviceType");
    if (obj->has("manufacturer")) config.manufacturer = obj->getValue<std::string>("manufacturer");
    if (obj->has("firmwareVersion")) config.firmwareVersion = obj->getValue<std::string>("firmwareVersion");
    
    if (obj->has("capabilities") && obj->isObject("capabilities")) {
        Poco::JSON::Array::Ptr capabilities = obj->getArray("capabilities");
        for (size_t i = 0; i < capabilities->size(); ++i) {
            config.capabilities.push_back(capabilities->getElement<std::string>(i));
        }
    }
    return config;
}

// 从JSON对象解析ResourceMonitoringConfig
ResourceMonitoringConfig parseResourceMonitoringConfig(const Poco::JSON::Object::Ptr& obj) {
    ResourceMonitoringConfig config;
    if (obj->has("enabled")) config.enabled = obj->getValue<bool>("enabled");
    if (obj->has("interval")) config.interval = obj->getValue<uint32_t>("interval");
    if (obj->has("cpuThreshold")) config.cpuThreshold = static_cast<float>(obj->getValue<double>("cpuThreshold"));
    if (obj->has("memoryThreshold")) config.memoryThreshold = static_cast<float>(obj->getValue<double>("memoryThreshold"));
    if (obj->has("diskThreshold")) config.diskThreshold = static_cast<float>(obj->getValue<double>("diskThreshold"));
    if (obj->has("reportChangesOnly")) config.reportChangesOnly = obj->getValue<bool>("reportChangesOnly");
    return config;
}

// 从JSON对象解析LoggingConfig
LoggingConfig parseLoggingConfig(const Poco::JSON::Object::Ptr& obj) {
    LoggingConfig config;
    if (obj->has("level")) config.level = obj->getValue<std::string>("level");
    if (obj->has("maxFileSize")) config.maxFileSize = obj->getValue<uint32_t>("maxFileSize");
    if (obj->has("maxFiles")) config.maxFiles = obj->getValue<uint32_t>("maxFiles");
    if (obj->has("uploadEnabled")) config.uploadEnabled = obj->getValue<bool>("uploadEnabled");
    if (obj->has("uploadInterval")) config.uploadInterval = obj->getValue<uint32_t>("uploadInterval");
    if (obj->has("uploadEndpoint")) config.uploadEndpoint = obj->getValue<std::string>("uploadEndpoint");
    return config;
}

// 将配置转换为JSON字符串
std::string configToJson(const SystemConfig& config) {
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    
    // CloudConfig cloud_endpoint
    Poco::JSON::Object::Ptr cloudObj = new Poco::JSON::Object();
    cloudObj->set("endpoint", config.cloud_endpoint.endpoint);
    cloudObj->set("protocol", config.cloud_endpoint.protocol);
    cloudObj->set("port", static_cast<int>(config.cloud_endpoint.port));
    cloudObj->set("clientId", config.cloud_endpoint.clientId);
    cloudObj->set("username", config.cloud_endpoint.username);
    cloudObj->set("password", config.cloud_endpoint.password);
    cloudObj->set("ssl_enable", config.cloud_endpoint.sslEnable);
    cloudObj->set("keepalive", static_cast<int>(config.cloud_endpoint.keepalive));
    cloudObj->set("reconnectInterval", static_cast<int>(config.cloud_endpoint.reconnectInterval));
    root->set("cloud", cloudObj);
    
    // CloudConfig register_endpoint;

    // DeviceInfoConfig device_info
    Poco::JSON::Object::Ptr regObj = new Poco::JSON::Object();
    regObj->set("deviceId", config.device_info.deviceId);
    regObj->set("deviceName", config.device_info.deviceName);
    regObj->set("deviceType", config.device_info.deviceType);
    regObj->set("manufacturer", config.device_info.manufacturer);
    regObj->set("firmwareVersion", config.device_info.firmwareVersion);
    
    Poco::JSON::Array::Ptr capabilities = new Poco::JSON::Array();
    for (const auto& cap : config.device_info.capabilities) {
        capabilities->add(cap);
    }
    regObj->set("capabilities", capabilities);
    root->set("registration", regObj);
    
    // ResourceMonitoringConfig
    Poco::JSON::Object::Ptr resObj = new Poco::JSON::Object();
    resObj->set("enabled", config.resourceMonitoring.enabled);
    resObj->set("interval", static_cast<int>(config.resourceMonitoring.interval));
    resObj->set("cpuThreshold", static_cast<double>(config.resourceMonitoring.cpuThreshold));
    resObj->set("memoryThreshold", static_cast<double>(config.resourceMonitoring.memoryThreshold));
    resObj->set("diskThreshold", static_cast<double>(config.resourceMonitoring.diskThreshold));
    resObj->set("reportChangesOnly", config.resourceMonitoring.reportChangesOnly);
    root->set("resourceMonitoring", resObj);
    
    // LoggingConfig
    Poco::JSON::Object::Ptr logObj = new Poco::JSON::Object();
    logObj->set("level", config.logging.level);
    logObj->set("maxFileSize", static_cast<int>(config.logging.maxFileSize));
    logObj->set("maxFiles", static_cast<int>(config.logging.maxFiles));
    logObj->set("uploadEnabled", config.logging.uploadEnabled);
    logObj->set("uploadInterval", static_cast<int>(config.logging.uploadInterval));
    logObj->set("uploadEndpoint", config.logging.uploadEndpoint);
    root->set("logging", logObj);
    
    // Other configs
    root->set("databaseName", config.databaseName);
    root->set("redisAddress", config.redisAddress);
    
    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

// 单例模式实现
ConfigManager* ConfigManager::GetInstance() {
    static ConfigManager instance;
    return &instance;
}

ConfigManager::ConfigManager()
    : config_(kDefaultSystemConfig),
      monitoringActive_(false) {
    // 注册SQLite连接器
    SQLite::Connector::registerConnector();
}

ConfigManager::~ConfigManager() {
    StopMonitoring();
    SQLite::Connector::unregisterConnector();
}

bool ConfigManager::Init() 
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // 确保数据库目录存在
        config_db_path_ = LWComm::GetConfigPath();
        config_db_path_ += LW_OS_DIR_SEPARATOR;
        config_db_path_ += kDefaultSystemConfig.databaseName;
        
        // 创建数据库会话
        Session session("SQLite", config_db_path_);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "ConfigManager::Init() - Database path: %s", config_db_path_.c_str());
        
        // 初始化设备信息（如果不存在）
        if (!LoadDeviceConfig(session)) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "No device configuration found, using defaults");
            // 从操作系统获取设备ID，并设置未配置
            std::string deviceId = get_edge_sn();
            config_.device_info.deviceId = deviceId;
            config_.cloud_endpoint.clientId = deviceId;
            // TODO: 写入数据库
        }
        
        // 加载系统配置
        if (!LoadSystemConfig(session)) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "No system configuration found, using defaults");
        }
        
        // 设置VSOA服务端点
        SetupVsoaEndpoints();
        
        return true;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize ConfigManager: %s", 
            e.what());
        return false;
    }
}

bool ConfigManager::LoadDeviceConfig(Session& session) 
{
    try {
        std::string deviceId, deviceName, deviceTypeCode, deviceModel;
        std::string location, firmwareVersion, manufacturer, projectId;
        std::string endpoint, protocol, username, password; 
        bool ssl_enable;
        int connection_timeout, keepalive, reconnect_interval;

        int deviceType, endpointPort;
        int registrationPhase;
        int64_t lastHeartbeat, lastModifiedTime;

        Statement select(session);
        select << "SELECT dev.device_id, dev.device_name, dev.device_type, type.type_code, dev.device_model, "
               << "dev.location, dev.firmware_version, dev.manufacturer, dev.project_id, dev.registration_phase, "
               << "broker.endpoint, broker.protocol, broker.port, broker.username, broker.password, broker.ssl_enable, "
               << "broker.connection_timeout, broker.keepalive, broker.reconnect_interval "
               << "FROM device_identity dev "
               << "JOIN dict_device_type type ON dev.device_type = type.type_id "
               << "JOIN registration_broker broker ON dev.registration_broker_id = broker.id "
               << "WHERE "
	           << "dev.is_active = 1",
               into(deviceId), into(deviceName), into(deviceType), into(deviceTypeCode), into(deviceModel),
               into(location), into(firmwareVersion), into(manufacturer), into(projectId), into(registrationPhase),
               into(endpoint), into(protocol), into(endpointPort), into(username), into(password), into(ssl_enable),
               into(connection_timeout), into(keepalive), into(reconnect_interval);

        if (select.execute() == 0) {
            return false; // 无设备信息
        }

        config_.device_info.deviceId = deviceId;
        config_.device_info.deviceName = deviceName;
        config_.device_info.deviceType = deviceTypeCode;
        config_.device_info.deviceModel = deviceModel;
        config_.device_info.registrationPhase = static_cast<RegistrationPhase>(registrationPhase);
        config_.device_info.projectId = projectId;
        config_.device_info.manufacturer = manufacturer;
        config_.device_info.location = location;
        config_.device_info.firmwareVersion = firmwareVersion;

        config_.register_endpoint.endpoint = endpoint;
        config_.register_endpoint.port = endpointPort;
        config_.register_endpoint.protocol = protocol;
        config_.register_endpoint.username = username;
        config_.register_endpoint.password = password;
        config_.register_endpoint.sslEnable = ssl_enable;
        config_.register_endpoint.connectTimeout = connection_timeout;
        config_.register_endpoint.keepalive = keepalive;
        config_.register_endpoint.reconnectInterval = reconnect_interval;

        // 设置MQTT客户端的证书地址；使用简单的CA文件认证，单向认证
        config_.register_endpoint.caFile = std::string(LWComm::GetDataPath()) + LW_OS_DIR_SEPARATOR + "certs" + LW_OS_DIR_SEPARATOR + "ca.crt";
        
        config_.register_endpoint.clientId = deviceId; // 使用设备ID作为客户端ID
        
        return true;
    } catch (const SQLiteException& e) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Failed to load device config: %s", 
            e.what());
        return false;
    }
}

bool ConfigManager::LoadSystemConfig(Session& session) 
{
    try {
        std::string metadata;
        int64_t createdTime;
        
        Statement select(session);
        select << "SELECT metadata, created_time FROM " << kConfigTableName
               << " WHERE source_type = 0 ORDER BY created_time DESC LIMIT 1",
               into(metadata), into(createdTime);
               
        if (select.execute() == 0) {
            return false; // 无系统配置
        }
        
        // 解析JSON配置
    //     Parser parser;
    //     auto result = parser.parse(metadata);
    //     Object::Ptr root = result.extract<Object::Ptr>();
        
    //     if (root->has("cloud")) {
    //         config_.cloud_endpoint = parseCloudConfig(root->getObject("cloud"));
    //     }
    //     if (root->has("registration")) {
    //         config_.registration = parseRegistrationConfig(root->getObject("registration"));
    //     }
    //     if (root->has("resourceMonitoring")) {
    //         config_.resourceMonitoring = parseResourceMonitoringConfig(root->getObject("resourceMonitoring"));
    //     }
    //     if (root->has("logging")) {
    //         config_.logging = parseLoggingConfig(root->getObject("logging"));
    //     }
    //     if (root->has("databaseName")) {
    //         config_.databaseName = root->getValue<std::string>("databaseName");
    //     }
    //     if (root->has("redisAddress")) {
    //         config_.redisAddress = root->getValue<std::string>("redisAddress");
    //     }
        
    //     return true;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Failed to load system config: %s", e.what());
        return false;
    }
    return true;
}

void ConfigManager::SetupVsoaEndpoints () {
    try {
        // TODO: 实现VSOA服务注册
        CFGM_SERVICE->Start();
        
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "VSOA endpoints for configuration service registered");
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to setup VSOA endpoints: %s", e.what());
    }
}

// TODO: VSOA请求处理函数
// void ConfigManager::handleConfigRequest(
//     const vsoa::Request::Ptr& request, 
//     const vsoa::Response::Ptr& response) {
//     // 实现VSOA请求处理
// }

const SystemConfig& ConfigManager::GetSystemConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

const CloudConfig& ConfigManager::GetCloudConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.cloud_endpoint;
}

const CloudConfig& ConfigManager::GetRegisterConfig() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.register_endpoint;
}

const DeviceInfoConfig& ConfigManager::GetDeviceInfoConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.device_info;
}

const ResourceMonitoringConfig& ConfigManager::GetResourceConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.resourceMonitoring;
}

const LoggingConfig& ConfigManager::GetLoggingConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.logging;
}

void ConfigManager::SetSystemConfig(const SystemConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}
void ConfigManager::SetCloudConfig(const CloudConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_.cloud_endpoint = config;
}

void ConfigManager::SetRegistrationConfig(const CloudConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_.register_endpoint = config;
}

void ConfigManager::SetDeviceInfoConfig(const DeviceInfoConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_.device_info = config;
}

void ConfigManager::SetResourceConfig(const ResourceMonitoringConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_.resourceMonitoring = config;
}

void ConfigManager::SetLoggingConfig(const LoggingConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_.logging = config;
}

bool ConfigManager::SetConfig(const std::string& configType, const std::string& jsonValue) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        Poco::JSON::Parser parser;
        auto result = parser.parse(jsonValue);
        Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();
        
        bool changed = false;
        std::vector<std::string> affectedKeys;
        std::vector<ConfigPartition> affectedPartitions;
        bool requiresRestart = false;
        
        if (configType == "cloud" && obj) {
            CloudConfig newConfig = parseCloudConfig(obj);
            if (newConfig.isValid() && newConfig != config_.cloud_endpoint) {
                config_.cloud_endpoint = newConfig;
                changed = true;
                affectedKeys.push_back("cloud");
                affectedPartitions.push_back(ConfigPartition::CLOUD_SERVICE);
                // 检查是否需要重启连接
                if (config_.cloud_endpoint.endpoint != newConfig.endpoint || 
                    config_.cloud_endpoint.port != newConfig.port) {
                    requiresRestart = true;
                }
            }
        } 
        else if (configType == "registration" && obj) {
            DeviceInfoConfig newConfig = parseRegistrationConfig(obj);
            if (newConfig.isValid() && newConfig != config_.device_info) {
                config_.device_info = newConfig;
                changed = true;
                affectedKeys.push_back("registration");
                affectedPartitions.push_back(ConfigPartition::REGISTRATION);
            }
        }
        else if (configType == "resource" && obj) {
            ResourceMonitoringConfig newConfig = parseResourceMonitoringConfig(obj);
            if (newConfig != config_.resourceMonitoring) {
                config_.resourceMonitoring = newConfig;
                changed = true;
                affectedKeys.push_back("resource");
                affectedPartitions.push_back(ConfigPartition::RESOURCE);
            }
        }
        else if (configType == "logging" && obj) {
            LoggingConfig newConfig = parseLoggingConfig(obj);
            if (newConfig != config_.logging) {
                config_.logging = newConfig;
                changed = true;
                affectedKeys.push_back("logging");
                affectedPartitions.push_back(ConfigPartition::LOGGING);
            }
        }
        else if (configType == "system" && obj) {
            // 系统级配置更新
            if (obj->has("databaseName")) {
                config_.databaseName = obj->getValue<std::string>("databaseName");
                changed = true;
                affectedKeys.push_back("databaseName");
                affectedPartitions.push_back(ConfigPartition::SYSTEM);
                requiresRestart = true; // 数据库路径变更需要重启
            }
            if (obj->has("redisAddress")) {
                config_.redisAddress = obj->getValue<std::string>("redisAddress");
                changed = true;
                affectedKeys.push_back("redisAddress");
                affectedPartitions.push_back(ConfigPartition::SYSTEM);
            }
        }
        
        if (changed) {
            // 保存到数据库
            Session session("SQLite", config_.databaseName);
            
            std::string metadata = configToJson(config_);
            int64_t now = Poco::Timestamp().epochMicroseconds() / 1000; // 转为毫秒
            int source_type = static_cast<int>(ConfigSource::EDGE);
            Statement insert(session);
            insert << "INSERT INTO " << kConfigTableName 
                   << " (source_type, source_id, metadata, created_time) "
                   << "VALUES (?, edge-core, ?, ?)",
                   use(source_type), // 本地边缘侧配置
                //    use(UUIDGenerator().createRandom().toString()),
                   use(metadata),
                   use(now);
            insert.execute();
            
            // 通知监听模块
            NotifyListeners(affectedPartitions, requiresRestart);
            
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to update configuration: %s", e.what());
        return false;
    }
}

void ConfigManager::NotifyListeners(
    const std::vector<ConfigPartition>& affectedPartitions,
    bool requiresRestart) 
{
    
    ConfigChangeEvent event;
    event.affectedPartitions = affectedPartitions;
    event.source = ConfigSource::EDGE;
    event.reason = "Configuration update";
    event.timestamp = Poco::Timestamp().epochMicroseconds() / 1000; // 毫秒
    event.requiresRestart = requiresRestart;
    
    // 确定需要通知的模块
    for (const auto& partition : affectedPartitions) {
        switch (partition) {
            case ConfigPartition::CLOUD_SERVICE:
                event.targetModules.push_back(ModuleId::TOKEN_SERVICE);
                break;
            case ConfigPartition::REGISTRATION:
                event.targetModules.push_back(ModuleId::REGM);
                break;
            case ConfigPartition::RESOURCE:
                event.targetModules.push_back(ModuleId::RESM);
                break;
            case ConfigPartition::LOGGING:
                event.targetModules.push_back(ModuleId::LOGM);
                break;
            case ConfigPartition::SYSTEM:
                event.targetModules.push_back(ModuleId::REGM);
                break;
            default:
                break;
        }
    }
    
    // 通过VSOA通知相关模块
    try {
        // TODO: 通过VSOA发送配置变更事件
        // for (const auto& moduleId : event.targetModules) {
        //     std::string serviceName = getModuleName(moduleId) + ".service";
        //     vsoa::Request::Ptr request = new vsoa::Request("config_changed");
        //     request->setContent(Poco::JSON::Stringifier::stringify(eventToJson(event)));
        //     vsoa::ServiceRegistry::getInstance().callService(serviceName, request);
        // }
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Failed to notify modules via VSOA: %s", e.what());
    }

    // 通知本地注册的监听器
    for (auto& listenerPair : moduleListeners_) {
        ModuleId moduleId = listenerPair.first;
        IConfigListener* listener = listenerPair.second;
        
        if (listener && std::find(event.targetModules.begin(), 
            event.targetModules.end(), moduleId) != event.targetModules.end()) {
            try {
                listener->onConfigChanged(event);
            } catch (const std::exception& e) {
                g_logger.LogMessage(LW_LOGLEVEL_WARN, "Listener for module %d threw exception: %s", 
                    static_cast<int>(moduleId), e.what());
            }
        }
    }
}

void ConfigManager::RegisterModuleListener(ModuleId moduleId, IConfigListener* listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (listener) {
        moduleListeners_[moduleId] = listener;
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registered config listener for module %d", static_cast<int>(moduleId));
    }
}

void ConfigManager::UnregisterModuleListener(ModuleId moduleId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = moduleListeners_.find(moduleId);
    if (it != moduleListeners_.end()) {
        moduleListeners_.erase(it);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Unregistered config listener for module %d", static_cast<int>(moduleId));
    }
}

bool ConfigManager::Reload() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        Session session("SQLite", config_.databaseName);
        
        // 重新加载设备配置
        LoadDeviceConfig(session);
        
        // 重新加载系统配置
        LoadSystemConfig(session);
        
        // 通知所有模块配置已重新加载
        ConfigChangeEvent event;
        event.source = ConfigSource::SYSTEM_DEFAULT;
        event.reason = "Configuration reload";
        event.timestamp = Poco::Timestamp().epochMicroseconds() / 1000;
        event.requiresRestart = false;
        
        // 通知所有已注册的监听器
        for (auto& listenerPair : moduleListeners_) {
            IConfigListener* listener = listenerPair.second;
            if (listener) {
                try {
                    listener->onConfigChanged(event);
                } catch (const std::exception& e) {
                    g_logger.LogMessage(LW_LOGLEVEL_WARN, "Listener threw exception during reload: %s", e.what());
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to reload configuration: %s", e.what());
        return false;
    }
}

void ConfigManager::StartMonitoring() 
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (monitoringActive_) {
        return;
    }
    
    monitoringActive_ = true;
    monitorThread_ = std::thread([this] {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Configuration monitoring started");
        Session session("SQLite", config_.databaseName);
        
        while (monitoringActive_) {
            try {
                // 检查配置表是否有新记录
                int64_t lastCheckTime = Poco::Timestamp().epochMicroseconds() / 1000;
                Poco::Thread::sleep(5000); // 每5秒检查一次
                
                int64_t now = Poco::Timestamp().epochMicroseconds() / 1000;
                std::string metadata;
                int64_t createdTime;
                
                Statement select(session);
                select << "SELECT metadata, created_time FROM " << kConfigTableName
                       << " WHERE created_time > ? ORDER BY created_time DESC LIMIT 1",
                       use(lastCheckTime), into(metadata), into(createdTime);
                
                if (select.execute() > 0 && createdTime > lastCheckTime) {
                    // 有新配置，重新加载
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "New configuration detected, reloading...");
                    Reload();
                }
                
                // 检查VSOA是否有配置更新请求
                // TODO: 检查VSOA队列中的配置更新请求
            } catch (const std::exception& e) {
                g_logger.LogMessage(LW_LOGLEVEL_WARN, "Error during config monitoring: %s", e.what());
                Poco::Thread::sleep(10000); // 出错时等待更长时间
            }
        }
        
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Configuration monitoring stopped");
    });
}

void ConfigManager::StopMonitoring() 
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!monitoringActive_) {
            return;
        }
        monitoringActive_ = false;
    }
    
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }
}
