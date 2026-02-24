/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/PointService.cpp
 * @Description: Point Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "PointService.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "service/AlarmService.hpp"
#include "service/LinkageService.hpp"
#include <chrono>

using vsoa::web::protocol::http::Status;

// 点位相关方法
vsoa::Object<PointPageDto> PointService::getPoints(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = pointDb->getPointCount();
    // OATPP_ASSERT_HTTP(totalCounts->isSuccess(), Status::CODE_500, totalCounts->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");

    vsoa::UInt32 pages = (totalCounts["count"] + size - 1) / size;
    if (page > pages) {
        page = pages;
    }
    
    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;
    dbResult = pointDb->getPointsWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<PointDto>>>();
    
    auto response = PointPageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<PointWithDevicePageDto> PointService::getPointsWithDevice(vsoa::UInt32 page, vsoa::UInt32 size)
{
    // 获取总页数
    auto dbResult = pointDb->getPointCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");

    vsoa::UInt32 pages = (totalCounts["count"] + size - 1) / size;
    if (page > pages) {
        page = pages;
    }
    
    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;

    dbResult = pointDb->getPointsWithDeviceWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<PointWithDeviceDto>>>();
    
    auto response = PointWithDevicePageDto::createShared();
    response->items = items;
    response->counts = items->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<PointDto> PointService::getPointById(vsoa::UInt32 id) 
{
    auto dbResult = pointDb->getPointById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Point not found");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<PointDto>>>();
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_500, "Unknown error");
    
    return items->front();
}

vsoa::Object<PointDto> PointService::createPoint(vsoa::Object<PointCreateDto> pointCreateDto) 
{
    // 创建 PointDto 对象并复制字段
    auto dbResult = pointDb->createPoint(pointCreateDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto pointId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    OATPP_ASSERT_HTTP(pointId > 0, Status::CODE_500, "Unknown error");
    
    return getPointById(pointId);
}

vsoa::Object<PointWithDeviceDto> PointService::getPointWithDeviceById(vsoa::UInt32 id) 
{
    auto dbResult = pointDb->getPointWithDeviceById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Point not found");
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<PointWithDeviceDto>>>();
    OATPP_ASSERT_HTTP(items->size() == 1, Status::CODE_500, "Unknown error");
    
    return items->front();
}

vsoa::Object<PointDto> PointService::updatePoint(vsoa::Object<PointDto> pointDto) 
{
    // 更新点位
    auto dbResult = pointDb->updatePoint(pointDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    return getPointById(pointDto->id);
}

vsoa::Object<StatusDto> PointService::deletePoint(vsoa::UInt32 id) 
{
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // 检查点位是否存在
    if (!getPointById(id)) {
        status->message = "Point not found";
        status->code = 404;
        return status;
    }
    
    // 1. 获取点位关联的所有报警规则
    auto alarmDbResult = alarmDb->getAlarmRuleByPointId(id);
    OATPP_ASSERT_HTTP(alarmDbResult->isSuccess(), Status::CODE_500, alarmDbResult->getErrorMessage());
    auto alarmRules = alarmDbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleDto>>>();
    
    // 2. 遍历报警规则，处理关联数据
    for (auto& alarmRule : *alarmRules) {
        // 3. 获取报警规则关联的所有触发源
        auto triggers = getLinkageService().getTriggersByAlarmRuleId(alarmRule->id);
        
        // 4. 遍历触发源，处理关联数据
        for (auto& trigger : *triggers) {
            // 5. 删除触发源
            getLinkageService().deleteTriggerSource(trigger->id);
        }
        
        // 6. 删除报警规则
        getAlarmService().deleteAlarmRule(alarmRule->id);
    }
    
    // 7. 删除点位
    auto dbResult = pointDb->deletePoint(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    status->message = "Point deleted";
    status->code = 204;
    return status;
}

// 获取其他服务的实例
AlarmService& PointService::getAlarmService() {
    return AlarmService::getInstance();
}

LinkageService& PointService::getLinkageService() {
    return LinkageService::getInstance();
}

vsoa::Object<PointValueDto> PointService::getPointValue(vsoa::String name) {
    // In a real implementation, this would come from nodeserver
    // For now, return simulated value
    auto pointValue = PointValueDto::createShared();
    pointValue->name = name;
    pointValue->value = "0";
    pointValue->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    pointValue->quality = true;
    
    return pointValue;
}

vsoa::Object<PointValueDto> PointService::getPointValueById(vsoa::UInt32 id) {
    // In a real implementation, this would come from nodeserver
    // For now, return simulated value
    auto point = getPointById(id);
    if (!point) {
        return nullptr;
    }
    
    auto pointValue = PointValueDto::createShared();
    pointValue->name = point->name;
    pointValue->value = "0";
    pointValue->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    pointValue->quality = true;
    
    return pointValue;
}

vsoa::Object<PointValueDto> PointService::setPointValue(vsoa::String name, vsoa::Object<PointWriteDto> writeDto) {
    // In a real implementation, this would send the write request to nodeserver
    // For now, just return the written value
    auto pointValue = PointValueDto::createShared();
    pointValue->name = name;
    pointValue->value = writeDto->value;
    pointValue->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    pointValue->quality = true;
    
    return pointValue;
}

vsoa::Object<PointValueDto> PointService::setPointValueById(vsoa::UInt32 id, vsoa::Object<PointWriteDto> writeDto) {
    // Check if point exists
    auto point = getPointById(id);
    if (!point) {
        return nullptr;
    }
    
    // In a real implementation, this would send the write request to nodeserver
    // For now, just return the written value
    auto pointValue = PointValueDto::createShared();
    pointValue->name = point->name;
    pointValue->value = writeDto->value;
    pointValue->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    pointValue->quality = true;
    
    return pointValue;
}
