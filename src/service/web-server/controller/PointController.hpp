/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/PointController.hpp
 * @Description: 点位控制器，用于处理点位相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef POINT_CONTROLLER_HPP
#define POINT_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/PointDto.hpp"
#include "service/PointService.hpp"

// 日志相关头文件
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 点位控制器
 * 处理点位相关的API请求，包括CRUD操作和点位值的读写
 */
class PointController : public vsoa::web::server::api::ApiController {
private:
    // 使用单例模式获取点位服务实例
    PointService& getPointService() { return PointService::getInstance(); }

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    PointController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<PointController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<PointController>(objectMapper);
    }

    /**
     * 获取所有点位
     * @return 点位列表
     */
    ENDPOINT_INFO(getPoints) {
        info->summary = "获取所有点位";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<PointPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/points", getPoints, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points called with page=%u, size=%u", 
            page.getValue(0), size.getValue(0));
        auto response = getPointService().getPoints(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points returned successfully with %u points", 
            response->size.getValue(0));
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取点位
     * @param id 点位ID
     * @return 点位信息
     */
    ENDPOINT_INFO(getPointById) {
        info->summary = "根据ID获取点位";
        info->pathParams.add<vsoa::UInt32>("id").description = "点位ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<PointDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/points/{id}", getPointById, PATH(vsoa::UInt32, id)) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points/{id} called with id=%d", id);
        auto point = getPointService().getPointById(id);
        if (!point) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found with id=%d", id);
            return createResponse(Status::CODE_404, "点位不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points/{id} returned successfully for id=%d", id);
        return createDtoResponse(Status::CODE_200, point);
    }

    /**
     * 获取所有点位（包含设备信息）
     * @return 点位列表
     */
    ENDPOINT_INFO(getPointsWithDevice) {
        info->summary = "获取所有点位（包含设备信息）";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<PointWithDevicePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/points-with-device", getPointsWithDevice, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points-with-device called with page=%u, size=%u", 
            page.getValue(0), size.getValue(0));
        auto response = getPointService().getPointsWithDevice(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points-with-device returned successfully with %u points", 
            response->size.getValue(0));
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取点位（包含设备信息）
     * @param id 点位ID
     * @return 点位信息
     */
    ENDPOINT_INFO(getPointWithDeviceById) {
        info->summary = "根据ID获取点位（包含设备信息）";
        info->pathParams.add<vsoa::UInt32>("id").description = "点位ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<PointWithDeviceDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/points-with-device/{id}", getPointWithDeviceById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points-with-device/{id} called with id=%d", id);
        auto point = getPointService().getPointWithDeviceById(id);
        if (!point) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found with id=%d", id);
            return createResponse(Status::CODE_404, "点位不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points-with-device/{id} returned successfully for id=%d", id);
        return createDtoResponse(Status::CODE_200, point);
    }

    /**
     * 创建新点位
     * @param pointDto 点位创建数据
     * @return 创建的点位信息
     */
    ENDPOINT_INFO(createPoint) {
        info->summary = "创建新点位";
        info->addConsumes<vsoa::Object<PointCreateDto>>("application/json");
        info->addResponse<vsoa::Object<PointDto>>(Status::CODE_201, "application/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/points", createPoint, 
        BODY_DTO(vsoa::Object<PointCreateDto>, pointDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] POST /api/points called to create point");
        auto point = getPointService().createPoint(pointDto);
        if (!point) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[PointController] Failed to create point");
            return createResponse(Status::CODE_400, "创建点位失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] Point created successfully");
        return createDtoResponse(Status::CODE_201, point);
    }

    /**
     * 更新点位
     * @param id 点位ID
     * @param pointDto 点位更新数据
     * @return 更新后的点位信息
     */
    ENDPOINT_INFO(updatePoint) {
        info->summary = "更新点位";
        info->pathParams.add<vsoa::UInt32>("id").description = "点位ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<PointDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/points/{id}", updatePoint, 
        PATH(vsoa::UInt32, id),
        BODY_DTO(vsoa::Object<PointDto>, pointDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] PUT /api/points/{id} called to update point with id=%d", id);
        // 确保DTO中的ID与路径参数一致
        pointDto->id = id;
        auto point = getPointService().updatePoint(pointDto);
        if (!point) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found for update with id=%d", id);
            return createResponse(Status::CODE_404, "点位不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] Point updated successfully with id=%d", id);
        return createDtoResponse(Status::CODE_200, point);
    }

    /**
     * 删除点位
     * @param id 点位ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deletePoint) {
        info->summary = "删除点位";
        info->pathParams.add<vsoa::UInt32>("id").description = "点位ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::String>(Status::CODE_204, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/points/{id}", deletePoint, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] DELETE /api/points/{id} called to delete point with id=%d", id);
        auto status = getPointService().deletePoint(id);
        if (status->code == 404) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found for deletion with id=%d", id);
            return createResponse(Status::CODE_404, status->message);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] Point deleted successfully with id=%d", id);
        return createResponse(Status::CODE_204, status->message);
    }

    // /**
    //  * 获取所有点位值
    //  * @return 点位值列表
    //  */
    // ENDPOINT_INFO(getPointValues) {
    //     info->summary = "获取所有点位值";
    //     info->queryParams.add<vsoa::Int32>("id").description = "点位ID";
    //     info->queryParams["id"].required = "true";   
    //     info->addResponse<vsoa::Object<PointPageDto>>(Status::CODE_200, "application/json");
    // }
    // ENDPOINT("GET", "/api/points/values", getPointValues, 
    //     QUERY(vsoa::Int32, id)) 
    // {
    //     auto response = pointService.getPointValues(id);
    //     return createDtoResponse(Status::CODE_200, response);
    // }

    /**
     * 获取点位值
     * @param name 点位名称
     * @return 点位值信息
     */
    ENDPOINT_INFO(getPointValue) {
        info->summary = "获取点位值";
        info->queryParams.add<vsoa::String>("name").description = "点位名称";
        info->queryParams["name"].required = "true";   
        info->addResponse<vsoa::Object<PointValueDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/points/value/", getPointValue, 
        QUERY(vsoa::String, name)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points/value/ called with name=%s", name->c_str());
        auto value = getPointService().getPointValue(name);
        if (!value) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found with name=%s", name->c_str());
            return createResponse(Status::CODE_404, "点位不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points/value/ returned successfully for name=%s", name->c_str());
        return createDtoResponse(Status::CODE_200, value);
    }

    /**
     * 根据ID获取点位值
     * @param id 点位ID
     * @return 点位值信息
     */
    ENDPOINT_INFO(getPointValueById) {
        info->summary = "根据ID获取点位值";
        info->queryParams.add<vsoa::UInt32>("id").description = "点位ID";
        info->queryParams["id"].required = "true";   
        info->addResponse<vsoa::Object<PointValueDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/points/value/by-id/{id}", getPointValueById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points/value/by-id/{id} called with id=%d", 
            id.getValue(-1));
        auto value = getPointService().getPointValueById(id);
        if (!value) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found with id=%d", 
                id.getValue(-1));
            return createResponse(Status::CODE_404, "点位不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] GET /api/points/value/by-id/{id} returned successfully for id=%d", 
            id.getValue(-1));
        return createDtoResponse(Status::CODE_200, value);
    }

    /**
     * 设置点位值
     * @param name 点位名称
     * @param writeDto 写入值数据
     * @return 写入后的点位值信息
     */
    ENDPOINT_INFO(setPointValue) {
        info->summary = "设置点位值";
        info->queryParams.add<vsoa::String>("name").description = "点位名称";
        info->queryParams["name"].required = "true";   
        info->addResponse<vsoa::Object<PointValueDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/points/value/", setPointValue, 
        QUERY(vsoa::String, name),
        BODY_DTO(vsoa::Object<PointWriteDto>, writeDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] POST /api/points/value/ called with name=%s", name->c_str());
        auto value = getPointService().setPointValue(name, writeDto);
        if (!value) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found or control not enabled for name=%s", name->c_str());
            return createResponse(Status::CODE_404, "点位不存在或控制未启用");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] Point value set successfully for name=%s", name->c_str());
        return createDtoResponse(Status::CODE_200, value);
    }

    /**
     * 根据ID设置点位值
     * @param id 点位ID
     * @param writeDto 写入值数据
     * @return 写入后的点位值信息
     */
    ENDPOINT_INFO(setPointValueById) {
        info->summary = "根据ID设置点位值";
        info->queryParams.add<vsoa::UInt32>("id").description = "点位ID";
        info->queryParams["id"].required = "true";   
        info->addResponse<vsoa::Object<PointValueDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");

        info->queryParams["id"].name = "点位ID";
    }
    ENDPOINT("POST", "/api/points/value/by-id/", setPointValueById, 
        QUERY(vsoa::UInt32, id),
        BODY_DTO(vsoa::Object<PointWriteDto>, writeDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] POST /api/points/value/by-id/ called with id=%d", id);
        auto value = getPointService().setPointValueById(id, writeDto);
        if (!value) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[PointController] Point not found or control not enabled for id=%d", id);
            return createResponse(Status::CODE_404, "点位不存在或控制未启用");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[PointController] Point value set successfully for id=%d", id);
        return createDtoResponse(Status::CODE_200, value);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif // POINT_CONTROLLER_HPP