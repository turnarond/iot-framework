/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 10:31:13
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-05 14:10:23
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkevt_alarm/alarm_node_dto.hpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/base/Environment.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

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