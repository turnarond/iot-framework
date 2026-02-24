/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/LinkageService.cpp
 * @Description: Linkage Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "LinkageService.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include "vsoa_dto/core/Types.hpp"

// 事件类型相关方法
vsoa::Object<EventTypePageDto> LinkageService::getEventTypes(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = linkageDb->getEventTypeCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;

    dbResult = linkageDb->getEventTypesWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<EventTypeDto>>>();
    
    auto response = EventTypePageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

// 表达式相关方法
vsoa::Object<ExpressionPageDto> LinkageService::getExpressions(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = linkageDb->getExpressionCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;

    dbResult = linkageDb->getExpressionsWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<ExpressionDto>>>();
    
    auto response = ExpressionPageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<ExpressionDto> LinkageService::getExpressionById(vsoa::UInt32 id) 
{
    auto dbResult = linkageDb->getExpressionById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Expression not found");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<ExpressionDto>>>();
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_500, "Multiple Expressions found with the same ID");
    
    return items->front();
}

vsoa::Object<ExpressionDto> LinkageService::createExpression(vsoa::Object<ExpressionCreateDto> expressionDto) 
{
    // Basic validation
    OATPP_ASSERT_HTTP(!expressionDto->name->empty(), Status::CODE_400, "name is required");
    OATPP_ASSERT_HTTP(!expressionDto->expression->empty(), Status::CODE_400, "expression is required");

    // Insert the expression
    auto dbResult = linkageDb->createExpression(expressionDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto expressionId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(expressionId > 0, Status::CODE_500, "Unknown error");

    return getExpressionById(expressionId);
}

vsoa::Object<ExpressionDto> LinkageService::updateExpression(vsoa::Object<ExpressionDto> expressionDto) 
{
    // Basic validation
    OATPP_ASSERT_HTTP(expressionDto->id > 0, Status::CODE_400, "id is required");
    OATPP_ASSERT_HTTP(!expressionDto->name->empty(), Status::CODE_400, "name is required");
    OATPP_ASSERT_HTTP(!expressionDto->expression->empty(), Status::CODE_400, "expression is required");

    // Update the expression
    auto dbResult = linkageDb->updateExpression(expressionDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    return getExpressionById(expressionDto->id);
}

vsoa::Object<StatusDto> LinkageService::deleteExpression(vsoa::UInt32 id) 
{
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Check if expression exists
    if (!getExpressionById(id)) {
        status->message = "Expression not found";
        status->code = 404;
        return status;
    }
    
    auto dbResult = linkageDb->deleteExpression(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    status->message = "OK";
    status->code = 200;
    return status;
}

// 触发源相关方法
vsoa::Object<TriggerSourcePageDto> LinkageService::getTriggerSources(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总记录数
    auto dbResult = linkageDb->getTriggerSourceCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    // 计算总页数
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;
    
    // 获取触发源列表
    dbResult = linkageDb->getTriggerSourcesWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<TriggerSourceDto>>>();
    
    // 创建分页响应
    auto response = TriggerSourcePageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<TriggerSourceDto> LinkageService::getTriggerSourceById(vsoa::UInt32 id) 
{
    auto dbResult = linkageDb->getTriggerSourceById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "TriggerSource not found");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<TriggerSourceDto>>>();
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_500, "Multiple TriggerSources found with the same ID");
    
    return items->front();
}

vsoa::Object<TriggerSourceDto> LinkageService::createTriggerSource(vsoa::Object<TriggerSourceCreateDto> triggerDto) 
{
    // Basic validation
    OATPP_ASSERT_HTTP(triggerDto->event_type_id > 0, Status::CODE_400, "event_type_id is required");
    if (triggerDto->event_type_id == 1 || triggerDto->event_type_id == 2) {
        OATPP_ASSERT_HTTP(triggerDto->alarm_rule_id > 0, Status::CODE_400, "alarm_rule_id is required for alarm event types");
    } else if (triggerDto->event_type_id == 3) {
        OATPP_ASSERT_HTTP(triggerDto->expr_id > 0, Status::CODE_400, "expr_id is required for expression event types");
    }

    // Insert the trigger
    auto dbResult = linkageDb->createTriggerSource(triggerDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto triggerId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(triggerId > 0, Status::CODE_500, "Unknown error");

    return getTriggerSourceById(triggerId);
}

vsoa::Object<TriggerSourceDto> LinkageService::updateTriggerSource(vsoa::Object<TriggerSourceDto> triggerDto) 
{
    // Basic validation
    OATPP_ASSERT_HTTP(triggerDto->id > 0, Status::CODE_400, "id is required");
    OATPP_ASSERT_HTTP(triggerDto->event_type_id > 0, Status::CODE_400, "event_type_id is required");
    if (triggerDto->event_type_id == 1 || triggerDto->event_type_id == 2) {
        OATPP_ASSERT_HTTP(triggerDto->alarm_rule_id > 0, Status::CODE_400, "alarm_rule_id is required for alarm event types");
    } else if (triggerDto->event_type_id == 3) {
        OATPP_ASSERT_HTTP(triggerDto->expr_id > 0, Status::CODE_400, "expr_id is required for expression event types");
    }

    // Update the trigger
    auto dbResult = linkageDb->updateTriggerSource(triggerDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    return getTriggerSourceById(triggerDto->id);
}

vsoa::Object<StatusDto> LinkageService::deleteTriggerSource(vsoa::UInt32 id) 
{
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    auto dbResult = linkageDb->deleteTriggerSource(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    status->message = "OK";
    status->code = 200;
    return status;
}

vsoa::Vector<vsoa::Object<TriggerSourceDto>> LinkageService::getTriggersByAlarmRuleId(vsoa::UInt32 alarmRuleId) 
{
    auto dbResult = linkageDb->getTriggersByAlarmRuleId(alarmRuleId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<TriggerSourceDto>>>();
    
    return items;
}

// 动作类型相关方法
vsoa::Object<ActionTypePageDto> LinkageService::getActionTypes(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总记录数
    auto dbResult = linkageDb->getActionTypeCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    // 计算总页数
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;
    
    // 获取动作类型列表
    dbResult = linkageDb->getActionTypesWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<ActionTypeDto>>>();
    
    // 创建分页响应
    auto response = ActionTypePageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

// 动作实例相关方法
vsoa::Object<ActionInstancePageDto> LinkageService::getActionInstances(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总记录数
    auto dbResult = linkageDb->getActionInstanceCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    // 计算总页数
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;
    
    // 获取动作实例列表
    dbResult = linkageDb->getActionInstancesWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<ActionInstanceDto>>>();
    
    // 创建分页响应
    auto response = ActionInstancePageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<ActionInstanceDto> LinkageService::getActionInstanceById(vsoa::UInt32 id) 
{
    auto dbResult = linkageDb->getActionInstanceById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "ActionInstance not found");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<ActionInstanceDto>>>();
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_500, "Multiple ActionInstances found with the same ID");
    
    return items->front();
}

vsoa::Object<ActionInstanceDto> LinkageService::createActionInstance(vsoa::Object<ActionInstanceCreateDto> actionDto) 
{
    // Basic validation
    OATPP_ASSERT_HTTP(actionDto->action_type_id > 0, Status::CODE_400, "action_type_id is required");

    auto dbResult = linkageDb->createActionInstance(actionDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto actionId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(actionId > 0, Status::CODE_500, "Unknown error");

    return getActionInstanceById(actionId);
}

vsoa::Object<ActionInstanceDto> LinkageService::updateActionInstance(vsoa::Object<ActionInstanceDto> actionDto) 
{
    OATPP_ASSERT_HTTP(actionDto->id > 0, Status::CODE_400, "id is required");
    OATPP_ASSERT_HTTP(actionDto->action_type_id > 0, Status::CODE_400, "action_type_id is required");

    auto dbResult = linkageDb->updateActionInstance(actionDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    return getActionInstanceById(actionDto->id);
}

vsoa::Object<StatusDto> LinkageService::deleteActionInstance(vsoa::UInt32 id) 
{
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Check if action exists
    if (!getActionInstanceById(id)) {
        status->message = "ActionInstance not found";
        status->code = 404;
        return status;
    }
    
    auto dbResult = linkageDb->deleteActionInstance(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    status->message = "OK";
    status->code = 200;
    return status;
}


// 联动规则相关方法
vsoa::Object<LinkageRulePageDto> LinkageService::getLinkageRules(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总记录数
    auto dbResult = linkageDb->getLinkageRuleCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    // 计算总页数
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    
    // 获取联动规则列表
    dbResult = linkageDb->getLinkageRulesWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<LinkageRuleDto>>>();
    
    // 创建分页响应
    auto response = LinkageRulePageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<LinkageRuleDto> LinkageService::getLinkageRuleById(vsoa::UInt32 id) 
{
    auto dbResult = linkageDb->getLinkageRuleById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "LinkageRule not found");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<LinkageRuleDto>>>(); 
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_500, "Multiple LinkageRules found with the same ID");
    
    return items->front();
}

vsoa::Object<LinkageRuleFullDto> LinkageService::getLinkageRuleWithDetails(vsoa::UInt32 id)
{
    // fetch base rule
    auto dbResult = linkageDb->getLinkageRuleById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    if (!dbResult->hasMoreToFetch()) {
        return nullptr;
    }
    auto rules = dbResult->fetch<vsoa::Vector<vsoa::Object<LinkageRuleDto>>>();
    OATPP_ASSERT_HTTP(rules->size() == 1, Status::CODE_500, "Multiple LinkageRules found with same ID");
    auto base = rules->front();

    auto dto = LinkageRuleFullDto::createShared();
    dto->id = base->id;
    dto->name = base->name;
    dto->logic_type = base->logic_type;
    dto->enable = base->enable;
    dto->description = base->description;

    // triggers
    dto->triggers = vsoa::Vector<vsoa::Object<TriggerSourceDto>>::createShared();
    auto tRes = linkageDb->getTriggersByRuleId(id);
    OATPP_ASSERT_HTTP(tRes->isSuccess(), Status::CODE_500, tRes->getErrorMessage());
    auto tFields = tRes->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>();
    for (auto &f : *tFields) {
        auto tId = f["trigger_id"];
        try {
            auto trg = getTriggerSourceById(tId);
            if (trg) dto->triggers->push_back(trg);
        } catch (...) {}
    }

    // actions
    dto->actions = vsoa::Vector<vsoa::Object<ActionInstanceDto>>::createShared();
    auto aRes = linkageDb->getActionsByRuleId(id);
    OATPP_ASSERT_HTTP(aRes->isSuccess(), Status::CODE_500, aRes->getErrorMessage());
    auto aFields = aRes->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>();
    for (auto &f : *aFields) {
        auto aId = f["action_id"];
        try {
            auto act = getActionInstanceById(aId);
            if (act) dto->actions->push_back(act);
        } catch (...) {}
    }

    return dto;
}

vsoa::Object<LinkageRuleWithDetailPageDto> LinkageService::getLinkageRulesWithDetails(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总记录数
    auto dbResult = linkageDb->getLinkageRuleCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    // 计算总页数
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    
    // 获取联动规则详情列表
    dbResult = linkageDb->getLinkageRulesWithDetailsWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<LinkageRuleWithDetailsDto>>>();
    
    // 创建分页响应
    auto response = LinkageRuleWithDetailPageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<LinkageRuleDto> LinkageService::createLinkageRule(vsoa::Object<LinkageRuleCreateDto> ruleDto) 
{
    // Create a LinkageRuleDto from the LinkageRuleCreateDto
    auto dbResult = linkageDb->createLinkageRule(ruleDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    // Get the last inserted ID (this part might need adjustment based on actual implementation)
    auto ruleId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(ruleId > 0, Status::CODE_500, "Unknown error");
    
    // For now, we'll just return the created rule
    return getLinkageRuleById(ruleId);
}

vsoa::Object<LinkageRuleDto> LinkageService::createLinkageRuleWithRelations(vsoa::Object<LinkageRuleCreateFullDto> ruleDto)
{
    // Create base rule DTO
    auto base = LinkageRuleCreateDto::createShared();
    base->name = ruleDto->name;
    base->logic_type = ruleDto->logic_type;
    base->enable = ruleDto->enable;
    base->description = ruleDto->description;

    auto dbResult = linkageDb->createLinkageRule(base);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto ruleId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(ruleId > 0, Status::CODE_500, "Unknown error");

    // Associate triggers (if any)
    if (ruleDto->triggers && ruleDto->triggers->size() > 0) {
        for (auto &t : *(ruleDto->triggers)) {
                // t->source_id refers to existing trigger id
            auto status = addTriggerToRule(ruleId, t->source_id);
            OATPP_ASSERT_HTTP(status->code == 200, Status::CODE_500, status->message);
        }
    }

    // Create action instances then associate
    if (ruleDto->actions && ruleDto->actions->size() > 0) {
        for (auto &a : *(ruleDto->actions)) {
            auto status = addActionToRule(ruleId, a->id);
            OATPP_ASSERT_HTTP(status->code == 200, Status::CODE_500, status->message);
        }
    }

    return getLinkageRuleById(ruleId);
}

vsoa::Object<StatusDto> LinkageService::addActionToRule(vsoa::UInt32 ruleId, vsoa::UInt32 actionId)
{
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Validate existence
    if (!getLinkageRuleById(ruleId)) {
        status->message = "Linkage rule not found";
        status->code = 404;
        return status;
    }
    if (!getActionInstanceById(actionId)) {
        status->message = "ActionInstance not found";
        status->code = 404;
        return status;
    }

    auto dbResult = linkageDb->addActionToRule(ruleId, actionId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    status->message = "OK";
    status->code = 200;
    return status;
}

vsoa::Object<LinkageRuleDto> LinkageService::updateLinkageRule(vsoa::Object<LinkageRuleDto> ruleDto) 
{
    // Update the rule
    auto dbResult = linkageDb->updateLinkageRule(ruleDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    return getLinkageRuleById(ruleDto->id);
}

vsoa::Object<StatusDto> LinkageService::deleteLinkageRule(vsoa::UInt32 id) 
{
    // Check if rule exists
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (!getLinkageRuleById(id)) {
        status->message = "Linkage rule not found";
        status->code = 404;
        return status;
    }
    
    // Delete the rule (the database layer should handle cascading deletes if needed)
    auto dbResult = linkageDb->deleteLinkageRule(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    status->message = "OK";
    status->code = 200;
    return status;
}

// 规则触发源相关方法
vsoa::Object<StatusDto> LinkageService::addTriggerToRule(vsoa::UInt32 ruleId, vsoa::UInt32 triggerId) 
{
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // Check if rule exists
    if (!getLinkageRuleById(ruleId)) {
        status->message = "Linkage rule not found";
        status->code = 404;
        return status;
    }
    
    // Check if trigger exists
    if (!getTriggerSourceById(triggerId)) {
        status->message = "Trigger source not found";
        status->code = 404;
        return status;
    }
    
    // Add the trigger association
    auto dbResult = linkageDb->addTriggerToRule(ruleId, triggerId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    status->message = "OK";
    status->code = 200;
    return status;
}

vsoa::Object<StatusDto> LinkageService::removeTriggerFromRule(vsoa::UInt32 ruleId, vsoa::UInt32 triggerId) 
{
    // Remove the trigger association
    auto dbResult = linkageDb->removeTriggerFromRule(ruleId, triggerId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto status = StatusDto::createShared();
    status->message = "OK";
    status->code = 200;
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return status;
}

// 联动日志相关方法
vsoa::Object<LinkageLogPageDto> LinkageService::getLinkageLogs(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总记录数
    auto dbResult = linkageDb->getLinkageLogCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    // 计算总页数
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;

    // 获取联动日志列表
    dbResult = linkageDb->getLinkageLogs(size, offset);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<LinkageLogDto>>>();

    // 创建分页响应
    auto dto = LinkageLogPageDto::createShared();
    dto->items = items;
    dto->counts = items->size();
    dto->page = page;
    dto->size = size;
    dto->pages = pages;
    
    return dto;
}
