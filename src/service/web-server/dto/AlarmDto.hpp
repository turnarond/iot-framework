/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/AlarmDto.hpp
 * @Description: 告警DTO定义，用于告警规则和告警实体的数据传输
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef ALARM_DTO_HPP
#define ALARM_DTO_HPP

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * 告警规则DTO
 * 用于告警规则信息的传输和展示
 */
class AlarmRuleDto : public vsoa::DTO {
    DTO_INIT(AlarmRuleDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");           ///< 告警规则ID
    DTO_FIELD(vsoa::UInt32, point_id, "point_id");         ///< 点位ID
    DTO_FIELD(vsoa::String, name, "name");            ///< 告警规则名称
    DTO_FIELD(vsoa::UInt32, method, "method");           ///< 告警方法
    DTO_FIELD(vsoa::Float32, threshold, "threshold");      ///< 告警阈值
    DTO_FIELD(vsoa::Float32, restore_threshold, "restore_threshold"); ///< 恢复阈值
    DTO_FIELD(vsoa::Float32, hysteresis, "hysteresis");     ///< 回差值
    DTO_FIELD(vsoa::Boolean, enable, "enable");         ///< 是否启用
    DTO_FIELD(vsoa::Boolean, enable_restore, "enable_restore"); ///< 是否启用恢复事件
};

/**
 * 告警规则创建DTO
 * 用于创建新告警规则时的数据传输
 */
class AlarmRuleCreateDto : public vsoa::DTO {
    DTO_INIT(AlarmRuleCreateDto, DTO)

    DTO_FIELD(vsoa::Int32, point_id, "point_id");  ///< 点位ID
    DTO_FIELD(vsoa::String, name, "name");         ///< 告警规则名称
    DTO_FIELD(vsoa::Int32, method, "method");      ///< 告警方法
    DTO_FIELD(vsoa::Float32, threshold, "threshold"); ///< 告警阈值
    DTO_FIELD(vsoa::Float32, restore_threshold, "restore_threshold"); ///< 恢复阈值
    DTO_FIELD(vsoa::Float32, hysteresis, "hysteresis"); ///< 回差值
    DTO_FIELD(vsoa::Boolean, enable, "enable");    ///< 是否启用
    DTO_FIELD(vsoa::Boolean, enable_restore, "enable_restore"); ///< 是否启用恢复事件
};

/**
 * 告警规则更新DTO
 * 用于更新告警规则信息时的数据传输
 */
class AlarmRuleUpdateDto : public vsoa::DTO {
    DTO_INIT(AlarmRuleUpdateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");         ///< 告警规则名称
    DTO_FIELD(vsoa::Int32, method, "method");      ///< 告警方法
    DTO_FIELD(vsoa::Float32, threshold, "threshold"); ///< 告警阈值
    DTO_FIELD(vsoa::Float32, restore_threshold, "restore_threshold"); ///< 恢复阈值
    DTO_FIELD(vsoa::Float32, hysteresis, "hysteresis"); ///< 回差值
    DTO_FIELD(vsoa::Boolean, enable, "enable");    ///< 是否启用
    DTO_FIELD(vsoa::Boolean, enable_restore, "enable_restore"); ///< 是否启用恢复事件
};

/**
 * 带点位信息的告警规则DTO
 * 用于同时展示告警规则和关联的点位信息
 */
class AlarmRuleWithPointDto : public vsoa::DTO {
    DTO_INIT(AlarmRuleWithPointDto, DTO)

    DTO_FIELD(vsoa::Int32, id, "id");           ///< 告警规则ID
    DTO_FIELD(vsoa::Int32, point_id, "point_id");         ///< 点位ID
    DTO_FIELD(vsoa::String, point_name, "point_name");      ///< 点位名称
    DTO_FIELD(vsoa::String, device_name, "device_name");     ///< 设备名称
    DTO_FIELD(vsoa::String, name, "name");            ///< 告警规则名称
    DTO_FIELD(vsoa::Int32, method, "method");           ///< 告警方法
    DTO_FIELD(vsoa::String, method_name, "method_name");     ///< 告警方法名称
    DTO_FIELD(vsoa::Float32, threshold, "threshold");      ///< 告警阈值
    DTO_FIELD(vsoa::Float32, restore_threshold, "restore_threshold"); ///< 恢复阈值
    DTO_FIELD(vsoa::Float32, hysteresis, "hysteresis");     ///< 回差值
    DTO_FIELD(vsoa::Boolean, enable, "enable");         ///< 是否启用
    DTO_FIELD(vsoa::Boolean, enable_restore, "enable_restore"); ///< 是否启用恢复事件
};

/**
 * 带点位信息的告警规则创建DTO
 * 用于创建新告警规则时的数据传输，包含点位信息
 */
class AlarmRuleWithPointCreateDto : public vsoa::DTO {
    DTO_INIT(AlarmRuleWithPointCreateDto, DTO)

