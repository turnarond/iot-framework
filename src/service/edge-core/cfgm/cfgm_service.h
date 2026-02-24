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

#pragma once

#include "config_types.h"
#include "platform_sdk/server.h"
#include <functional>

namespace edge {
namespace core {

class CfgmService {

public:
    ~CfgmService();

    int Start();
    void Stop();
    
    void SetRegistrationConfigCallback(std::function<void(const DeviceInfoConfig&)> callback);
    void SetNetworkInterfaceConfigCallback(std::function<void(const NetworkInterfaceConfig&)> callback);
    void SetIpConfigCallback(std::function<void(const IpConfig&)> callback);
    void SetLogConfigCallback(std::function<void(const LoggingConfig&)> callback);
    void SetResourceConfigCallback(std::function<void(const ResourceMonitoringConfig&)> callback);

public:
    static CfgmService *GetInstance();

private:
    void InitServerListeners();
    CfgmService() = default;

private:
    std::string server_name_ = "edge_cfgm_service";
    unsigned short server_port_ = 20001;
    std::string url_edge_config_registration_ = "/edge/config/registration/set";
    std::string url_edge_config_network_interface_ = "/edge/config/netif/set";
    std::string url_dege_config_ip_ = "/edge/config/ip/set";
    std::string url_edge_config_resource_ = "/edge/config/resource/set";
    std::string url_edge_config_logging_ = "/edge/config/log/set";
    std::shared_ptr<vsoa_sdk::server::ServerHandle> sh_ = nullptr;

    // 配置变更回调
    std::function<void(const DeviceInfoConfig&)> registration_callback_;
    std::function<void(const NetworkInterfaceConfig&)> network_interface_callback_;
    std::function<void(const IpConfig&)> ip_callback_;
    std::function<void(const LoggingConfig&)> log_callback_;
    std::function<void(const ResourceMonitoringConfig&)> resource_callback_;

    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr;
private:
    static CfgmService instance_;
};

#define CFGM_SERVICE CfgmService::GetInstance()

} // namespace core
} // namespace edge