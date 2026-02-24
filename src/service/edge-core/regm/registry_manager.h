/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: registry_manager.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include "cfgm/config_types.h"
#include "base/base.h"
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include "cloud/cloud_handler.h"
#include "security/security_context.h"

#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "Poco/UUID.h"
#include <Poco/UUIDGenerator.h>

namespace edge {
namespace core {

class RegistryManager {
public:
    static RegistryManager *GetInstance();
    
    // 初始化
    bool Init();
    
    // 执行设备注册
    bool CheckRegister();
    
    // 获取当前注册状态
    RegistrationPhase GetRegistrationPhase() const;
    
    // 刷新服务token
    bool RefreshServiceToken();
    
    // 建立注册broker连接
    bool ConnectToRegistrationBroker();

    // 断开与注册broker的连接
    void DisconnectFromRegistrationBroker();

    // 执行注册请求
    bool DoRegistrationRequest();

    // 建立服务broker连接
    bool ConnectToServiceBroker();
    
    // // 注册凭证变更监听器
    // using CredentialChangeListener = std::function<void(const ServiceCredential& newCredential)>;
    // void RegisterCredentialListener(CredentialChangeListener listener);

private:
    RegistryManager() {}
    ~RegistryManager() {}
    mutable std::mutex mutex_;
    DeviceInfoConfig deviceConfig_;  // 注册配置
    NetworkInterfaceConfig networkConfig_; // 网络配置
    IpConfig ipConfig_; // ip配置

    CloudConfig cloudRegistryConfig_{}; // 注册broker配置
    CloudConfig cloudServerConfig_{}; // 云端服务配置
    RegistrationPhase phase_ = RegistrationPhase::UNCONFIGURED; // 注册阶段
    std::chrono::steady_clock::time_point phaseStartTime_; // 阶段开始时间
    ServiceCredential currentCredential_{}; // 当前服务凭证
    std::shared_ptr<CloudHandler> registrationClient_;          // 注册broker连接
    std::shared_ptr<CloudHandler> serviceClient_;                // 服务broker连接
    std::atomic<bool> running_{false}; // 运行状态
    std::thread tokenRefreshThread_{}; // 凭证刷新线程
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr; // 对象映射器
    Poco::UUIDGenerator &UUIDGenerator = Poco::UUIDGenerator::defaultGenerator();

private:
    std::string registrationRequestTopic_ = "/sys/v1/registration/request"; // 注册请求topic
    std::string registrationResponseTopic_ = "/sys/v1/registration/response/"; // 注册应答 topic
    std::string heartBeatTopic_; // 心跳 topic 
    std::string client_prefix_ = "iot_";
    // // 从存储加载凭证
    // bool LoadCredentialFromStorage();
    
    // // 保存凭证到存储
    // bool SaveCredentialToStorage() const;
    
    // // 处理注册响应
    // void HandleRegistrationResponse(const std::string& payload);
    
    // // 处理凭证刷新响应
    // void HandleTokenRefreshResponse(const std::string& payload);
    
    // // token刷新线程
    // void TokenRefreshLoop();

    // // 重连服务broker
    // void ReconnectServiceBroker();
    
    // // 更新凭证
    // void UpdateCredential(const ServiceCredential& newCredential);
    
    // // 更新注册阶段
    // void UpdatePhase(RegistrationPhase newPhase);
private:
    static RegistryManager instance_;
};
#define REGISTRY_MANAGER RegistryManager::GetInstance()

} // namespace core
} // namespace edge