    DTO_FIELD(vsoa::Int32, point_id, "point_id");         ///< 点位ID
    DTO_FIELD(vsoa::String, point_name, "point_name");      ///< 点位名称
    DTO_FIELD(vsoa::String, device_name, "device_name");     ///< 设备名称
    DTO_FIELD(vsoa::String, name, "name");            ///< 告警规则名称
    DTO_FIELD(vsoa::Int32, method, "method");           ///< 告警方法
    DTO_FIELD(vsoa::String, method_name, "method_name");     ///< 告警方法名称
    DTO_FIELD(vsoa::Float32, threshold, "threshold");      ///< 告警阈值
    DTO_FIELD(vsoa::Float32, restore_threshold, "restore_threshold"); ///< 恢复阈值
    DTO_FIELD(vsoa::Float32, hysteresis, "hysteresis");     ///< 回差值
    DTO_FIELD(vsoa::Boolean, enable, "enable");         ///< 是否启用
    DTO_FIELD(vsoa::Boolean, enable_restore, "enable_restore"); ///< 是否启用恢复事件
};

/**
 * 告警DTO
 * 用于告警信息的传输和展示
 */
class AlarmDto : public vsoa::DTO {
    DTO_INIT(AlarmDto, DTO)

    DTO_FIELD(vsoa::Int32, id);               ///< 告警ID
    DTO_FIELD(vsoa::Int32, rule_id);          ///< 告警规则ID
    DTO_FIELD(vsoa::Int32, point_id);         ///< 点位ID
    DTO_FIELD(vsoa::String, point_name);      ///< 点位名称
    DTO_FIELD(vsoa::String, alarm_name);      ///< 告警名称
    DTO_FIELD(vsoa::Int32, event_type);       ///< 事件类型（1=产生, 2=恢复）
    DTO_FIELD(vsoa::Float32, value);          ///< 触发时的实际值
    DTO_FIELD(vsoa::Float32, threshold);      ///< 告警阈值
    DTO_FIELD(vsoa::Float32, restore_threshold); ///< 恢复阈值
    DTO_FIELD(vsoa::String, message);         ///< 可读消息
    DTO_FIELD(vsoa::Int32, ack_status);       ///< 确认状态（0=未确认, 1=已确认）
    DTO_FIELD(vsoa::String, ack_user);        ///< 确认用户
    DTO_FIELD(vsoa::Int64, ack_time);         ///< 确认时间
    DTO_FIELD(vsoa::Int64, start_time);       ///< 报警开始时间
    DTO_FIELD(vsoa::Int64, end_time);         ///< 恢复时间
    DTO_FIELD(vsoa::Int64, duration_ms);      ///< 持续时间
    DTO_FIELD(vsoa::Boolean, is_active);      ///< 是否激活
    DTO_FIELD(vsoa::Int64, timestamp);        ///< 时间戳
    DTO_FIELD(vsoa::String, description);     ///< 告警描述
};

class AlarmRulePageDto : public PageDto<vsoa::Object<AlarmRuleDto>> {
    DTO_INIT(AlarmRulePageDto, PageDto<vsoa::Object<AlarmRuleDto>>)
};

class AlarmRuleWithPointPageDto : public PageDto<vsoa::Object<AlarmRuleWithPointDto>> {
    DTO_INIT(AlarmRuleWithPointPageDto, PageDto<vsoa::Object<AlarmRuleWithPointDto>>)
};

class AlarmPageDto : public PageDto<vsoa::Object<AlarmDto>> {
    DTO_INIT(AlarmPageDto, PageDto<vsoa::Object<AlarmDto>>)
};

#include VSOA_CODEGEN_END(DTO)

#endif // ALARM_DTO_HPP
