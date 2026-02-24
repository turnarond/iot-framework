/*
 * @Author: yanchaodong
 * @Date: 2026-02-15 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-15 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/server_mgr/src/controller/ServiceManagerController.hpp
 * @Description: Service Manager Controller for API endpoints
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef SERVICE_MANAGER_CONTROLLER_HPP
#define SERVICE_MANAGER_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "../dto/StatusDto.hpp"
#include "../dto/ServiceInfoDto.hpp"
#include "../service/ServiceManagerService.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 服务管理器控制器
 * 处理服务管理相关的API请求
 */
class ServiceManagerController : public vsoa::web::server::api::ApiController {
private:
    ServiceManagerService serviceManagerService; ///< 服务管理器服务实例，用于处理业务逻辑

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     * @param serverManager 服务器管理器实例
     */
    ServiceManagerController(const std::shared_ptr<vsoa::data::mapping::ObjectMapper>& objectMapper, ServerManager& serverManager)
        : vsoa::web::server::api::ApiController(objectMapper)
        , serviceManagerService(serverManager)
    {}

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @param serverManager 服务器管理器实例
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<ServiceManagerController> createShared(
        const std::shared_ptr<vsoa::data::mapping::ObjectMapper>& objectMapper,
        ServerManager& serverManager) {
        return std::make_shared<ServiceManagerController>(objectMapper, serverManager);
    }

    // 服务管理相关端点
    /**
     * 获取服务列表
     * @return 服务名称列表
     */
    ENDPOINT_INFO(getServices) {
        info->summary = "获取服务列表";
        info->addResponse<vsoa::List<vsoa::Object<ServiceInfoDto>>>(vsoa::web::protocol::http::Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/services", getServices) {
        auto services = serviceManagerService.getServices();
        return createDtoResponse(vsoa::web::protocol::http::Status::CODE_200, services);
    }

    /**
     * 启动服务
     * @param serviceName 服务名称
     * @return 操作结果
     */
    ENDPOINT_INFO(startService) {
        info->summary = "启动服务";
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_200, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_400, "application/json");
    }
    ENDPOINT("POST", "/services/start/{serviceName}", startService, 
        PATH(vsoa::String, serviceName)) {
        auto status = serviceManagerService.startService(serviceName);
        return createDtoResponse(vsoa::web::protocol::http::Status::CODE_200, status);
    }

    /**
     * 停止服务
     * @param serviceName 服务名称
     * @return 操作结果
     */
    ENDPOINT_INFO(stopService) {
        info->summary = "停止服务";
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_200, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_400, "application/json");
    }
    ENDPOINT("POST", "/services/stop/{serviceName}", stopService, 
        PATH(vsoa::String, serviceName)) {
        auto status = serviceManagerService.stopService(serviceName);
        return createDtoResponse(vsoa::web::protocol::http::Status::CODE_200, status);
    }

    /**
     * 重启服务
     * @param serviceName 服务名称
     * @return 操作结果
     */
    ENDPOINT_INFO(restartService) {
        info->summary = "重启服务";
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_200, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_400, "application/json");
    }
    ENDPOINT("POST", "/services/restart/{serviceName}", restartService, 
        PATH(vsoa::String, serviceName)) {
        auto status = serviceManagerService.restartService(serviceName);
        return createDtoResponse(vsoa::web::protocol::http::Status::CODE_200, status);
    }
    
    /**
     * 重新加载配置
     * @return 操作结果
     */
    ENDPOINT_INFO(reloadConfig) {
        info->summary = "重新加载配置";
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_200, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(vsoa::web::protocol::http::Status::CODE_400, "application/json");
    }
    ENDPOINT("POST", "/services/reload", reloadConfig) {
        auto status = serviceManagerService.reloadConfig();
        return createDtoResponse(vsoa::web::protocol::http::Status::CODE_200, status);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif /* SERVICE_MANAGER_CONTROLLER_HPP */
