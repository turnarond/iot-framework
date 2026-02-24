/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/DriverService.hpp
 * @Description: Driver Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DRIVER_SERVICE_HPP
#define DRIVER_SERVICE_HPP

#include "dto/DriverDto.hpp"
#include "dto/PageDto.hpp"
#include "dto/StatusDto.hpp"

#include "db/DriverDb.hpp"
#include "db/DeviceDb.hpp"
#include "db/PointDb.hpp"

#include "service/DeviceService.hpp"
#include "service/PointService.hpp"
#include "service/AlarmService.hpp"
#include "service/LinkageService.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

class DriverService {
private:
    typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<DriverDb>, driverDb);
    VSOA_COMPONENT(std::shared_ptr<DeviceDb>, deviceDb);
    VSOA_COMPONENT(std::shared_ptr<PointDb>, pointDb);
    VSOA_COMPONENT(std::shared_ptr<AlarmDb>, alarmDb);
    VSOA_COMPONENT(std::shared_ptr<LinkageDb>, linkageDb);
    
    // 使用单例模式获取服务实例
    DeviceService& getDeviceService() { return DeviceService::getInstance(); }
    PointService& getPointService() { return PointService::getInstance(); }
    AlarmService& getAlarmService() { return AlarmService::getInstance(); }
    LinkageService& getLinkageService() { return LinkageService::getInstance(); }

public:
    // 驱动相关方法
    vsoa::Object<DriversPageDto> getDrivers(vsoa::UInt32 page, vsoa::UInt32 size);
    vsoa::Object<DriverDto> getDriverById(vsoa::UInt32 id);
    vsoa::Object<DriverDto> createDriver(vsoa::Object<DriverCreateDto> driverDto);
    vsoa::Object<DriverDto> updateDriver(vsoa::Object<DriverDto> driverDto);
    vsoa::Object<StatusDto> deleteDriver(vsoa::UInt32 id);
    
    // 关联检查方法
    vsoa::Object<StatusDto> checkDriverDependencies(vsoa::UInt32 driverId);
};

#endif // DRIVER_SERVICE_HPP
