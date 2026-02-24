/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/AlarmService.cpp
 * @Description: Alarm Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "AlarmService.hpp"

// 告警规则相关方法
vsoa::Object<AlarmRulePageDto> AlarmService::getAlarmRules(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = alarmDb->getAlarmRuleCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }   

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    
    dbResult = alarmDb->getAlarmRules(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleDto>>>();

    auto dto = AlarmRulePageDto::createShared();
    dto->items = items;
    dto->counts = items->size();
    dto->page = page;
    dto->size = size;
    dto->pages = pages;
    
    return dto;
}

vsoa::Object<AlarmRuleWithPointPageDto> AlarmService::getAlarmRulesWithPoint(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = alarmDb->getAlarmRuleCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }   

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    
    dbResult = alarmDb->getAlarmRulesWithPointWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleWithPointDto>>>();

    auto dto = AlarmRuleWithPointPageDto::createShared();
    
    dto->items = items;
    dto->counts = items->size();
    dto->page = page;
    dto->size = size;
    dto->pages = pages;
    
    return dto;
}

vsoa::Object<AlarmRuleDto> AlarmService::getAlarmRuleById(vsoa::UInt32 id) 
{
    auto dbResult = alarmDb->getAlarmRuleById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "告警规则不存在");
    
    auto rules = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleDto>>>();
    OATPP_ASSERT_HTTP(rules->size() == 1, Status::CODE_404, "告警规则不存在");
    
    return rules[0];
}

vsoa::Object<AlarmRuleWithPointDto> AlarmService::getAlarmRuleWithPointById(vsoa::UInt32 id) 
{
    auto dbResult = alarmDb->getAlarmRuleWithPointById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "告警规则不存在");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleWithPointDto>>>();
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_404, "告警规则不存在");
    
    return items[0];
}

vsoa::Object<AlarmRuleWithPointDto> AlarmService::createAlarmRuleWithPoint(vsoa::Object<AlarmRuleWithPointCreateDto> ruleDto) 
{
    auto dbResult = alarmDb->createAlarmRuleWithPoint(ruleDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto id = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(id > 0, Status::CODE_500, "创建告警规则失败");
    
    return getAlarmRuleWithPointById(id);
}

vsoa::Object<AlarmRuleDto> AlarmService::createAlarmRule(vsoa::Object<AlarmRuleCreateDto> ruleDto) 
{
    auto dbResult = alarmDb->createAlarmRule(ruleDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto id = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(id > 0, Status::CODE_500, "创建告警规则失败");
    
    return getAlarmRuleById(id);
}

vsoa::Object<AlarmRuleDto> AlarmService::updateAlarmRule(vsoa::Object<AlarmRuleDto> ruleDto) 
{
    auto dbResult = alarmDb->updateAlarmRule(ruleDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    return getAlarmRuleById(ruleDto->id);
}

vsoa::Object<StatusDto> AlarmService::deleteAlarmRule(vsoa::UInt32 id) 
{
    auto dbResult = alarmDb->deleteAlarmRule(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto status = StatusDto::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    status->message = "OK";
    status->code = 200;
    return status;
}

vsoa::Object<AlarmPageDto> AlarmService::getAlarms(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = alarmDb->getAlarmCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }   

    auto response = AlarmPageDto::createShared();
    
    vsoa::UInt32 offset = (page - 1) * size;
    dbResult = alarmDb->getAlarmsWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmDto>>>();
    
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<AlarmPageDto> AlarmService::getActiveAlarms(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = alarmDb->getActiveAlarmCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }   

    auto response = AlarmPageDto::createShared();
    
    vsoa::UInt32 offset = (page - 1) * size;
    dbResult = alarmDb->getActiveAlarmsWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmDto>>>();
    
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Vector<vsoa::Object<AlarmRuleWithPointDto>> AlarmService::getAlarmRulesByPointId(vsoa::UInt32 pointId) 
{
    auto dbResult = alarmDb->getAlarmRuleByPointId(pointId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleWithPointDto>>>();
    
    return items;
}
