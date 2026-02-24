/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/DriverController.hpp
 * @Description: Driver Controller Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DRIVER_CONTROLLER_HPP
#define DRIVER_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/DriverDto.hpp"
#include "dto/PageDto.hpp"
#include "service/DriverService.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 驱动控制器
 * 处理驱动相关的API请求
 */
class DriverController : public vsoa::web::server::api::ApiController {
private:
    DriverService driverService; ///< 驱动服务实例，用于处理业务逻辑

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    DriverController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<DriverController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<DriverController>(objectMapper);
    }

    // 驱动相关端点
    /**
     * 获取驱动列表（分页）
     * @param page 页码
     * @param size 每页大小
     * @return 驱动列表分页信息
     */
    ENDPOINT_INFO(getDrivers) {
        info->summary = "获取驱动列表（分页）";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页大小";
        info->addResponse<vsoa::Object<DriversPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/drivers", getDrivers, 
        QUERY(vsoa::UInt32, page), 
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] GET /api/drivers called with page=%u, size=%u", 
            page.getValue(0), size.getValue(0));
        auto response = driverService.getDrivers(page, size); 
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] GET /api/drivers returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取驱动
     * @param id 驱动ID
     * @return 驱动信息
     */
    ENDPOINT_INFO(getDriverById) {
        info->summary = "根据ID获取驱动";
        info->pathParams.add<vsoa::UInt32>("id").description = "驱动ID";
        info->addResponse<vsoa::Object<DriverDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/drivers/{id}", getDriverById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] GET /api/drivers/{id} called with id=%d", 
            id.getValue(-1));
        auto driver = driverService.getDriverById(id);
        if (!driver) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[DriverController] Driver not found with id=%d", 
                id.getValue(-1));
            return createResponse(Status::CODE_404, "驱动不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] GET /api/drivers/{id} returned successfully for id=%d", 
            id.getValue(-1));
        return createDtoResponse(Status::CODE_200, driver);
    }

    /**
     * 创建驱动
     * @param driverDto 驱动创建数据
     * @return 创建的驱动信息
     */
    ENDPOINT_INFO(createDriver) {
        info->summary = "创建驱动";
        info->addResponse<vsoa::Object<DriverCreateDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/drivers", createDriver, 
        BODY_DTO(vsoa::Object<DriverCreateDto>, driverDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] POST /api/drivers called to create driver");
        auto driver = driverService.createDriver(driverDto);
        if (!driver) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[DriverController] Failed to create driver");
            return createResponse(Status::CODE_400, "创建驱动失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] Driver created successfully");
        return createDtoResponse(Status::CODE_201, driver);
    }

    /**
     * 更新驱动
     * @param id 驱动ID
     * @param driverDto 驱动更新数据
     * @return 更新后的驱动信息
     */
    ENDPOINT_INFO(updateDriver) {
        info->summary = "更新驱动";
        info->addResponse<vsoa::Object<DriverDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/drivers", updateDriver, 
        BODY_DTO(vsoa::Object<DriverDto>, driverDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] PUT /api/drivers called to update driver with id=%d", driverDto->id);
        auto driver = driverService.updateDriver(driverDto);
        if (!driver) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[DriverController] Driver not found for update with id=%d", driverDto->id);
            return createResponse(Status::CODE_404, "驱动不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] Driver updated successfully with id=%d", driverDto->id);
        return createDtoResponse(Status::CODE_200, driver);
    }

    /**
     * 删除驱动
     * @param id 驱动ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteDriver) {
        info->summary = "删除驱动";
        info->pathParams.add<vsoa::UInt32>("id").description = "驱动ID";
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_404, "application/json");
    }
    ENDPOINT("DELETE", "/api/drivers/{id}", deleteDriver, PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] DELETE /api/drivers/{id} called with id=%d", 
            id.getValue(-1));
        auto status = driverService.deleteDriver(id);
        if (status->code == 404) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[DriverController] Driver not found for deletion with id=%d", 
                id.getValue(-1));
            return createDtoResponse(Status::CODE_404, status);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DriverController] Driver deleted successfully with id=%d", 
            id.getValue(-1));
        return createDtoResponse(Status::CODE_204, status);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif /* DRIVER_CONTROLLER_HPP */
