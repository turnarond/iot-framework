/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/DeviceController.hpp
 * @Description: 设备控制器，用于处理设备和驱动相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DEVICE_CONTROLLER_HPP
#define DEVICE_CONTROLLER_HPP

#include "service/DeviceService.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/PageDto.hpp"
#include "dto/DeviceDto.hpp"
#include "common/Logger.hpp"
#include <any>


#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 设备控制器
 * 处理设备和驱动相关的API请求，包括CRUD操作
 */
class DeviceController : public vsoa::web::server::api::ApiController {

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    DeviceController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}
private:
    // 使用单例模式获取设备服务实例
    DeviceService& getDeviceService() { return DeviceService::getInstance(); }

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<DeviceController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    ) {
        return std::make_shared<DeviceController>(objectMapper);
    }

    // 设备相关端点
    /**
     * 获取设备列表（分页）
     * @param page 页码
     * @param size 每页大小
     * @return 设备列表
     */
    ENDPOINT_INFO(getDevices) {
        info->summary = "获取设备列表（分页）";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码，从1开始";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页大小，默认10";
        info->addResponse<vsoa::Object<PageDto<vsoa::Object<DeviceWithDriverDto>>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/devices", getDevices, 
        QUERY(vsoa::UInt32, page, "page"), 
        QUERY(vsoa::UInt32, size, "size")) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] GET /api/devices called with page=%u, size=%u",
            page.getValue(0), size.getValue(0));
        auto response = getDeviceService().getDevices(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] GET /api/devices returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取设备
     * @param id 设备ID
     * @return 设备信息
     */
    ENDPOINT_INFO(getDeviceById) {
        info->summary = "根据ID获取设备";
        info->pathParams.add<vsoa::UInt32>("id").description = "设备ID";
        info->pathParams["id"].addExample("id", vsoa::Any{vsoa::UInt32(1)});
        info->addResponse<vsoa::Object<DeviceWithDriverDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/devices/{id}", getDeviceById, 
        PATH(vsoa::UInt32, id, "id")) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] GET /api/devices/{id} called with id=%d", 
            id.getValue(0));
        auto device = getDeviceService().getDeviceById(id.getValue(0));
        if (!device) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[DeviceController] Device not found with id=%d", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "设备不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] GET /api/devices/{id} returned successfully for id=%d", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, device);
    }

    /**
     * 创建新设备
     * @param deviceDto 设备创建数据
     * @return 创建的设备信息
     */
    ENDPOINT_INFO(createDevice) {
        info->summary = "创建新设备";
        info->addConsumes<vsoa::Object<DeviceCreateDto>>("application/json");
        info->addResponse<vsoa::Object<DeviceDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/devices", createDevice, BODY_DTO(vsoa::Object<DeviceCreateDto>, deviceDto)) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] POST /api/devices called to create device");
        auto device = getDeviceService().createDevice(deviceDto);
        if (!device) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[DeviceController] Failed to create device");
            return createResponse(Status::CODE_400, "创建设备失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] Device created successfully");
        return createDtoResponse(Status::CODE_201, device);
    }

    /**
     * 更新设备
     * @param id 设备ID
     * @param deviceDto 设备更新数据
     * @return 更新后的设备信息
     */
    ENDPOINT_INFO(updateDevice) {
        info->summary = "更新设备";
        info->addConsumes<vsoa::Object<DeviceDto>>("application/json");
        info->addResponse<vsoa::Object<DeviceDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/devices", updateDevice, 
        BODY_DTO(vsoa::Object<DeviceDto>, deviceDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] PUT /api/devices called to update device with id=%d", 
            deviceDto->id.getValue(0));
        auto device = getDeviceService().updateDevice(deviceDto);
        if (!device) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[DeviceController] Device not found for update with id=%d", 
                deviceDto->id.getValue(0));
            return createResponse(Status::CODE_404, "设备不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] Device updated successfully with id=%d", 
            deviceDto->id.getValue(0));
        return createDtoResponse(Status::CODE_200, device);
    }

    /**
     * 删除设备
     * @param id 设备ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteDevice) {
        info->summary = "删除设备";
        info->pathParams.add<vsoa::UInt32>("id").description = "设备ID";
        info->pathParams["id"].addExample("id", vsoa::Any{vsoa::UInt32(1)});
        info->addResponse<vsoa::String>(Status::CODE_204, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/devices/{id}", deleteDevice, 
        PATH(vsoa::UInt32, id, "id")) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] DELETE /api/devices/{id} called with id=%d", 
            id.getValue(0));
        if (!getDeviceService().deleteDevice(id.getValue(0))) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[DeviceController] Device not found for deletion with id=%d", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "设备不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DeviceController] Device deleted successfully with id=%d", 
            id.getValue(0));
        return createResponse(Status::CODE_204, "设备已删除");
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif // DEVICE_CONTROLLER_HPP