/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: registry_manager.cpp .
 *
 * Date: 2025-12-18
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "registry_manager.h"
#include "base/base.h"
#include "cfgm/config_manager.h"
#include "cloud/cloud_handler.h"
#include "base/dto/cloud_msg_dto.hpp"
#include "lwlog/lwlog.h"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/parser/ParsingError.hpp"
#include <memory>
#include <stdexcept>


using namespace edge::core;

extern CLWLog g_logger;

// Helper function to convert InterFaceType enum to string
static std::string InterFaceTypeToString(InterFaceType type) {
    switch (type) {
        case InterFaceType::ETHERNET: return "ethernet";
        case InterFaceType::WIFI: return "wifi";
        case InterFaceType::CELLULAR: return "cellular";
        case InterFaceType::VIRTUAL: return "virtual";
        case InterFaceType::LOOPBACK: return "loopback";
        case InterFaceType::BLUETOOTH: return "bluetooth";
        case InterFaceType::USB: return "usb";
        case InterFaceType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

RegistryManager RegistryManager::instance_;
RegistryManager *RegistryManager::GetInstance()
{
    return &instance_;
}

// 初始化
bool RegistryManager::Init()
{
    // 获取设备信息和注册信息
    deviceConfig_ = CONFIG_MANAGER->GetDeviceInfoConfig();
    cloudRegistryConfig_ = CONFIG_MANAGER->GetRegisterConfig();
    phase_ = deviceConfig_.registrationPhase;
    if (phase_ != RegistrationPhase::REGISTERED && phase_ != RegistrationPhase::UNCONFIGURED) {
        // 重启之后，要重新进行broker连接、注册。
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device not registered, phase: %d", phase_);
        phase_ = RegistrationPhase::CONFIGURED;
        if (!SECURITY_CONTEXT->init(deviceConfig_)) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN,"Failed to start security service");
        }
        phaseStartTime_ = std::chrono::steady_clock::now();
    }

    registrationResponseTopic_ = registrationResponseTopic_ + deviceConfig_.deviceId;
    heartBeatTopic_ = "/edge/" + deviceConfig_.deviceId + "/status/heartbeat";

    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = true;

    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);

    return true;
}

// 执行设备注册
bool RegistryManager::CheckRegister()
{
    if (phase_ == RegistrationPhase::UNCONFIGURED) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device not registered, phase: unconfigured");
        return false;
    }

    if (phase_ == RegistrationPhase::CONFIGURED || phase_ == RegistrationPhase::DISCONNECTED) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered, phase: configured");
        return ConnectToRegistrationBroker();
    }

    if (phase_ == RegistrationPhase::CONNECTING) {
        auto duration = std::chrono::steady_clock::now() - phaseStartTime_;
        if (std::chrono::duration_cast<std::chrono::seconds>(duration).count() > cloudRegistryConfig_.connectTimeout) {
            phase_ = RegistrationPhase::CONFIGURED;
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Registration broker connect timeout, reconnect interval is %d", 
                cloudRegistryConfig_.reconnectInterval);
            DisconnectFromRegistrationBroker();
            return false;
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered, phase: connecting");
        return true;
    }

    // 注册失败和连接成功都需要重新注册
    if (phase_ == RegistrationPhase::CONNECTED || phase_ == RegistrationPhase::FAILED) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered, phase: connected");
        return DoRegistrationRequest();
    }

    if (phase_ == RegistrationPhase::REGISTERING) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered, phase: registering");
        auto duration = std::chrono::steady_clock::now() - phaseStartTime_;
        // 暂时使用keepalive作为注册超时时间
        if (std::chrono::duration_cast<std::chrono::seconds>(duration).count() > cloudRegistryConfig_.keepalive) {
            phase_ = RegistrationPhase::FAILED;
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Registration timeout");
            return false;
        }
        return true;
    }

    if (phase_ == RegistrationPhase::REGISTERED) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered, phase: registered");
        ConnectToServiceBroker();
        return true;
    }

    if (phase_ == RegistrationPhase::EXPIRED) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered, phase: expired");
        return false;
    }

    return false;
}

