/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-27 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/DeviceService.hpp
 * @Description: Device Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DEVICE_SERVICE_HPP
#define DEVICE_SERVICE_HPP

#include "dto/DriverDto.hpp"
#include "dto/DeviceDto.hpp"
#include "dto/StatusDto.hpp"
#include "dto/PointDto.hpp"
#include "dto/AlarmDto.hpp"
#include "db/DeviceDb.hpp"
#include "db/PointDb.hpp"
#include "db/AlarmDb.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

class PointService;
class AlarmService;
class LinkageService;

class DeviceService {
private:
    typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<DeviceDb>, deviceDb);
    VSOA_COMPONENT(std::shared_ptr<PointDb>, pointDb);
    VSOA_COMPONENT(std::shared_ptr<AlarmDb>, alarmDb);

private:
    // 获取其他服务的实例
    PointService& getPointService();
    AlarmService& getAlarmService();
    LinkageService& getLinkageService();

public:
    // 单例模式
    static DeviceService& getInstance() {
        static DeviceService instance;
        return instance;
    }

    // 设备相关方法
    vsoa::Object<DevicesWithDriverPageDto> getDevices(vsoa::UInt32 page, vsoa::UInt32 size);
    vsoa::Object<DeviceWithDriverDto> getDeviceById(vsoa::UInt32 id);
    vsoa::Object<DeviceWithDriverDto> createDevice(vsoa::Object<DeviceCreateDto> deviceDto);
    vsoa::Object<DeviceWithDriverDto> updateDevice(vsoa::Object<DeviceDto> deviceDto);
    vsoa::Object<StatusDto> deleteDevice(vsoa::UInt32 id);

private:
    // 私有构造函数，防止外部创建实例
    DeviceService() = default;
    ~DeviceService() = default;
    // 禁用拷贝和赋值
    DeviceService(const DeviceService&) = delete;
    DeviceService& operator=(const DeviceService&) = delete;
};

#endif // DEVICE_SERVICE_HPP