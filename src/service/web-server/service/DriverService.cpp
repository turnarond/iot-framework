/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/DriverService.cpp
 * @Description: Driver Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "DriverService.hpp"
#include "common/Logger.hpp"
#include "lwlog/lwlog.h"
#include "vsoa_dto/core/Types.hpp"
#include "db/DeviceDb.hpp"
#include "db/PointDb.hpp"
#include "db/AlarmDb.hpp"
#include "db/LinkageDb.hpp"
#include <chrono>

// 驱动相关方法
vsoa::Object<DriversPageDto> DriverService::getDrivers(vsoa::UInt32 page, vsoa::UInt32 size) 
{   
    // 获取总页数
    auto dbResult = driverDb->getDriverCount();
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
    
    // 获取驱动列表
    dbResult = driverDb->getDriversWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<DriverDto>>>();
    
    auto dto = vsoa::Object<DriversPageDto>::createShared();
    dto->items = items;
    dto->counts = items->size();
    dto->page = page;
    dto->size = size;
    dto->pages = pages;
    
    return dto;
}

vsoa::Object<DriverDto> DriverService::getDriverById(vsoa::UInt32 id) 
{
    auto dbResult = driverDb->getDriverById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Driver not found: " + std::to_string(id));

    auto result = dbResult->fetch<vsoa::Vector<vsoa::Object<DriverDto>>>();
    OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

    return result->front();
}

vsoa::Object<DriverDto> DriverService::createDriver(vsoa::Object<DriverCreateDto> driverDto) 
{
    // 获取当前时间戳（毫秒）
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto create_time = static_cast<vsoa::UInt64>(now);
    
    auto dbResult = driverDb->createDriver(driverDto, create_time);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    // 获取插入的ID
    // auto result = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    // OATPP_ASSERT_HTTP(result->size() > 0, Status::CODE_500, "Failed to get insert ID");
    auto newId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Create Driver %s success, id is %ld.",
        driverDto->name->c_str(), newId);
    return getDriverById(newId);
}

vsoa::Object<DriverDto> DriverService::updateDriver(vsoa::Object<DriverDto> driverDto) 
{
    auto dbResult = driverDb->updateDriver(driverDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return getDriverById(driverDto->id);
}

vsoa::Object<StatusDto> DriverService::deleteDriver(vsoa::UInt32 id) 
{
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (!getDriverById(id)) {
        status->message = "Driver not found";
        status->code = 404;
        return status;
    }
    
    // 1. 获取驱动关联的所有设备
    auto dbResult = deviceDb->getDevicesByDriverId(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto devices = dbResult->fetch<vsoa::Vector<vsoa::Object<DeviceWithDriverDto>>>();
    
    // 2. 遍历设备，处理关联数据
    for (auto& device : *devices) {
        // 3. 获取设备关联的所有点位
        auto pointDbResult = pointDb->getPointsByDeviceId(device->id);
        OATPP_ASSERT_HTTP(pointDbResult->isSuccess(), Status::CODE_500, pointDbResult->getErrorMessage());
        auto points = pointDbResult->fetch<vsoa::Vector<vsoa::Object<PointDto>>>();
        
        // 4. 遍历点位，处理关联数据
        for (auto& point : *points) {
            // 5. 获取点位关联的所有报警规则
            auto alarmDbResult = alarmDb->getAlarmRuleByPointId(point->id);
            OATPP_ASSERT_HTTP(alarmDbResult->isSuccess(), Status::CODE_500, alarmDbResult->getErrorMessage());
            auto alarmRules = alarmDbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleDto>>>();
            
            // 6. 遍历报警规则，处理关联数据
            for (auto& alarmRule : *alarmRules) {
                // 7. 获取报警规则关联的所有触发源
                auto triggers = getLinkageService().getTriggersByAlarmRuleId(alarmRule->id);
                
                // 8. 遍历触发源，处理关联数据
                for (auto& trigger : *triggers) {
                    // 9. 这里可以添加获取触发源关联的联动规则的逻辑
                    // 由于联动规则与触发源是多对多关系，需要先删除关联表中的记录
                    // 然后再删除触发源
                    getLinkageService().deleteTriggerSource(trigger->id);
                }
                
                // 10. 删除报警规则
                getAlarmService().deleteAlarmRule(alarmRule->id);
            }
            
            // 11. 删除点位
            getPointService().deletePoint(point->id);
        }
        
        // 12. 删除设备
        getDeviceService().deleteDevice(device->id);
    }
    
    // 13. 删除驱动
    auto dbResultDelete = driverDb->deleteDriver(id);
    OATPP_ASSERT_HTTP(dbResultDelete->isSuccess(), Status::CODE_500, dbResultDelete->getErrorMessage());
    
    status->message = "OK";
    status->code = 200;
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return status;
}

vsoa::Object<StatusDto> DriverService::checkDriverDependencies(vsoa::UInt32 driverId) 
{
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 检查驱动是否存在
    if (!getDriverById(driverId)) {
        status->message = "Driver not found";
        status->code = 404;
        return status;
    }
    
    // 获取驱动关联的所有设备
    auto dbResult = deviceDb->getDevicesByDriverId(driverId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto devices = dbResult->fetch<vsoa::Vector<vsoa::Object<DeviceWithDriverDto>>>();
    
    if (devices->empty()) {
        status->message = "No dependencies found";
        status->code = 200;
        return status;
    }
    
    unsigned int deviceCount = 0;
    unsigned int pointCount = 0;
    unsigned int alarmRuleCount = 0;
    unsigned int triggerCount = 0;
    
    // 遍历设备，统计关联数据
    for (auto& device : *devices) {
        deviceCount++;
        
        // 获取设备关联的所有点位
        auto pointDbResult = pointDb->getPointsByDeviceId(device->id);
        OATPP_ASSERT_HTTP(pointDbResult->isSuccess(), Status::CODE_500, pointDbResult->getErrorMessage());
        auto points = pointDbResult->fetch<vsoa::Vector<vsoa::Object<PointDto>>>();
        
        for (auto& point : *points) {
            pointCount++;
            
            // 获取点位关联的所有报警规则
            auto alarmDbResult = alarmDb->getAlarmRuleByPointId(point->id);
            OATPP_ASSERT_HTTP(alarmDbResult->isSuccess(), Status::CODE_500, alarmDbResult->getErrorMessage());
            auto alarmRules = alarmDbResult->fetch<vsoa::Vector<vsoa::Object<AlarmRuleDto>>>();
            
            for (auto& alarmRule : *alarmRules) {
                alarmRuleCount++;
                
                // 获取报警规则关联的所有触发源
                auto triggers = getLinkageService().getTriggersByAlarmRuleId(alarmRule->id);
                triggerCount += triggers->size();
            }
        }
    }
    
    status->message = "Dependencies found: " + std::to_string(deviceCount) + " devices, " + 
                     std::to_string(pointCount) + " points, " + 
                     std::to_string(alarmRuleCount) + " alarm rules, " + 
                     std::to_string(triggerCount) + " triggers";
    status->code = 200;
    return status;
}
