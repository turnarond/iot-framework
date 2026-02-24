/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/AlarmService.hpp
 * @Description: Alarm Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef ALARM_SERVICE_HPP
#define ALARM_SERVICE_HPP

#include "dto/AlarmDto.hpp"
#include "dto/StatusDto.hpp"

#include "db/AlarmDb.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/data/mapping/type/Type.hpp"

class AlarmService {
private:
  typedef vsoa::web::protocol::http::Status Status;
private:
    VSOA_COMPONENT(std::shared_ptr<AlarmDb>, alarmDb);

public:
    // 单例模式
    static AlarmService& getInstance() {
        static AlarmService instance;
        return instance;
    }

    /**
     * 获取所有告警规则
     * @return 告警规则列表
     */
    vsoa::Object<AlarmRulePageDto> getAlarmRules(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 获取所有告警规则（包含点位信息）
     * @return 告警规则列表
     */
    vsoa::Object<AlarmRuleWithPointPageDto> getAlarmRulesWithPoint(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取告警规则
     * @param id 告警规则ID
     * @return 告警规则信息
     */
    vsoa::Object<AlarmRuleDto> getAlarmRuleById(vsoa::UInt32 id);

    /**
     * 根据ID获取告警规则（包含点位信息）
     * @param id 告警规则ID
     * @return 告警规则信息
     */
    vsoa::Object<AlarmRuleWithPointDto> getAlarmRuleWithPointById(vsoa::UInt32 id);

    /**
     * 创建告警规则
     * @param ruleDto 告警规则创建数据
     * @return 创建的告警规则信息
     */
    vsoa::Object<AlarmRuleDto> createAlarmRule(vsoa::Object<AlarmRuleCreateDto> ruleDto);

    /**
     * 创建告警规则（包含点位信息）
     * @param ruleDto 告警规则创建数据
     * @return 创建的告警规则信息
     */
    vsoa::Object<AlarmRuleWithPointDto> createAlarmRuleWithPoint(vsoa::Object<AlarmRuleWithPointCreateDto> ruleDto);

    /**
     * 更新告警规则
     * @param id 告警规则ID
     * @param ruleDto 告警规则更新数据
     * @return 更新后的告警规则信息
     */
    vsoa::Object<AlarmRuleDto> updateAlarmRule(vsoa::Object<AlarmRuleDto> ruleDto);

    /**
     * 删除告警规则
     * @param id 告警规则ID
     * @return 删除结果
     */
    vsoa::Object<StatusDto> deleteAlarmRule(vsoa::UInt32 id);

    /**
     * 根据点位ID获取告警规则（包含点位信息）
     * @param pointId 点位ID
     * @return 告警规则列表
     */
    vsoa::Vector<vsoa::Object<AlarmRuleWithPointDto>> getAlarmRulesByPointId(vsoa::UInt32 pointId);

    /**
     * 获取所有告警
     * @return 告警列表
     */
    vsoa::Object<AlarmPageDto> getAlarms(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 获取活跃告警
     * @return 活跃告警列表
     */
    vsoa::Object<AlarmPageDto> getActiveAlarms(vsoa::UInt32 page, vsoa::UInt32 size);

private:
    // 私有构造函数，防止外部创建实例
    AlarmService() = default;
    ~AlarmService() = default;
    // 禁用拷贝和赋值
    AlarmService(const AlarmService&) = delete;
    AlarmService& operator=(const AlarmService&) = delete;
};

#endif // ALARM_SERVICE_HPP