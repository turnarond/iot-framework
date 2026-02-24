#include "SystemConfigService.hpp"
#include <vsoa/error/error.hpp>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "common/Logger.hpp"
#include "lwlog/lwlog.h"

/**
 * 获取所有系统配置
 */
vsoa::Vector<vsoa::Object<SystemConfigDto>> SystemConfigService::getSystemConfig() {
    auto result = systemConfigDb->getSystemConfig();
    if (!result->isSuccess()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to get system config: %s", result->getErrorMessage().c_str());
        throw vsoa::Error(Status::CODE_500, result->getErrorMessage());
    }
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Fetched system config, rows=%d", result->getRowsChanged());
    return result->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
}

/**
 * 根据键获取系统配置
 */
vsoa::Object<SystemConfigDto> SystemConfigService::getSystemConfigByKey(const std::string& configKey) {
    auto result = systemConfigDb->getSystemConfigByKey({{"config_key", configKey}});
    if (!result->isSuccess()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to get system config by key=%s: %s", configKey.c_str(), result->getErrorMessage().c_str());
        throw vsoa::Error(Status::CODE_500, result->getErrorMessage());
    }
    auto configs = result->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    if (configs.empty()) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "System config not found for key=%s", configKey.c_str());
        throw vsoa::Error(Status::CODE_404, "System config not found");
    }
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Get system config key=%s", configKey.c_str());
    return configs[0];
}

/**
 * 更新系统配置
 */
vsoa::Object<SystemConfigDto> SystemConfigService::updateSystemConfig(const std::string& configKey, const std::string& configValue) {
    // 获取当前时间戳
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 更新系统配置
    auto result = systemConfigDb->updateSystemConfig({
        {"config_key", configKey},
        {"config_value", configValue},
        {"updated_at", timestamp}
    });
    
    if (!result->isSuccess()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to update system config key=%s value=%s: %s", configKey.c_str(), configValue.c_str(), result->getErrorMessage().c_str());
        throw vsoa::Error(Status::CODE_500, result->getErrorMessage());
    }
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Updated system config key=%s value=%s", configKey.c_str(), configValue.c_str());
    
    // 获取更新后的系统配置信息
    return getSystemConfigByKey(configKey);
}

/**
 * 获取NTP配置
 */
vsoa::Object<NtpConfigDto> SystemConfigService::getNtpConfig() {
    // 获取NTP服务器地址
    auto ntpServerResult = systemConfigDb->getSystemConfigByKey({{"config_key", "ntp_server"}});
    if (!ntpServerResult->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, ntpServerResult->getErrorMessage());
    }
    auto ntpServerConfigs = ntpServerResult->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string ntpServer = ntpServerConfigs.empty() ? "pool.ntp.org" : ntpServerConfigs[0]->config_value;
    
    // 获取NTP启用状态
    auto ntpEnabledResult = systemConfigDb->getSystemConfigByKey({{"config_key", "ntp_enabled"}});
    if (!ntpEnabledResult->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, ntpEnabledResult->getErrorMessage());
    }
    auto ntpEnabledConfigs = ntpEnabledResult->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    bool ntpEnabled = ntpEnabledConfigs.empty() ? false : (ntpEnabledConfigs[0]->config_value == "true");
    
    // 获取当前系统时间
    std::string currentTime = getCurrentSystemTime();
    
    // 构建NTP配置对象
    auto ntpConfig = vsoa::Object<NtpConfigDto>::createShared();
    ntpConfig->server = ntpServer;
    ntpConfig->enabled = ntpEnabled;
    ntpConfig->current_time = currentTime;
    ntpConfig->sync_status = ntpEnabled ? "Enabled" : "Disabled";
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "NTP config server=%s enabled=%s", ntpServer.c_str(), ntpEnabled ? "true" : "false");
    
    return ntpConfig;
}

/**
 * 更新NTP配置
 */
vsoa::Object<NtpConfigDto> SystemConfigService::updateNtpConfig(const std::string& server, bool enabled) {
    // 更新NTP服务器地址
    updateSystemConfig("ntp_server", server);
    
    // 更新NTP启用状态
    updateSystemConfig("ntp_enabled", enabled ? "true" : "false");
    
    // 获取更新后的NTP配置
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Updated NTP config server=%s enabled=%s", server.c_str(), enabled ? "true" : "false");
    return getNtpConfig();
}

/**
 * 获取网络配置
 */
