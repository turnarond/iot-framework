/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/DeviceService.cpp
 * @Description: Device Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "DeviceService.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "service/PointService.hpp"
#include "service/AlarmService.hpp"
#include "service/LinkageService.hpp"
#include <chrono>

// 设备相关方法
vsoa::Object<DevicesWithDriverPageDto> DeviceService::getDevices(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    // 获取总页数
    auto dbResult = deviceDb->getDeviceCount();
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
    
    // 获取设备列表
    dbResult = deviceDb->getDevicesWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    // 解析结果
    auto devices = dbResult->fetch<vsoa::Vector<vsoa::Object<DeviceWithDriverDto>>>();
    
    // 创建分页对象
    auto response = vsoa::Object<DevicesWithDriverPageDto>::createShared();
    response->items = devices;
    response->counts = devices->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

vsoa::Object<DeviceWithDriverDto> DeviceService::getDeviceById(vsoa::UInt32 id) 
{
    auto dbResult = deviceDb->getDeviceById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto devices = dbResult->fetch<vsoa::Vector<vsoa::Object<DeviceWithDriverDto>>>();
    if (devices->empty()) {
        return nullptr;
    }
    
    return devices->at(0);
}

vsoa::Object<DeviceWithDriverDto> DeviceService::createDevice(vsoa::Object<DeviceCreateDto> deviceCreateDto) 
{
    // 插入设备
    auto dbResult = deviceDb->createDevice(deviceCreateDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    // 获取新插入的设备ID
    auto newId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    
    return getDeviceById((vsoa::UInt32)newId);
}

vsoa::Object<DeviceWithDriverDto> DeviceService::updateDevice(vsoa::Object<DeviceDto> deviceDto) 
{
    // 更新设备
    auto dbResult = deviceDb->updateDevice(deviceDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    return getDeviceById(deviceDto->id);
}

vsoa::Object<StatusDto> DeviceService::deleteDevice(vsoa::UInt32 id) 
{
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (!getDeviceById(id)) {
        status->message = "Device not found";
        status->code = 404;
        return status;
    }
    
    // 1. 获取设备关联的所有点位
    auto dbResult = pointDb->getPointsByDeviceId(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto points = dbResult->fetch<vsoa::Vector<vsoa::Object<PointDto>>>();
    
    // 2. 遍历点位，处理关联数据
    for (auto& point : *points) {
        // 3. 获取点位关联的所有报警规则
        auto alarmDbResult = alarmDb->getAlarmRuleByPointId(point->id);
        OATPP_ASSERT_HTTP(alarmDbResult->isSuccess(), Status::CODE_500, alarmDbResult->getErrorMessage());
        auto alarmRules = alarmDbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleDto>>>();
        
        // 4. 遍历报警规则，处理关联数据
        for (auto& alarmRule : *alarmRules) {
            // 5. 获取报警规则关联的所有触发源
            auto triggers = getLinkageService().getTriggersByAlarmRuleId(alarmRule->id);
            
            // 6. 遍历触发源，处理关联数据
            for (auto& trigger : *triggers) {
                // 7. 删除触发源
                getLinkageService().deleteTriggerSource(trigger->id);
            }
            
            // 8. 删除报警规则
            getAlarmService().deleteAlarmRule(alarmRule->id);
        }
        
        // 9. 删除点位
        getPointService().deletePoint(point->id);
    }
    
    // 10. 删除设备
    auto dbResultDelete = deviceDb->deleteDevice(id);
    OATPP_ASSERT_HTTP(dbResultDelete->isSuccess(), Status::CODE_500, dbResultDelete->getErrorMessage());
    
    status->message = "OK";
    status->code = 200;
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return status;
}

// 获取其他服务的实例
PointService& DeviceService::getPointService() {
    return PointService::getInstance();
}

AlarmService& DeviceService::getAlarmService() {
    return AlarmService::getInstance();
}

LinkageService& DeviceService::getLinkageService() {
    return LinkageService::getInstance();
}
