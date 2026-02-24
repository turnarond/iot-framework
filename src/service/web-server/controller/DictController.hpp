/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/DictController.hpp
 * @Description: 字典数据控制器，用于处理字典数据相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "lwlog/lwlog.h"
#include "service/DictService.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/DictDto.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 字典数据控制器
 * 处理字典数据相关的HTTP请求
 */
class DictController : public vsoa::web::server::api::ApiController {

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    DictController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {
    }

    static std::shared_ptr<DictController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<DictController>(objectMapper);
    }

private:
    DictService dictService; ///< 字典服务实例，用于处理业务逻辑

    /**
     * 获取所有数据类型
     */
    ENDPOINT_INFO(getDataTypes)
    {
        info->summary = "获取所有数据类型";
        info->addResponse<vsoa::Vector<vsoa::Object<DictDataTypeDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/data-types", getDataTypes)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/data-types called");
        auto response = dictService.getDataTypes();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/data-types returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取所有点位类型
     */
    ENDPOINT_INFO(getPointTypes)
    {
        info->summary = "获取所有点位类型";
        info->addResponse<vsoa::Vector<vsoa::Object<DictPointTypeDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/point-types", getPointTypes)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/point-types called");
        auto response = dictService.getPointTypes();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/point-types returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取所有传输方法
     */
    ENDPOINT_INFO(getTransferMethods)
    {
        info->summary = "获取所有传输方法";
        info->addResponse<vsoa::Vector<vsoa::Object<DictTransferTypeDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/transfer-methods", getTransferMethods)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/transfer-methods called");
        auto response = dictService.getTransferMethods();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/transfer-methods returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取所有报警方法
     */
    ENDPOINT_INFO(getAlarmMethods)
    {
        info->summary = "获取所有报警方法";
        info->addResponse<vsoa::Vector<vsoa::Object<DictAlarmMethodDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/alarm-methods", getAlarmMethods)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/alarm-methods called");
        auto response = dictService.getAlarmMethods();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/alarm-methods returned successfully");
        return createDtoResponse(Status::CODE_200, response);  
    }

    /**
     * 获取所有驱动类型
     */
    ENDPOINT_INFO(getDriverTypes)
    {
        info->summary = "获取所有驱动类型";
        info->addResponse<vsoa::Vector<vsoa::Object<DictDriverTypeDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/driver-types", getDriverTypes)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/driver-types called");
        auto response = dictService.getDriverTypes();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/driver-types returned successfully");
        return createDtoResponse(Status::CODE_200, response);  
    }

    /**
     * 获取所有连接类型
     */
    ENDPOINT_INFO(getConnTypes)
    {
        info->summary = "获取所有连接类型";
        info->addResponse<vsoa::Vector<vsoa::Object<DictConnTypeDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/conn-types", getConnTypes)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/conn-types called");
        auto response = dictService.getConnTypes();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/conn-types returned successfully");
        return createDtoResponse(Status::CODE_200, response);  
    }

    /**
     * 获取所有取流协议
     */
    ENDPOINT_INFO(getStreamProtocols)
    {
        info->summary = "获取所有取流协议";
        info->addResponse<vsoa::Vector<vsoa::Object<DictStreamProtocolDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/stream-protocols", getStreamProtocols)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/stream-protocols called");
        auto response = dictService.getStreamProtocols();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/stream-protocols returned successfully");
        return createDtoResponse(Status::CODE_200, response);  
    }

    /**
     * 获取所有控制协议
     */
    ENDPOINT_INFO(getControlProtocols)
    {
        info->summary = "获取所有控制协议";
        info->addResponse<vsoa::Vector<vsoa::Object<DictControlProtocolDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/dict/control-protocols", getControlProtocols)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/control-protocols called");
        auto response = dictService.getControlProtocols();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[DictController] GET /api/dict/control-protocols returned successfully");
        return createDtoResponse(Status::CODE_200, response);  
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen
