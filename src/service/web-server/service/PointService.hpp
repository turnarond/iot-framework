/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/PointService.hpp
 * @Description: Point Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef POINT_SERVICE_HPP
#define POINT_SERVICE_HPP

#include "dto/PointDto.hpp"
#include "dto/StatusDto.hpp"
#include "dto/AlarmDto.hpp"

#include "db/PointDb.hpp"
#include "db/AlarmDb.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"
#include "vsoa_dto/core/Types.hpp"

class AlarmService;
class LinkageService;

class PointService {
private:
    typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<PointDb>, pointDb);
    VSOA_COMPONENT(std::shared_ptr<AlarmDb>, alarmDb);

private:
    // 获取其他服务的实例
    AlarmService& getAlarmService();
    LinkageService& getLinkageService();

public:
    // 单例模式
    static PointService& getInstance() {
        static PointService instance;
        return instance;
    }

    /**
     * 获取所有点位
     * @return 点位列表
     */
    vsoa::Object<PointPageDto> getPoints(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 获取所有点位（包含设备信息）
     * @return 点位列表
     */
    vsoa::Object<PointWithDevicePageDto> getPointsWithDevice(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取点位
     * @param id 点位ID
     * @return 点位信息
     */
    vsoa::Object<PointDto> getPointById(vsoa::UInt32 id);

    /**
     * 根据ID获取点位（包含设备信息）
     * @param id 点位ID
     * @return 点位信息
     */
    vsoa::Object<PointWithDeviceDto> getPointWithDeviceById(vsoa::UInt32 id);

    /**
     * 创建点位
     * @param pointCreateDto 点位创建数据
     * @return 创建的点位信息
     */
    vsoa::Object<PointDto> createPoint(vsoa::Object<PointCreateDto> pointCreateDto);

    /**
     * 更新点位
     * @param id 点位ID
     * @param pointUpdateDto 点位更新数据
     * @return 更新后的点位信息
     */
    vsoa::Object<PointDto> updatePoint(vsoa::Object<PointDto> pointUpdateDto);

    /**
     * 删除点位
     * @param id 点位ID
     * @return 删除结果
     */
    vsoa::Object<StatusDto> deletePoint(vsoa::UInt32 id);

    // /**
    //  * 获取所有点位值
    //  * @return 点位值列表
    //  */
    // vsoa::Vector<vsoa::Object<PointValueDto>> getPointValues();

    /**
     * 获取点位值
     * @param name 点位名称
     * @return 点位值信息
     */
    vsoa::Object<PointValueDto> getPointValue(vsoa::String name);

    /**
     * 根据ID获取点位值
     * @param id 点位ID
     * @return 点位值信息
     */
    vsoa::Object<PointValueDto> getPointValueById(vsoa::UInt32 id);

    /**
     * 设置点位值
     * @param name 点位名称
     * @param writeDto 写入值数据
     * @return 写入后的点位值信息
     */
    vsoa::Object<PointValueDto> setPointValue(vsoa::String name, vsoa::Object<PointWriteDto> writeDto);

    /**
     * 根据ID设置点位值
     * @param id 点位ID
     * @param writeDto 写入值数据
     * @return 写入后的点位值信息
     */
    vsoa::Object<PointValueDto> setPointValueById(vsoa::UInt32 id, vsoa::Object<PointWriteDto> writeDto);

private:
    // 私有构造函数，防止外部创建实例
    PointService() = default;
    ~PointService() = default;
    // 禁用拷贝和赋值
    PointService(const PointService&) = delete;
    PointService& operator=(const PointService&) = delete;
};

#endif // POINT_SERVICE_HPP