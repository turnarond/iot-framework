/*
 * @Author: yanchaodong
 * @Date: 2026-02-06 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-06 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/SystemConfigController.hpp
 * @Description: 系统配置控制器，用于处理系统配置相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "service/SystemConfigService.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/UserDto.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 系统配置控制器
 * 处理系统配置相关的HTTP请求
 */
class SystemConfigController : public vsoa::web::server::api::ApiController {

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    SystemConfigController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

    static std::shared_ptr<SystemConfigController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    ) {
        return std::make_shared<SystemConfigController>(objectMapper);
    }

private:
    SystemConfigService systemConfigService; ///< 系统配置服务实例，用于处理业务逻辑

    /**
     * 获取所有系统配置
     */
    ENDPOINT_INFO(getSystemConfig) {
        info->summary = "获取所有系统配置";
        info->addResponse<vsoa::Vector<vsoa::Object<SystemConfigDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/system/config", getSystemConfig) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/config called");
        auto response = systemConfigService.getSystemConfig();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/config returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    // /**
    //  * 根据键获取系统配置
    //  */
    // ENDPOINT_INFO(getSystemConfigByKey) {
    //     info->summary = "根据键获取系统配置";
    //     info->addPathParam<vsoa::String>("configKey", "配置键");
    //     info->addResponse<vsoa::Object<SystemConfigDto>>(Status::CODE_200, "application/json");
    //     info->addResponse(Status::CODE_404, "System config not found");
    // }
    // ENDPOINT("GET", "/api/system/config/{configKey}", getSystemConfigByKey, PATH(vsoa::String, configKey)) {
    //     g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/config/{configKey} called with configKey=%s", configKey->c_str());
    //     auto response = systemConfigService.getSystemConfigByKey(configKey);
    //     g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/config/{configKey} returned successfully for configKey=%s", configKey->c_str());
    //     return createDtoResponse(Status::CODE_200, response);
    // }

    /**
     * 更新系统配置
     */
    ENDPOINT_INFO(updateSystemConfig) {
        info->summary = "更新系统配置";
        info->queryParams.add<vsoa::String>("configKey").description = "配置键";
        info->addResponse<vsoa::Object<SystemConfigDto>>(Status::CODE_200, "application/json");
        info->addResponse(Status::CODE_400, "Invalid request");
    }
    ENDPOINT("PUT", "/api/system/config/", updateSystemConfig, 
        QUERY(vsoa::String, configKey), 
        BODY_DTO(vsoa::Object<SystemConfigRequestDto>, configValue)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] PUT /api/system/config/ called with configKey=%s", configKey->c_str());
        auto response = systemConfigService.updateSystemConfig(configKey, configValue->config_value);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] PUT /api/system/config/ returned successfully for configKey=%s", configKey->c_str());
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取NTP配置
     */
    ENDPOINT_INFO(getNtpConfig) {
        info->summary = "获取NTP配置";
        info->addResponse<vsoa::Object<NtpConfigDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/system/ntp", getNtpConfig) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/ntp called");
        auto response = systemConfigService.getNtpConfig();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/ntp returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 更新NTP配置
     */
    ENDPOINT_INFO(updateNtpConfig) {
        info->summary = "更新NTP配置";
        info->addResponse<vsoa::Object<NtpConfigDto>>(Status::CODE_200, "application/json");
        info->addResponse(Status::CODE_400, "Invalid request");
    }
    ENDPOINT("PUT", "/api/system/ntp", updateNtpConfig, 
        BODY_DTO(vsoa::Object<NtpConfigDto>, ntpConfig)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] PUT /api/system/ntp called");
        auto response = systemConfigService.updateNtpConfig(ntpConfig->server, ntpConfig->enabled);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] PUT /api/system/ntp returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取网络配置
     */
    ENDPOINT_INFO(getNetworkConfig) {
        info->summary = "获取网络配置";
        info->addResponse<vsoa::Object<NetworkConfigDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/system/network", getNetworkConfig) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/network called");
        auto response = systemConfigService.getNetworkConfig();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] GET /api/system/network returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 更新网络配置
     */
    ENDPOINT_INFO(updateNetworkConfig) {
        info->summary = "更新网络配置";
        info->addResponse<vsoa::Object<NetworkConfigDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("PUT", "/api/system/network", updateNetworkConfig, 
        BODY_DTO(vsoa::Object<NetworkConfigDto>, networkConfig))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] PUT /api/system/network called");
        auto response = systemConfigService.updateNetworkConfig(
            networkConfig->mode,
            networkConfig->ip_address,
            networkConfig->subnet_mask,
            networkConfig->gateway,
            networkConfig->dns1,
            networkConfig->dns2
        );
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] PUT /api/system/network returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    // /**
    //  * 重启网络服务
    //  */
    // ENDPOINT_INFO(restartNetworkService) {
    //     info->summary = "重启网络服务";
    //     info->addResponse<vsoa::Object<vsoa::DTO>>(Status::CODE_200, "application/json");
    // }
    // ENDPOINT("POST", "/api/system/restart-network", restartNetworkService) {
    //     g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] POST /api/system/restart-network called");
    //     bool result = systemConfigService.restartNetworkService();
    //     auto response = vsoa::Object<vsoa::DTO>::create();
    //     response->put("success", result);
    //     g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] POST /api/system/restart-network returned successfully with result=%d", result);
    //     return createDtoResponse(Status::CODE_200, response);
    // }

    // /**
    //  * 重启系统
    //  */
    // ENDPOINT_INFO(restartSystem) {
    //     info->summary = "重启系统";
    //     info->addResponse<vsoa::Object<vsoa::DTO>>(Status::CODE_200, "application/json");
    // }
    // ENDPOINT("POST", "/api/system/restart", restartSystem) {
    //     g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] POST /api/system/restart called");
    //     bool result = systemConfigService.restartSystem();
    //     auto response = vsoa::Object<vsoa::DTO>::create();
    //     response->put("success", result);
    //     g_logger.LogMessage(LW_LOGLEVEL_INFO, "[SystemConfigController] POST /api/system/restart returned successfully with result=%d", result);
    //     return createDtoResponse(Status::CODE_200, response);
    // }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen
