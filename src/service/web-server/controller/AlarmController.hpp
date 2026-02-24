/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/AlarmController.hpp
 * @Description: 告警控制器，用于处理告警规则和告警相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef ALARM_CONTROLLER_HPP
#define ALARM_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/AlarmDto.hpp"
#include "service/AlarmService.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 告警控制器
 * 处理告警规则和告警相关的API请求，包括CRUD操作
 */
class AlarmController : public vsoa::web::server::api::ApiController {
private:
    // 使用单例模式获取告警服务实例
    AlarmService& getAlarmService() { return AlarmService::getInstance(); }

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    AlarmController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<AlarmController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<AlarmController>(objectMapper);
    }

    // 告警规则相关端点
    /**
     * 获取所有告警规则
     * @return 告警规则列表
     */
    ENDPOINT_INFO(getAlarmRules) {
        info->summary = "获取所有告警规则";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->addResponse<vsoa::Object<AlarmRulePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/alarm-rules", getAlarmRules, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules called with page=%u, size=%u",
            page.getValue(0), size.getValue(0));
        auto response = getAlarmService().getAlarmRules(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取所有告警规则（包含点位信息）
     * @return 告警规则列表
     */
    ENDPOINT_INFO(getAlarmRulesWithPoint) {
        info->summary = "获取所有告警规则（包含点位信息）";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->addResponse<vsoa::Object<AlarmRuleWithPointPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/alarm-rules/with-point", getAlarmRulesWithPoint, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/with-point called with page=%u, size=%u",
            page.getValue(0), size.getValue(0));
        auto response = getAlarmService().getAlarmRulesWithPoint(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/with-point returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取告警规则
     * @param id 告警规则ID
     * @return 告警规则信息
     */
    ENDPOINT_INFO(getAlarmRuleById) {
        info->summary = "根据ID获取告警规则";
        info->pathParams.add<vsoa::UInt32>("id").description = "告警规则ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<AlarmRuleDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/alarm-rules/{id}", getAlarmRuleById, 
        PATH(vsoa::UInt32, id))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/{id} called with id=%u", 
            id.getValue(0));
        auto rule = getAlarmService().getAlarmRuleById(id);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[AlarmController] Alarm rule not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "告警规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/{id} returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, rule);
    }

    /**
     * 根据ID获取告警规则（包含点位信息）
     * @param id 告警规则ID
     * @return 告警规则信息
     */
    ENDPOINT_INFO(getAlarmRuleWithPointById) {
        info->summary = "根据ID获取告警规则（包含点位信息）";
        info->pathParams.add<vsoa::UInt32>("id").description = "告警规则ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<AlarmRuleWithPointDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/alarm-rules/with-point/{id}", getAlarmRuleWithPointById, 
        PATH(vsoa::UInt32, id))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/with-point/{id} called with id=%u", 
            id.getValue(0));
        auto rule = getAlarmService().getAlarmRuleWithPointById(id);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[AlarmController] Alarm rule not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "告警规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/with-point/{id} returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, rule);
    }

    /**
     * 创建新告警规则
     * @param ruleDto 告警规则创建数据
     * @return 创建的告警规则信息
     */
    ENDPOINT_INFO(createAlarmRule) {
        info->summary = "创建新告警规则";
        info->addConsumes<vsoa::Object<AlarmRuleCreateDto>>("application/json");
        info->addResponse<vsoa::Object<AlarmRuleDto>>(Status::CODE_201, "application/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/alarm-rules", createAlarmRule, BODY_DTO(vsoa::Object<AlarmRuleCreateDto>, ruleDto)) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] POST /api/alarm-rules called to create alarm rule");
        auto rule = getAlarmService().createAlarmRule(ruleDto);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[AlarmController] Failed to create alarm rule");
            return createResponse(Status::CODE_400, "创建告警规则失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] Alarm rule created successfully");
        return createDtoResponse(Status::CODE_201, rule);
    }

    /**
     * 更新告警规则
     * @param id 告警规则ID
     * @param ruleDto 告警规则更新数据
     * @return 更新后的告警规则信息
     */
    ENDPOINT_INFO(updateAlarmRule) {
        info->summary = "更新告警规则";
        info->addConsumes<vsoa::Object<AlarmRuleDto>>("application/json");
        info->addResponse<vsoa::Object<AlarmRuleDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/alarm-rules/", updateAlarmRule, 
        BODY_DTO(vsoa::Object<AlarmRuleDto>, ruleDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] PUT /api/alarm-rules/ called to update alarm rule");
        auto rule = getAlarmService().updateAlarmRule(ruleDto);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[AlarmController] Alarm rule not found for update");
            return createResponse(Status::CODE_404, "告警规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] Alarm rule updated successfully");
        return createDtoResponse(Status::CODE_200, rule);
    }

    /**
     * 删除告警规则
     * @param id 告警规则ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteAlarmRule) {
        info->summary = "删除告警规则";
        info->pathParams.add<vsoa::UInt32>("id").description = "告警规则ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/alarm-rules/{id}", deleteAlarmRule, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] DELETE /api/alarm-rules/{id} called with id=%u", 
            id.getValue(0));
        auto status = getAlarmService().deleteAlarmRule(id);
        if (status->code != 200) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[AlarmController] Failed to delete alarm rule with id=%u: %s", 
                id.getValue(0), status->message->c_str());
            return createResponse(Status::CODE_404, status->message);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] Alarm rule deleted successfully with id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_204, status);
    }

    /**
     * 获取所有告警
     * @return 告警列表
     */
    ENDPOINT_INFO(getAlarms) {
        info->summary = "获取所有告警";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<AlarmPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/alarms/", getAlarms, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarms/ called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getAlarmService().getAlarms(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarms/ returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 获取活跃告警
     * @return 活跃告警列表
     */
    ENDPOINT_INFO(getActiveAlarms) {
        info->summary = "获取活跃告警";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<AlarmPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/alarms/active", getActiveAlarms, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarms/active called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getAlarmService().getActiveAlarms(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarms/active returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据点位ID获取报警规则
     * @param pointId 点位ID
     * @return 报警规则列表
     */
    ENDPOINT_INFO(getAlarmRulesByPointId) {
        info->summary = "根据点位ID获取报警规则";
        info->pathParams.add<vsoa::UInt32>("pointId").description = "点位ID";
        info->pathParams["pointId"].required = "true";
        info->addResponse<vsoa::Vector<vsoa::Object<AlarmRuleWithPointDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/alarm-rules/by-point/{pointId}", getAlarmRulesByPointId, 
        PATH(vsoa::UInt32, pointId)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/by-point/{pointId} called with pointId=%u", 
            pointId.getValue(0));
        auto response = getAlarmService().getAlarmRulesByPointId(pointId);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[AlarmController] GET /api/alarm-rules/by-point/{pointId} returned successfully for pointId=%u", 
            pointId.getValue(0));
        return createDtoResponse(Status::CODE_200, response);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif // ALARM_CONTROLLER_HPP