#pragma once

#include "db/UserDb.hpp"
#include "dto/UserDto.hpp"
#include "dto/StatusDto.hpp"
#include "vsoa_dto/core/Types.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 * 系统配置服务类
 */
class SystemConfigService {
private:
    typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<SystemConfigDb>, systemConfigDb);

public:
    /**
     * 获取所有系统配置
     */
    vsoa::Vector<vsoa::Object<SystemConfigDto>> getSystemConfig();

    /**
     * 根据键获取系统配置
     */
    vsoa::Object<SystemConfigDto> getSystemConfigByKey(const std::string& configKey);

    /**
     * 更新系统配置
     */
    vsoa::Object<SystemConfigDto> updateSystemConfig(const std::string& configKey, const std::string& configValue);

    /**
     * 获取NTP配置
     */
    vsoa::Object<NtpConfigDto> getNtpConfig();

    /**
     * 更新NTP配置
     */
    vsoa::Object<NtpConfigDto> updateNtpConfig(const std::string& server, bool enabled);

    /**
     * 获取网络配置
     */
    vsoa::Object<NetworkConfigDto> getNetworkConfig();

    /**
     * 更新网络配置
     */
    vsoa::Object<NetworkConfigDto> updateNetworkConfig(
        const std::string& mode,
        const std::string& ipAddress,
        const std::string& subnetMask,
        const std::string& gateway,
        const std::string& dns1,
        const std::string& dns2
    );

    /**
     * 重启网络服务
     */
    bool restartNetworkService();

    /**
     * 重启系统
     */
    bool restartSystem();

    /**
     * 获取当前系统时间
     */
    std::string getCurrentSystemTime();
};
