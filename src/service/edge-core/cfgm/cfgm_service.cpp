/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: cfgm_service.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "cfgm_service.h"
#include "dto/cfgm_config_dto.hpp"
#include "lwlog/lwlog.h"
#include "platform_sdk/server.h"
#include "vsoa_dto/core/IODefinitions.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/parser/ParsingError.hpp"

extern CLWLog g_logger;

using namespace edge::core;

CfgmService CfgmService::instance_;
CfgmService *CfgmService::GetInstance()
{
    return &instance_;
}

CfgmService::~CfgmService()
{
    if (sh_) {
        sh_->DestroyServer();
        sh_ = nullptr;
    }
}

int CfgmService::Start()
{
    int ret = 0;
    try {
        sh_ = std::make_shared<vsoa_sdk::server::ServerHandle>(server_name_);

        auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
        serializerConfig->includeNullFields = false;
        
        auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
        deserializerConfig->allowUnknownFields = false;

        obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
    } catch (std::exception &e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create server handle: %s", e.what());
        return -1;
    } 

    sh_->SetServerPort(server_port_);

    sh_->SetOnConnectCb([](vsoa_sdk::server::ServerHandle *server, uint32_t cid, bool connected, void *arg) {
            if (connected) {
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Client connected: %d", cid);
            } else {
                g_logger.LogMessage(LW_LOGLEVEL_INFO, "Client disconnected: %d", cid);
            }
        }, this);

    InitServerListeners();

    ret = sh_->CreateServer();
    if (ret != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create server: %d", ret);
        return -1;
    }
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Server %s started at port %d", 
        server_name_.c_str(), server_port_);

    sh_->SrvSpinAsync(3000);
    return 0; 
}

void CfgmService::Stop()
{
    if (sh_) {
        sh_->DestroyServer();
        sh_ = nullptr;
    }
}

void CfgmService::SetRegistrationConfigCallback(std::function<void(const DeviceInfoConfig&)> callback)
{
    registration_callback_ = callback;
}

void CfgmService::SetNetworkInterfaceConfigCallback(std::function<void(const NetworkInterfaceConfig&)> callback)
{
    network_interface_callback_ = callback;
}

void CfgmService::SetIpConfigCallback(std::function<void(const IpConfig&)> callback)
{
    ip_callback_ = callback;
}

void CfgmService::SetLogConfigCallback(std::function<void(const LoggingConfig&)> callback)
{
    log_callback_ = callback;
}

void CfgmService::SetResourceConfigCallback(std::function<void(const ResourceMonitoringConfig&)> callback)
{
    resource_callback_ = callback;
}