vsoa::Object<NetworkConfigDto> SystemConfigService::getNetworkConfig() {
    // 获取网络模式
    auto networkModeResult = systemConfigDb->getSystemConfigByKey({{"config_key", "network_mode"}});
    if (!networkModeResult->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, networkModeResult->getErrorMessage());
    }
    auto networkModeConfigs = networkModeResult->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string networkMode = networkModeConfigs.empty() ? "dhcp" : networkModeConfigs[0]->config_value;
    
    // 获取IP地址
    auto ipAddressResult = systemConfigDb->getSystemConfigByKey({{"config_key", "network_ip"}});
    if (!ipAddressResult->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, ipAddressResult->getErrorMessage());
    }
    auto ipAddressConfigs = ipAddressResult->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string ipAddress = ipAddressConfigs.empty() ? "192.168.1.100" : ipAddressConfigs[0]->config_value;
    
    // 获取子网掩码
    auto subnetMaskResult = systemConfigDb->getSystemConfigByKey({{"config_key", "network_netmask"}});
    if (!subnetMaskResult->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, subnetMaskResult->getErrorMessage());
    }
    auto subnetMaskConfigs = subnetMaskResult->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string subnetMask = subnetMaskConfigs.empty() ? "255.255.255.0" : subnetMaskConfigs[0]->config_value;
    
    // 获取网关
    auto gatewayResult = systemConfigDb->getSystemConfigByKey({{"config_key", "network_gateway"}});
    if (!gatewayResult->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, gatewayResult->getErrorMessage());
    }
    auto gatewayConfigs = gatewayResult->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string gateway = gatewayConfigs.empty() ? "192.168.1.1" : gatewayConfigs[0]->config_value;
    
    // 获取首选DNS
    auto dns1Result = systemConfigDb->getSystemConfigByKey({{"config_key", "network_dns1"}});
    if (!dns1Result->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, dns1Result->getErrorMessage());
    }
    auto dns1Configs = dns1Result->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string dns1 = dns1Configs.empty() ? "8.8.8.8" : dns1Configs[0]->config_value;
    
    // 获取备用DNS
    auto dns2Result = systemConfigDb->getSystemConfigByKey({{"config_key", "network_dns2"}});
    if (!dns2Result->isSuccess()) {
        throw vsoa::Error(Status::CODE_500, dns2Result->getErrorMessage());
    }
    auto dns2Configs = dns2Result->fetch<vsoa::Vector<vsoa::Object<SystemConfigDto>>>();
    std::string dns2 = dns2Configs.empty() ? "8.8.4.4" : dns2Configs[0]->config_value;
    
    // 构建网络配置对象
    auto networkConfig = vsoa::Object<NetworkConfigDto>::create();
    networkConfig->mode = networkMode;
    networkConfig->ip_address = ipAddress;
    networkConfig->subnet_mask = subnetMask;
    networkConfig->gateway = gateway;
    networkConfig->dns1 = dns1;
    networkConfig->dns2 = dns2;
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Network config mode=%s ip=%s gateway=%s", networkMode.c_str(), ipAddress.c_str(), gateway.c_str());
    
    return networkConfig;
}

/**
 * 更新网络配置
 */
vsoa::Object<NetworkConfigDto> SystemConfigService::updateNetworkConfig(
    const std::string& mode,
    const std::string& ipAddress,
    const std::string& subnetMask,
    const std::string& gateway,
    const std::string& dns1,
    const std::string& dns2
) {
    // 更新网络模式
    updateSystemConfig("network_mode", mode);
    
    // 更新IP地址
    updateSystemConfig("network_ip", ipAddress);
    
    // 更新子网掩码
    updateSystemConfig("network_netmask", subnetMask);
    
    // 更新网关
    updateSystemConfig("network_gateway", gateway);
    
    // 更新首选DNS
    updateSystemConfig("network_dns1", dns1);
    
    // 更新备用DNS
    updateSystemConfig("network_dns2", dns2);
    
    // 获取更新后的网络配置
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Updated Network config mode=%s ip=%s netmask=%s gateway=%s dns1=%s dns2=%s", mode.c_str(), ipAddress.c_str(), subnetMask.c_str(), gateway.c_str(), dns1.c_str(), dns2.c_str());
    return getNetworkConfig();
}

/**
 * 重启网络服务
 */
bool SystemConfigService::restartNetworkService() {
    // 注意：实际系统中应使用合适的命令重启网络服务
    // 这里为了示例，直接返回成功
    // 示例命令：system("sudo systemctl restart networking");
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Restart network service requested (simulated)");
    return true;
}

/**
 * 重启系统
 */
bool SystemConfigService::restartSystem() {
    // 注意：实际系统中应使用合适的命令重启系统
    // 这里为了示例，直接返回成功
    // 示例命令：system("sudo reboot");
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Restart system requested (simulated)");
    return true;
}

/**
 * 获取当前系统时间
 */
std::string SystemConfigService::getCurrentSystemTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_time);
    
    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    
    return ss.str();
}
