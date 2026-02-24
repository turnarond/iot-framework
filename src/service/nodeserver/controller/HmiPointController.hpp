/*
 * @Author: yanchaodong
 * @Date: 2026-02-09 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-09 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/nodeserver/controller/HmiPointController.hpp
 * @Description: HMI点位控制器，用于处理HMI接口的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef HMI_POINT_CONTROLLER_HPP
#define HMI_POINT_CONTROLLER_HPP

#include "service/HmiPointService.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/HmiPointDto.hpp"

// 日志相关头文件
#include <lwlog/lwlog.h>

extern CLWLog g_logger;

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * HMI点位控制器
 * 处理HMI接口的API请求，包括批量点位查询、前缀查询和控制命令下发
 */
class HmiPointController : public vsoa::web::server::api::ApiController 
{
public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    HmiPointController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

private:
    HmiPointService hmiPointService; ///< HMI点位服务实例，用于处理业务逻辑

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<HmiPointController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<HmiPointController>(objectMapper);
    }

    /**
     * 批量点位查询
     * @param requestDto 批量查询请求
     * @return 批量查询响应
     */
    ENDPOINT_INFO(batchQueryPoints) {
        info->summary = "批量点位查询";
        info->addConsumes<vsoa::Object<HmiBatchPointsRequestDto>>("application/json");
        info->addResponse<vsoa::Object<HmiBatchPointsResponseDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/v1/points/batch", batchQueryPoints, 
        BODY_DTO(vsoa::Object<HmiBatchPointsRequestDto>, requestDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiPointController] POST /api/v1/points/batch called");
        auto response = hmiPointService.batchQueryPoints(requestDto);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiPointController] Batch query completed successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 前缀查询点位
     * @param prefix 点位前缀
     * @return 符合前缀的点位值映射
     */
    ENDPOINT_INFO(queryPointsByPrefix) {
        info->summary = "前缀查询点位";
        info->queryParams.add<vsoa::String>("prefix").description = "点位前缀";
        info->queryParams["prefix"].required = "true";
        info->addResponse<vsoa::Object<HmiBatchPointsResponseDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("GET", "/api/v1/points", queryPointsByPrefix, 
        QUERY(vsoa::String, prefix)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiPointController] GET /api/v1/points called with prefix=%s", prefix ? prefix->c_str() : "null");
        auto response = hmiPointService.queryPointsByPrefix(prefix);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiPointController] Prefix query completed successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 下发控制命令
     * @param controlDto 控制命令
     * @return 控制响应
     */
    ENDPOINT_INFO(sendControlCommand) {
        info->summary = "下发控制命令";
        info->addConsumes<vsoa::Object<HmiControlCommandDto>>("application/json");
        info->addResponse<vsoa::Object<HmiControlResponseDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/v1/control", sendControlCommand, 
        BODY_DTO(vsoa::Object<HmiControlCommandDto>, controlDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiPointController] POST /api/v1/control called");
        auto response = hmiPointService.sendControlCommand(controlDto);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiPointController] Control command processed, success=%s", response->success ? "true" : "false");
        return createDtoResponse(Status::CODE_200, response);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif // HMI_POINT_CONTROLLER_HPP