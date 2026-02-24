/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/LinkageController.hpp
 * @Description: 联动控制器，用于处理联动规则和相关实体的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef LINKAGE_CONTROLLER_HPP
#define LINKAGE_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"
#include "vsoa_dto/core/data/mapping/type/Type.hpp"

#include "dto/LinkageDto.hpp"
#include "service/LinkageService.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 联动控制器
 * 处理联动规则和相关实体的API请求，包括事件类型、触发源、动作类型、动作实例、联动规则和联动日志
 */
class LinkageController : public vsoa::web::server::api::ApiController {
private:
    // 使用单例模式获取联动服务实例
    LinkageService& getLinkageService() { return LinkageService::getInstance(); }

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    LinkageController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<LinkageController> createShared(
    VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
    ){
        return std::make_shared<LinkageController>(objectMapper);
    }

    // 事件类型相关端点
    /**
     * 获取所有事件类型
     * @return 事件类型列表
     */
    ENDPOINT_INFO(getEventTypes) {
        info->summary = "获取所有事件类型";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<EventTypePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/event-types", getEventTypes, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/event-types called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getEventTypes(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/event-types returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    // 表达式相关端点
    /**
     * 获取所有表达式
     * @return 表达式列表
     */
    ENDPOINT_INFO(getExpressions) {
        info->summary = "获取所有表达式";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<ExpressionPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/expressions", getExpressions, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/expressions called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getExpressions(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/expressions returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取表达式
     * @param id 表达式ID
     * @return 表达式信息
     */
    ENDPOINT_INFO(getExpressionById) {
        info->summary = "根据ID获取表达式";
        info->pathParams.add<vsoa::UInt32>("id").description = "表达式ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<ExpressionDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/expressions/{id}", getExpressionById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/expressions/{id} called with id=%u", id.getValue(0));
        auto expression = getLinkageService().getExpressionById(id);
        if (!expression) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Expression not found with id=%u", id.getValue(0));
            return createResponse(Status::CODE_404, "表达式不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/expressions returned successfully for id=%u", id.getValue(0));
        return createDtoResponse(Status::CODE_200, expression);
    }

    /**
     * 创建新表达式
     * @param expressionDto 表达式创建数据
     * @return 创建的表达式信息
     */
    ENDPOINT_INFO(createExpression) {
        info->summary = "创建新表达式";
        info->addConsumes<vsoa::Object<ExpressionCreateDto>>("application/json");
        info->addResponse<vsoa::Object<ExpressionDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/expressions", createExpression, 
        BODY_DTO(vsoa::Object<ExpressionCreateDto>, expressionDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] POST /api/expressions called to create expression");
        auto expression = getLinkageService().createExpression(expressionDto);
        if (!expression) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[LinkageController] Failed to create expression");
            return createResponse(Status::CODE_400, "创建表达式失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Expression created successfully");
        return createDtoResponse(Status::CODE_201, expression);
    }

    /**
     * 更新表达式
     * @param expressionDto 表达式更新数据
     * @return 更新后的表达式信息
     */
    ENDPOINT_INFO(updateExpression) {
        info->summary = "更新表达式";
        info->addConsumes<vsoa::Object<ExpressionDto>>("application/json");
        info->addResponse<vsoa::Object<ExpressionDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/expressions", updateExpression, 
        BODY_DTO(vsoa::Object<ExpressionDto>, expressionDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] PUT /api/expressions called to update expression");
        auto expression = getLinkageService().updateExpression(expressionDto);
        if (!expression) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Expression not found for update");
            return createResponse(Status::CODE_404, "表达式不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Expression updated successfully");
        return createDtoResponse(Status::CODE_200, expression);
    }

    /**
     * 删除表达式
     * @param id 表达式ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteExpression) {
        info->summary = "删除表达式";
        info->pathParams.add<vsoa::UInt32>("id").description = "表达式ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_404, "application/json");
    }
    ENDPOINT("DELETE", "/api/expressions/{id}", deleteExpression, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] DELETE /api/expressions/{id} called with id=%u", 
            id.getValue(0));
        auto status = getLinkageService().deleteExpression(id);
        if (status->code == 404) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Expression not found for deletion with id=%u", 
                id.getValue(0));
            return createDtoResponse(Status::CODE_404, status);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Expression deleted successfully with id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_204, status);
    }

    // 触发源相关端点
    /**
     * 获取所有触发源
     * @return 触发源列表
     */
    ENDPOINT_INFO(getTriggerSources) {
        info->summary = "获取所有触发源";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<TriggerSourcePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/trigger-sources", getTriggerSources, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size) )
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/trigger-sources called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getTriggerSources(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/trigger-sources returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取触发源
     * @param id 触发源ID
     * @return 触发源信息
     */
    ENDPOINT_INFO(getTriggerSourceById) {
        info->summary = "根据ID获取触发源";
        info->pathParams.add<vsoa::UInt32>("id").description = "触发源ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<TriggerSourceDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/trigger-sources/{id}", getTriggerSourceById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/trigger-sources/{id} called with id=%u", 
            id.getValue(0));
        auto trigger = getLinkageService().getTriggerSourceById(id);
        if (!trigger) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Trigger source not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "触发源不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/trigger-sources returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, trigger);
    }

    /**
     * 创建新触发源
     * @param triggerDto 触发源创建数据
     * @return 创建的触发源信息
     */
    ENDPOINT_INFO(createTriggerSource) {
        info->summary = "创建新触发源";
        info->addConsumes<vsoa::Object<TriggerSourceCreateDto>>("application/json");
        info->addResponse<vsoa::Object<TriggerSourceDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/trigger-sources", createTriggerSource, 
        BODY_DTO(vsoa::Object<TriggerSourceCreateDto>, triggerDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] POST /api/trigger-sources called to create trigger source");
        auto trigger = getLinkageService().createTriggerSource(triggerDto);
        if (!trigger) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[LinkageController] Failed to create trigger source");
            return createResponse(Status::CODE_400, "创建触发源失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Trigger source created successfully");
        return createDtoResponse(Status::CODE_201, trigger);
    }

    /**
     * 更新触发源
     * @param id 触发源ID
     * @param triggerDto 触发源更新数据
     * @return 更新后的触发源信息
     */
    ENDPOINT_INFO(updateTriggerSource) {
        info->summary = "更新触发源";
        info->addConsumes<vsoa::Object<TriggerSourceDto>>("application/json");
        info->addResponse<vsoa::Object<TriggerSourceDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/trigger-sources", updateTriggerSource, 
        BODY_DTO(vsoa::Object<TriggerSourceDto>, triggerDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] PUT /api/trigger-sources called to update trigger source");
        auto trigger = getLinkageService().updateTriggerSource(triggerDto);
        if (!trigger) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Trigger source not found for update");
            return createResponse(Status::CODE_404, "触发源不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Trigger source updated successfully");
        return createDtoResponse(Status::CODE_200, trigger);
    }

    /**
     * 删除触发源
     * @param id 触发源ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteTriggerSource) {
        info->summary = "删除触发源";
        info->pathParams.add<vsoa::UInt32>("id").description = "触发源ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_404, "application/json");
    }
    ENDPOINT("DELETE", "/api/trigger-sources/{id}", deleteTriggerSource, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] DELETE /api/trigger-sources/{id} called with id=%u", 
            id.getValue(0));
        auto status = getLinkageService().deleteTriggerSource(id);
        if (status->code == 404) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Trigger source not found for deletion with id=%u", 
                id.getValue(0));
            return createDtoResponse(Status::CODE_404, status);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Trigger source deleted successfully with id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_204, status);
    }

    // 动作类型相关端点
    /**
     * 获取所有动作类型
     * @return 动作类型列表
     */
    ENDPOINT_INFO(getActionTypes) {
        info->summary = "获取所有动作类型";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<ActionTypePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/action-types", getActionTypes, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/action-types called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getActionTypes(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/action-types returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    // 动作实例相关端点
    /**
     * 获取所有动作实例
     * @return 动作实例列表
     */
    ENDPOINT_INFO(getActionInstances) {
        info->summary = "获取所有动作实例";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<ActionInstancePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/action-instances", getActionInstances, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/action-instances called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getActionInstances(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/action-instances returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取动作实例
     * @param id 动作实例ID
     * @return 动作实例信息
     */
    ENDPOINT_INFO(getActionInstanceById) {
        info->summary = "根据ID获取动作实例";
        info->pathParams.add<vsoa::UInt32>("id").description = "动作实例ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<ActionInstanceDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/action-instances/{id}", getActionInstanceById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/action-instances/{id} called with id=%u", 
            id.getValue(0));
        auto action = getLinkageService().getActionInstanceById(id);
        if (!action) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Action instance not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "动作实例不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/action-instances returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, action);
    }

    /**
     * 创建新动作实例
     * @param actionDto 动作实例创建数据
     * @return 创建的动作实例信息
     */
    ENDPOINT_INFO(createActionInstance) {
        info->summary = "创建新动作实例";
        info->addResponse<vsoa::Object<ActionInstanceDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/action-instances", createActionInstance, 
        BODY_DTO(vsoa::Object<ActionInstanceCreateDto>, actionDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] POST /api/action-instances called to create action instance");
        auto action = getLinkageService().createActionInstance(actionDto);
        if (!action) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[LinkageController] Failed to create action instance");
            return createResponse(Status::CODE_400, "创建动作实例失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Action instance created successfully");
        return createDtoResponse(Status::CODE_201, action);
    }

    /**
     * 更新动作实例
     * @param id 动作实例ID
     * @param actionDto 动作实例更新数据
     * @return 更新后的动作实例信息
     */
    ENDPOINT_INFO(updateActionInstance) {
        info->summary = "更新动作实例";
        info->addResponse<vsoa::Object<ActionInstanceDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/action-instances", updateActionInstance, 
        BODY_DTO(vsoa::Object<ActionInstanceDto>, actionDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] PUT /api/action-instances called to update action instance");
        auto action = getLinkageService().updateActionInstance(actionDto);
        if (!action) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Action instance not found for update");
            return createResponse(Status::CODE_404, "动作实例不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Action instance updated successfully");
        return createDtoResponse(Status::CODE_200, action);
    }

    /**
     * 删除动作实例
     * @param id 动作实例ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteActionInstance) {
        info->summary = "删除动作实例";
        info->pathParams.add<vsoa::UInt32>("id").description = "动作实例ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::String>(Status::CODE_204, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/action-instances/{id}", deleteActionInstance, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] DELETE /api/action-instances/{id} called with id=%u", 
            id.getValue(0));
        if (!getLinkageService().deleteActionInstance(id)) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Action instance not found for deletion with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "动作实例不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Action instance deleted successfully with id=%u", 
            id.getValue(0));
        return createResponse(Status::CODE_204, "动作实例已删除");

    }

    // 联动规则相关端点
    /**
     * 获取所有联动规则
     * @return 联动规则列表
     */
    ENDPOINT_INFO(getLinkageRules) {
        info->summary = "获取所有联动规则";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<LinkageRulePageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/linkage-rules", getLinkageRules, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-rules called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getLinkageRules(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-rules returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    // 获取联动规则（包含触发源/动作详情）用于编辑
    ENDPOINT_INFO(getLinkageRuleFullById) {
        info->summary = "根据ID获取联动规则完整信息（包含触发源与动作详情）";
        info->pathParams.add<vsoa::UInt32>("id").description = "联动规则ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<LinkageRuleFullDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/linkage-rules/full/{id}", getLinkageRuleFullById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-rules/full/{id} called with id=%u", 
            id.getValue(0));
        auto dto = getLinkageService().getLinkageRuleWithDetails(id);
        if (!dto) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Linkage rule not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "联动规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-rules/full/{id} returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, dto);
    }

    /**
     * 根据ID获取联动规则
     * @param id 联动规则ID
     * @return 联动规则信息
     */
    ENDPOINT_INFO(getLinkageRuleById) {
        info->summary = "根据ID获取联动规则";
        info->pathParams.add<vsoa::UInt32>("id").description = "联动规则ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::Object<LinkageRuleDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/linkage-rules/{id}", getLinkageRuleById, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-rules/{id} called with id=%u", 
            id.getValue(0));
        auto rule = getLinkageService().getLinkageRuleById(id);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Linkage rule not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "联动规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-rules returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, rule);
    }

    /**
     * 创建新联动规则
     * @param ruleDto 联动规则创建数据
     * @return 创建的联动规则信息
     */
    ENDPOINT_INFO(createLinkageRule) {
        info->summary = "创建新联动规则";
        info->addConsumes<vsoa::Object<LinkageRuleCreateDto>>("application/json");
        info->addResponse<vsoa::Object<LinkageRuleDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/linkage-rules", createLinkageRule, 
        BODY_DTO(vsoa::Object<LinkageRuleCreateDto>, ruleDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] POST /api/linkage-rules called to create linkage rule");
        auto rule = getLinkageService().createLinkageRule(ruleDto);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[LinkageController] Failed to create linkage rule");
            return createResponse(Status::CODE_400, "创建联动规则失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Linkage rule created successfully");
        return createDtoResponse(Status::CODE_201, rule);
    }

    // 创建联动规则并关联触发源/动作（一次性提交）
    ENDPOINT_INFO(createLinkageRuleFull) {
        info->summary = "创建联动规则并同时关联触发源与动作";
        info->addConsumes<vsoa::Object<LinkageRuleCreateFullDto>>("application/json");
        info->addResponse<vsoa::Object<LinkageRuleDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/linkage-rules/full", createLinkageRuleFull, 
        BODY_DTO(vsoa::Object<LinkageRuleCreateFullDto>, ruleDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] POST /api/linkage-rules/full called to create linkage rule with relations");
        auto rule = getLinkageService().createLinkageRuleWithRelations(ruleDto);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[LinkageController] Failed to create linkage rule with relations");
            return createResponse(Status::CODE_400, "创建联动规则失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Linkage rule with relations created successfully");
        return createDtoResponse(Status::CODE_201, rule);
    }

    /**
     * 更新联动规则
     * @param id 联动规则ID
     * @param ruleDto 联动规则更新数据
     * @return 更新后的联动规则信息
     */
    ENDPOINT_INFO(updateLinkageRule) {
        info->summary = "更新联动规则";
        info->addConsumes<vsoa::Object<LinkageRuleDto>>("application/json");
        info->addResponse<vsoa::Object<LinkageRuleDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/linkage-rules", updateLinkageRule, 
        BODY_DTO(vsoa::Object<LinkageRuleDto>, ruleDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] PUT /api/linkage-rules called to update linkage rule");
        auto rule = getLinkageService().updateLinkageRule(ruleDto);
        if (!rule) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Linkage rule not found for update");
            return createResponse(Status::CODE_404, "联动规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Linkage rule updated successfully");
        return createDtoResponse(Status::CODE_200, rule);
    }

    /**
     * 删除联动规则
     * @param id 联动规则ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteLinkageRule) {
        info->summary = "删除联动规则";
        info->pathParams.add<vsoa::UInt32>("id").description = "联动规则ID";
        info->pathParams["id"].required = "true";
        info->addResponse<vsoa::String>(Status::CODE_204, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/linkage-rules/{id}", deleteLinkageRule, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] DELETE /api/linkage-rules/{id} called with id=%u",
            id.getValue(0));
        if (!getLinkageService().deleteLinkageRule(id)) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Linkage rule not found for deletion with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "联动规则不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Linkage rule deleted successfully with id=%u", 
            id.getValue(0));
        return createResponse(Status::CODE_204, "联动规则已删除");
    }

    /**
     * 为联动规则添加触发源
     * @param ruleTrigger 规则触发关联数据
     * @return 添加结果
     */
    ENDPOINT_INFO(addTriggerToRule) {
        info->summary = "为联动规则添加触发源";
        info->addConsumes<vsoa::Object<RuleTriggerDto>>("application/json");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_200, "text/plain");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/linkage-rules/triggers", addTriggerToRule, 
        BODY_DTO(vsoa::Object<RuleTriggerDto>, ruleTrigger))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] POST /api/linkage-rules/triggers called to add trigger to rule");
        auto status = getLinkageService().addTriggerToRule(ruleTrigger->rule_id, ruleTrigger->trigger_id);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Added trigger to rule successfully");
        return createDtoResponse(Status::CODE_200, status);
    }