void CfgmService::InitServerListeners()
{ 
    if (sh_ == nullptr || obj_mapper_ == nullptr) {
        return;
    }

    // 设备注册配置更新
    sh_->AddRpcListener(url_edge_config_registration_, 
        [this](vsoa_sdk::server::CliRpcInfo &cli, const void *dto, size_t len, void *arg) {
            // TODO: 调用 RegistrationConfigChanged 传过来的std::function
            try {
                auto dto_obj = obj_mapper_->readFromString<vsoa::Object<RegistrationConfigDTO>>
                    (vsoa::String((char*)dto, len));
                
                DeviceInfoConfig config {
                    dto_obj->deviceId,
                    dto_obj->deviceName,
                    dto_obj->deviceType,
                    dto_obj->manufaturer,
                    dto_obj->projectId,
                    dto_obj->location,
                    dto_obj->firmwareVersion,
                };
                registration_callback_(config);
            } catch (vsoa::parser::ParsingError &e) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse registration config: %s", e.what());
                sh_->SrvResponse(cli, vsoa::Status::CODE_2, nullptr, 0);
                return;
            }
            sh_->SrvResponse(cli, vsoa::Status::CODE_0, nullptr, 0);
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Recv registration config success.");
        }, 
        this);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Add rpc listener %s success.", 
        url_edge_config_registration_.c_str());

    // 网络接口配置更新
    sh_->AddRpcListener(url_edge_config_network_interface_, 
        [this](vsoa_sdk::server::CliRpcInfo &cli, const void *dto, size_t len, void *arg) {
            try {
                auto dto_obj = obj_mapper_->readFromString<vsoa::Object<NetworkInterfaceConfigDTO>>
                    (vsoa::String((char*)dto, len));
                
                NetworkInterfaceConfig config {
                    dto_obj->interfaceName,
                    InterFaceType::UNKNOWN,
                    dto_obj->enabled,
                    dto_obj->macAddr,
                    dto_obj->mtu,
                    dto_obj->description,
                };
                
                network_interface_callback_(config);
            } catch (vsoa::parser::ParsingError &e) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse network config: %s", e.what());
                sh_->SrvResponse(cli, vsoa::Status::CODE_2, nullptr, 0);
                return;
            }
            sh_->SrvResponse(cli, vsoa::Status::CODE_0, nullptr, 0);
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Recv network config success.");
        }, 
        this);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Add rpc listener %s success.", 
        url_edge_config_network_interface_.c_str());

    // ip配置更新
    sh_->AddRpcListener(url_dege_config_ip_, 
        [this](vsoa_sdk::server::CliRpcInfo &cli, const void *dto, size_t len, void *arg) {
            try {
                auto dto_obj = obj_mapper_->readFromString<vsoa::Object<IpConfigDTO>>
                    (vsoa::String((char*)dto, len));
                
                IpConfig config {
                    dto_obj->interfaceName,
                    dto_obj->ipAddress,
                    dto_obj->ipVersion,
                    dto_obj->subnetMask,
                    dto_obj->gateway,
                    dto_obj->dnsServer,
                    dto_obj->searchDomain,
                    dto_obj->metric
                };
                
                ip_callback_(config);
            } catch (vsoa::parser::ParsingError &e) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse network config: %s", e.what());
                sh_->SrvResponse(cli, vsoa::Status::CODE_2, nullptr, 0);
                return;
            }
            sh_->SrvResponse(cli, vsoa::Status::CODE_0, nullptr, 0);
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Recv network config success.");
        }, 
        this);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Add rpc listener %s success.", 
        url_dege_config_ip_.c_str());

    // 日志配置更新
    sh_->AddRpcListener(url_edge_config_logging_, 
        [this](vsoa_sdk::server::CliRpcInfo &cli, const void *dto, size_t len, void *arg) {
            try {
                auto dto_obj = obj_mapper_->readFromString<vsoa::Object<LoggingConfigDTO>>
                    (vsoa::String((char*)dto, len));
                
                LoggingConfig config {
                    dto_obj->level,
                    dto_obj->maxFileSize,
                    dto_obj->maxFiles,
                    dto_obj->uploadEnabled,
                    dto_obj->uploadInterval,
                    dto_obj->uploadEndpoint,
                };
            } catch (vsoa::parser::ParsingError &e) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse logging config: %s", e.what());
                sh_->SrvResponse(cli, vsoa::Status::CODE_2, nullptr, 0);
                return;
            }
            sh_->SrvResponse(cli, vsoa::Status::CODE_0, nullptr, 0);
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Recv logging config success.");
        }, 
        this);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Add rpc listener %s success.", 
        url_edge_config_logging_.c_str());

    // 资源配置更新
    sh_->AddRpcListener(url_edge_config_resource_, 
        [this](vsoa_sdk::server::CliRpcInfo &cli, const void *dto, size_t len, void *arg) {
            try {
                auto dto_obj = obj_mapper_->readFromString<vsoa::Object<ResourceConfigDTO>>
                    (vsoa::String((char*)dto, len));
                
                ResourceMonitoringConfig config {
                    dto_obj->enabled,
                    dto_obj->interval,
                    dto_obj->cpuThreshold,
                    dto_obj->memoryThreshold,
                    dto_obj->diskThreshold,
                    dto_obj->reportChangesOnly
                };
                resource_callback_(config);
            } catch (vsoa::parser::ParsingError &e) {
                g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to parse resource config: %s", e.what());
                sh_->SrvResponse(cli, vsoa::Status::CODE_2, nullptr, 0);
                return;
            }
            sh_->SrvResponse(cli, vsoa::Status::CODE_0, nullptr, 0);
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Recv resource config success.");
        }, 
        this);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Add rpc listener %s success.", 
        url_edge_config_resource_.c_str());
}