// 建立注册broker连接
bool RegistryManager::ConnectToRegistrationBroker()
{
    CloudHandler::Config register_config;
    register_config.onConnect = [this](int ret) {
        phase_ = RegistrationPhase::CONNECTED;
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Connected to registration broker");
    };
    register_config.onDisconnect = [this](int ret) {
        phase_ = RegistrationPhase::UNCONFIGURED;
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Disconnected from registration broker, ret is %d", ret);
    };
    register_config.onError = [this](int ret, const std::string& msg) {
        phase_ = RegistrationPhase::FAILED;
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Registration broker error: %d, msg is %s", 
            ret, msg.c_str());
    };

    register_config.brokerHost = cloudRegistryConfig_.endpoint;
    register_config.brokerPort = cloudRegistryConfig_.port;
    register_config.username = cloudRegistryConfig_.username;
    register_config.password = cloudRegistryConfig_.password;
    register_config.keepAlive = cloudRegistryConfig_.keepalive;
    register_config.caFile = cloudRegistryConfig_.caFile;
    register_config.certFile = cloudRegistryConfig_.certFile;
    register_config.keyFile = cloudRegistryConfig_.keyFile;
    register_config.useTls = cloudRegistryConfig_.sslEnable;
    register_config.clientId = client_prefix_ + deviceConfig_.deviceId;

    try {
        registrationClient_ = std::make_shared<CloudHandler>(register_config);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration client initialized with clientId: %s to %s:%d", 
            register_config.clientId.c_str(), register_config.brokerHost.c_str(), register_config.brokerPort);
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize registration client: %s", e.what());
        return false;
    }

    registrationClient_->set_message_callback([this](int mid, const std::string& topic, const std::string& payload) 
        {
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Received message from topic: %s", topic.c_str());
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Received message: %s", payload.c_str());
            if (topic == registrationResponseTopic_) {
                // 解析注册响应
                auto response = RegistrationResponseMsgDto::createShared();
                try {
                    response = obj_mapper_->readFromString<vsoa::Object<RegistrationResponseMsgDto>>(
                        vsoa::String((char*)payload.c_str(), payload.length()));
                } catch (const vsoa::parser::ParsingError& e) {
                    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse registration response: %s, position: %d", 
                        e.what(), e.getPosition());
                    return;
                } catch (const std::exception& e) {
                    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse registration response: %s", e.what());
                    return;
                }
                if (response->code == 200) {
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration successful");
                    phase_ = RegistrationPhase::REGISTERED;
                    auto now = std::chrono::system_clock::now();
                    currentCredential_ = ServiceCredential {
                        response->payload->cloudEndpoint,
                        response->payload->cloudPort,
                        std::string(response->payload->certInfo->deviceCert),
                        std::string(response->payload->certInfo->caChain),
                        response->payload->certInfo->validFrom,
                        response->payload->certInfo->validTo
                    };
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration credential: broker endpoint = %s", 
                        currentCredential_.serviceBrokerEndpoint.c_str());
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration credential: broker port = %d", 
                        currentCredential_.serviceBrokerPort);
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration credential: device cert = %s", 
                        currentCredential_.deviceCert.c_str());
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration credential: ca cert = %s", 
                        currentCredential_.caCert.c_str());
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration credential: issued at = %lu", 
                        currentCredential_.issuedAt);
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Registration credential: expires at = %lu", 
                        currentCredential_.expiresAt);
                    
                    // 保存注册凭证
                    if (!SECURITY_CONTEXT->submitCertificate(currentCredential_.deviceCert)) {
                        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to submit device certificate");
                        phase_ = RegistrationPhase::FAILED;
                        return;
                    }
                    if (!SECURITY_CONTEXT->submitCaCertificate(currentCredential_.caCert)) {
                        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to submit CA certificate");
                        phase_ = RegistrationPhase::FAILED;
                        return;
                    }
                    phase_ = RegistrationPhase::REGISTERED;
                } else {
                    phase_ = RegistrationPhase::FAILED;
                    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Registration failed, code: %d, message: %s", 
                        response->code, std::string(response->message).c_str());
                }
            }
        }
    );

    registrationClient_->connect();
    registrationClient_->subscribe(registrationResponseTopic_);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Subscribed to topic: %s", 
        registrationResponseTopic_.c_str());
    phase_ = RegistrationPhase::CONNECTING;
    phaseStartTime_ = std::chrono::steady_clock::now();
    registrationClient_->loop_async(1000);
    return true;
}

