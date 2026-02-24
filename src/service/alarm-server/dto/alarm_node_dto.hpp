/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 10:31:13
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-05 18:12:05
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/dto/alarm_node_dto.hpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/base/Environment.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

class AlarmInfoDto : public vsoa::DTO
{
    DTO_INIT(AlarmInfoDto, DTO)
    DTO_FIELD(UInt64, time, "time");
    DTO_FIELD(String, moduleId, "moduleId");
    DTO_FIELD(String, nodeId, "nodeId");
    DTO_FIELD(String, value, "value");
    DTO_FIELD_INFO(nodeId) {
        info->required = true;
    }
    DTO_FIELD_INFO(value) {
        info->required = true;
    }
};

class AlarmDataDto : public vsoa::DTO
{ 
    DTO_INIT(AlarmDataDto, DTO)
    DTO_FIELD(vsoa::Vector<vsoa::Object<AlarmInfoDto> >, alarms, "alarms");

};

class AlarmNodeDto : public vsoa::DTO
{
    DTO_INIT(AlarmNodeDto, DTO)
    DTO_FIELD(String, timestamp, "timestamp");
    DTO_FIELD(vsoa::Object<AlarmDataDto>, data, "data");
    DTO_FIELD_INFO(data) {
        info->required = true;
    }
};

ENUM  
(
    AlarmMessageType, 
    v_uint8,
    VALUE(ALARM_CLEAR, 1),
    VALUE(ALARM_TRIGGER, 2)
)

class AlarmMessageDto : public vsoa::DTO
{
    DTO_INIT(AlarmMessageDto, DTO)
    DTO_FIELD(String, point_id, "point_id");
    DTO_FIELD(String, point_name, "point_name");
    DTO_FIELD(String, point_value, "point_value");
    DTO_FIELD(String, rule_name, "rule_name");
    DTO_FIELD(Int32, rule_id, "rule_id");
    DTO_FIELD(Int32, rule_method, "rule_method");
    DTO_FIELD(UInt64, timestamp, "timestamp");
    DTO_FIELD(Enum<AlarmMessageType>, message_type, "trigger_type");
    DTO_FIELD_INFO(point_id) {
        info->required = true;
    }
    DTO_FIELD_INFO(point_value) {
        info->required = true;
    }
};