    /**
     * 从联动规则中移除触发源
     * @param ruleTrigger 规则触发关联数据
     * @return 移除结果
     */
    ENDPOINT_INFO(removeTriggerFromRule) {
        info->summary = "从联动规则中移除触发源";
        info->addConsumes<vsoa::Object<RuleTriggerDto>>("application/json");
        info->addResponse<vsoa::String>(Status::CODE_200, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("DELETE", "/api/linkage-rules/triggers", removeTriggerFromRule, BODY_DTO(vsoa::Object<RuleTriggerDto>, ruleTrigger)) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] DELETE /api/linkage-rules/triggers called to remove trigger from rule");
        if (!getLinkageService().removeTriggerFromRule(ruleTrigger->rule_id, ruleTrigger->trigger_id)) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[LinkageController] Failed to remove trigger from rule");
            return createResponse(Status::CODE_404, "联动规则或触发源不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] Removed trigger from rule successfully");
        return createResponse(Status::CODE_200, "触发源已从规则中移除");
    }

    // 联动日志相关端点
    /**
     * 获取所有联动日志
     * @return 联动日志列表
     */
    ENDPOINT_INFO(getLinkageLogs) {
        info->summary = "获取所有联动日志";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams["page"].required = "true";
        info->queryParams["size"].required = "true";
        info->addResponse<vsoa::Object<LinkageLogPageDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/linkage-logs", getLinkageLogs, 
        QUERY(vsoa::UInt32, page), 
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-logs called with page=%u, size=%u", page.getValue(0), size.getValue(0));
        auto response = getLinkageService().getLinkageLogs(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[LinkageController] GET /api/linkage-logs returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif // LINKAGE_CONTROLLER_HPP_