void RegistryManager::DisconnectFromRegistrationBroker()
{
    if (registrationClient_) {
        registrationClient_->disconnect();
        registrationClient_.reset();
    }
    phase_ = RegistrationPhase::DISCONNECTED; // 配置完成，等待连接
}

bool RegistryManager::DoRegistrationRequest()
{
    auto request = vsoa::Object<RegistrationRequestMsgDto>::createShared();
    request->header = vsoa::Object<HeaderDto>::createShared();
    request->header->msgId = "edge-req-" + UUIDGenerator.create().toString();

    request->header->timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    request->device = vsoa::Object<DeviceInnerDto>::createShared();
    request->device->sn = deviceConfig_.deviceId;
    request->device->name = deviceConfig_.deviceName;
    if (!deviceConfig_.deviceModel.empty()) {
        request->device->model = deviceConfig_.deviceModel;
    }
    if (!deviceConfig_.deviceType.empty()) {
        request->device->type = deviceConfig_.deviceType;
    }
    if (!deviceConfig_.firmwareVersion.empty()) {
        request->device->firmwareVersion = deviceConfig_.firmwareVersion;
    }

    request->network = vsoa::Object<NetworkDto>::createShared();
    request->network->interfaces = vsoa::Object<NetInterfaceDto>::createShared();
    request->network->interfaces->interfaceName = networkConfig_.if_name;
    request->network->interfaces->interfaceType = InterFaceTypeToString(networkConfig_.type);
    request->network->interfaces->ip = ipConfig_.ip_address;
    request->network->interfaces->subnet = ipConfig_.subnet_mask;
    request->network->interfaces->gateway = ipConfig_.gateway;
    request->network->interfaces->macAddr = networkConfig_.mac_address;
    request->network->interfaces->mtu = networkConfig_.mtu;
    request->network->interfaces->description = networkConfig_.description;
    request->network->interfaces->enabled = true;

    request->security = vsoa::Object<SecurityDto>::createShared();
    auto device_csr = SECURITY_CONTEXT->generateDeviceCsr();
    if (device_csr.has_value()) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device CSR available");
        request->security->csr = device_csr.value();
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "No device CSR available");
        phase_ = RegistrationPhase::FAILED; // 注册失败
        return false;
    }
    // request->security->publishKey = device_crt->publishKey;
    // request->security->nonce = device_crt->nonce;

    vsoa::String payload;
    try {
        payload = obj_mapper_->writeToString(request);
        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Registration request payload: %s", payload->c_str());
    } catch (vsoa::parser::ParsingError &e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Registration request payload serialization failed: %s", e.what());
        phase_ = RegistrationPhase::FAILED; // 注册失败
        return false;
    }
    try {
        registrationClient_->publish(registrationRequestTopic_, payload->c_str());
    } catch (std::runtime_error &e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Registration request publish failed: %s", e.what());
        phase_ = RegistrationPhase::FAILED; // 注册失败
    }

    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Sent registration request to broker");
    phase_ = RegistrationPhase::REGISTERING; // 注册完成，等待服务token
    phaseStartTime_ = std::chrono::steady_clock::now();
    return true;
}
// 刷新服务token
bool RegistryManager::RefreshServiceToken()
{
    return true;
}

// 建立服务broker连接
bool RegistryManager::ConnectToServiceBroker()
{
    // if (!serviceClient_) {
    //     serviceClient_ = std::make_shared<LwcommClient>(
    //         serviceBrokerHost_, serviceBrokerPort_, serviceBrokerTopic_);
    // }
    // serviceClient_->connect();
    // serviceClient_->subscribe(serviceBrokerTopic_);
    // g_logger.LogMessage(LW_LOGLEVEL_INFO, "Subscribed to topic: %s", 
    //     serviceBrokerTopic_.c_str());
    // phase_ = RegistrationPhase::CONNECTING;
    // phaseStartTime_ = std::chrono::steady_clock::now();
    // serviceClient_->loop_async(1000);
    return true;
}
