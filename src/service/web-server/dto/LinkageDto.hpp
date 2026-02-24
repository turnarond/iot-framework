/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/LinkageDto.hpp
 * @Description: 联动DTO定义，用于联动规则和相关实体的数据传输
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef LINKAGE_DTO_HPP
#define LINKAGE_DTO_HPP

#include "PageDto.hpp"
#include "dto/StatusDto.hpp"
#include "vsoa_dto/core/Types.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * 事件类型DTO
 * 用于事件类型信息的传输和展示
 */
class EventTypeDto : public vsoa::DTO {
    DTO_INIT(EventTypeDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");             ///< 事件类型ID
    DTO_FIELD(vsoa::String, name, "name");           ///< 事件类型名称
    DTO_FIELD(vsoa::String, cname, "cname");          ///< 事件类型中文名称
    DTO_FIELD(vsoa::String, desc, "desc");           ///< 事件类型描述
    DTO_FIELD(vsoa::String, producer, "producer");       ///< 事件生产者
    DTO_FIELD(vsoa::String, description, "description");     ///< 事件描述
};


/**
 * 事件类型分页DTO
 * 用于分页展示事件类型列表
 */
class EventTypePageDto : public PageDto<vsoa::Object<EventTypeDto>> {
    DTO_INIT(EventTypePageDto, PageDto<vsoa::Object<EventTypeDto>>)
};

/**
 * 触发源创建DTO
 * 用于创建触发源的传输和展示
 */
class TriggerSourceCreateDto: public vsoa::DTO {
    DTO_INIT(TriggerSourceCreateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");           ///< 触发源名称
    DTO_FIELD(vsoa::UInt32, event_type_id, "event_type_id");  ///< 事件类型ID
    DTO_FIELD(vsoa::UInt32, alarm_rule_id, "alarm_rule_id");  ///< 关联报警规则ID（可选）
    DTO_FIELD(vsoa::UInt32, expr_id, "expr_id");      ///< 表达式ID（如果使用表达式触发）
    DTO_FIELD(vsoa::String, custom_event_key, "custom_event_key");///< 自定义事件键
    DTO_FIELD(vsoa::Boolean, enable, "enable");        ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description");    ///< 触发源描述

    DTO_FIELD_INFO(enable) {
        info->required = false;
    }
};

/**
 * 触发源DTO
 * 用于触发源信息的传输和展示
 */
class TriggerSourceDto :  public vsoa::DTO {
    DTO_INIT(TriggerSourceDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");              ///< 触发源ID
    DTO_FIELD(vsoa::String, name, "name");           ///< 触发源名称
    DTO_FIELD(vsoa::UInt32, event_type_id, "event_type_id");  ///< 事件类型ID
    DTO_FIELD(vsoa::UInt32, alarm_rule_id, "alarm_rule_id");  ///< 关联报警规则ID
    DTO_FIELD(vsoa::UInt32, expr_id, "expr_id");      ///< 表达式ID
    DTO_FIELD(vsoa::String, custom_event_key, "custom_event_key");///< 自定义事件键
    DTO_FIELD(vsoa::Boolean, enable, "enable");        ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description");    ///< 触发源描述

    DTO_FIELD_INFO(enable) {
        info->required = false;
    }
};

/**
 * 触发源分页DTO
 * 用于分页展示触发源列表
 */
class TriggerSourcePageDto : public PageDto<vsoa::Object<TriggerSourceDto>> {
    DTO_INIT(TriggerSourcePageDto, PageDto<vsoa::Object<TriggerSourceDto>>)
};

/**
 * 表达式DTO
 * 用于表达式信息的传输和展示
 */
class ExpressionDto : public vsoa::DTO {
    DTO_INIT(ExpressionDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");             ///< 表达式ID
    DTO_FIELD(vsoa::String, name, "name");           ///< 表达式名称
    DTO_FIELD(vsoa::String, expression, "expression"); ///< 表达式内容
    DTO_FIELD(vsoa::Boolean, enable, "enable");        ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description");    ///< 表达式描述
};

/**
 * 表达式创建DTO
 * 用于创建新表达式时的数据传输
 */
class ExpressionCreateDto : public vsoa::DTO {
    DTO_INIT(ExpressionCreateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");           ///< 表达式名称
    DTO_FIELD(vsoa::String, expression, "expression"); ///< 表达式内容
    DTO_FIELD(vsoa::Boolean, enable, "enable");        ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description");    ///< 表达式描述

    DTO_FIELD_INFO(enable) {
        info->required = false;
    }
};

/**
 * 表达式更新DTO
 * 用于更新表达式信息时的数据传输
 */
class ExpressionUpdateDto : public vsoa::DTO {
    DTO_INIT(ExpressionUpdateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");           ///< 表达式名称
    DTO_FIELD(vsoa::String, expression, "expression"); ///< 表达式内容
    DTO_FIELD(vsoa::Boolean, enable, "enable");        ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description");    ///< 表达式描述
};

/**
 * 表达式分页DTO
 * 用于分页展示表达式列表
 */
class ExpressionPageDto : public PageDto<vsoa::Object<ExpressionDto>> {
    DTO_INIT(ExpressionPageDto, PageDto<vsoa::Object<ExpressionDto>>)
};

/**
 * 动作类型DTO
 * 用于动作类型信息的传输和展示
 */
class ActionTypeDto :  public vsoa::DTO {
    DTO_INIT(ActionTypeDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");              ///< 动作类型ID
    DTO_FIELD(vsoa::String, name, "name");           ///< 动作类型名称
    DTO_FIELD(vsoa::String, libname, "libname");        ///< 库名称
    DTO_FIELD(vsoa::String, cname, "cname");          ///< 动作类型中文名称
    DTO_FIELD(vsoa::String, param1_name, "param1_name");    ///< 参数1名称
    DTO_FIELD(vsoa::String, param1_tip, "param1_tip");     ///< 参数1提示
    DTO_FIELD(vsoa::String, param2_name, "param2_name");    ///< 参数2名称
    DTO_FIELD(vsoa::String, param2_tip, "param2_tip");     ///< 参数2提示
    DTO_FIELD(vsoa::String, param3_name, "param3_name");    ///< 参数3名称
    DTO_FIELD(vsoa::String, param3_tip, "param3_tip");     ///< 参数3提示
    DTO_FIELD(vsoa::String, param4_name, "param4_name");    ///< 参数4名称
    DTO_FIELD(vsoa::String, param4_tip, "param4_tip");     ///< 参数4提示
};

/**
 * 动作类型分页DTO
 * 用于分页展示动作类型列表
 */
class ActionTypePageDto : public PageDto<vsoa::Object<ActionTypeDto>> {
    DTO_INIT(ActionTypePageDto, PageDto<vsoa::Object<ActionTypeDto>>)
};

/**
 * 动作实例DTO
 * 用于动作实例信息的传输和展示
 */
class ActionInstanceDto :  public vsoa::DTO {
    DTO_INIT(ActionInstanceDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");              ///< 动作实例ID
    DTO_FIELD(vsoa::String, name, "name");           ///< 动作实例名称
    DTO_FIELD(vsoa::UInt32, action_type_id, "action_type_id");  ///< 动作类型ID
    DTO_FIELD(vsoa::String, param1, "param1");         ///< 参数1值
    DTO_FIELD(vsoa::String, param2, "param2");         ///< 参数2值
    DTO_FIELD(vsoa::String, param3, "param3");         ///< 参数3值
    DTO_FIELD(vsoa::String, param4, "param4");         ///< 参数4值
    DTO_FIELD(vsoa::Boolean, need_confirm, "need_confirm");     ///< 是否需要确认
};

/**
 * 动作实例分页DTO
 * 用于分页展示动作实例列表
 */
class ActionInstancePageDto : public PageDto<vsoa::Object<ActionInstanceDto>> {
    DTO_INIT(ActionInstancePageDto, PageDto<vsoa::Object<ActionInstanceDto>>)
};


/**
 * 动作实例创建DTO
 * 用于创建新动作实例时的数据传输
 */
class ActionInstanceCreateDto :  public vsoa::DTO {
    DTO_INIT(ActionInstanceCreateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");               ///< 动作实例名称
    DTO_FIELD(vsoa::UInt32, action_type_id, "action_type_id"); ///< 动作类型ID
    DTO_FIELD(vsoa::String, param1, "param1");           ///< 参数1值
    DTO_FIELD(vsoa::String, param2, "param2");           ///< 参数2值
    DTO_FIELD(vsoa::String, param3, "param3");           ///< 参数3值
    DTO_FIELD(vsoa::String, param4, "param4");           ///< 参数4值
    DTO_FIELD(vsoa::Boolean, need_confirm, "need_confirm");   ///< 是否需要确认
    DTO_FIELD_INFO(need_confirm) {
        info->required = false;
    }
    DTO_FIELD_INFO(param1) {
        info->required = false;
    }
    DTO_FIELD_INFO(param2) {
        info->required = false;
    }
    DTO_FIELD_INFO(param3) {
        info->required = false;
    }
    DTO_FIELD_INFO(param4) {
        info->required = false;
    }

};

/**
 * 动作实例引用DTO
 * 用于引用动作实例时的数据传输
 */
class ActionRefDto :  public vsoa::DTO {
    DTO_INIT(ActionRefDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");              ///< 动作实例ID
};

/**
 * 动作实例更新DTO
 * 用于更新动作实例信息时的数据传输
 */
class ActionInstanceUpdateDto :  public vsoa::DTO {
    DTO_INIT(ActionInstanceUpdateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");               ///< 动作实例名称
    DTO_FIELD(vsoa::UInt32, action_type_id, "action_type_id"); ///< 动作类型ID
    DTO_FIELD(vsoa::String, param1, "param1");           ///< 参数1值
    DTO_FIELD(vsoa::String, param2, "param2");           ///< 参数2值
    DTO_FIELD(vsoa::String, param3, "param3");           ///< 参数3值
    DTO_FIELD(vsoa::String, param4, "param4");           ///< 参数4值
    DTO_FIELD(vsoa::Boolean, need_confirm, "need_confirm");   ///< 是否需要确认
};

/**
 * 联动规则DTO
 * 用于联动规则信息的传输和展示
 */
class LinkageRuleDto :  public vsoa::DTO {
    DTO_INIT(LinkageRuleDto, DTO)

    DTO_FIELD(vsoa::UInt32, id);              ///< 联动规则ID
    DTO_FIELD(vsoa::String, name);           ///< 联动规则名称
    DTO_FIELD(vsoa::String, logic_type);     ///< 逻辑类型
    DTO_FIELD(vsoa::Boolean, enable);         ///< 是否启用
    DTO_FIELD(vsoa::String, description);    ///< 联动规则描述
};

/**
 * 联动规则分页DTO
 * 用于分页展示联动规则列表
 */
class LinkageRulePageDto : public PageDto<vsoa::Object<LinkageRuleDto>> {
    DTO_INIT(LinkageRulePageDto, PageDto<vsoa::Object<LinkageRuleDto>>)
};

/**
 * 联动规则创建DTO
 * 用于创建新联动规则时的数据传输
 */
class LinkageRuleCreateDto :  public vsoa::DTO {
    DTO_INIT(LinkageRuleCreateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");           ///< 联动规则名称
    DTO_FIELD(vsoa::String, logic_type, "logic_type"); ///< 逻辑类型
    DTO_FIELD(vsoa::Boolean, enable, "enable");       ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description"); ///< 联动规则描述
};

/**
 * 联动规则更新DTO
 * 用于更新联动规则信息时的数据传输
 */
class LinkageRuleUpdateDto :  public vsoa::DTO {
    DTO_INIT(LinkageRuleUpdateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");           ///< 联动规则名称
    DTO_FIELD(vsoa::String, logic_type, "logic_type"); ///< 逻辑类型
    DTO_FIELD(vsoa::Boolean, enable, "enable");       ///< 是否启用
    DTO_FIELD(vsoa::String, description, "description"); ///< 联动规则描述
};

/**
 * 带详细信息的联动规则DTO
 * 用于同时展示联动规则和关联的详细信息
 */
class LinkageRuleWithDetailsDto :  public vsoa::DTO {
    DTO_INIT(LinkageRuleWithDetailsDto, DTO)

    DTO_FIELD(vsoa::UInt32, id);              ///< 联动规则ID
    DTO_FIELD(vsoa::String, name);           ///< 联动规则名称
    DTO_FIELD(vsoa::String, action_ids);     ///< 动作ID 列表（CSV）
    DTO_FIELD(vsoa::String, action_names);   ///< 动作名称 列表（分号分隔）
    DTO_FIELD(vsoa::String, logic_type);     ///< 逻辑类型
    DTO_FIELD(vsoa::Boolean, enable);         ///< 是否启用
    DTO_FIELD(vsoa::String, description);    ///< 联动规则描述
    DTO_FIELD(vsoa::String, triggers);       ///< 触发源ID数组（JSON格式）
};

class LinkageRuleWithDetailPageDto : public PageDto<vsoa::Object<LinkageRuleWithDetailsDto>> {
    DTO_INIT(LinkageRuleWithDetailPageDto, PageDto<vsoa::Object<LinkageRuleWithDetailsDto>>)
};

/**
 * 联动规则完整 DTO（包含触发源与动作详情），用于编辑时返回
 */
class LinkageRuleFullDto : public vsoa::DTO {
    DTO_INIT(LinkageRuleFullDto, DTO)

    DTO_FIELD(vsoa::UInt32, id);
    DTO_FIELD(vsoa::String, name);
    DTO_FIELD(vsoa::String, logic_type);
    DTO_FIELD(vsoa::Boolean, enable);
    DTO_FIELD(vsoa::String, description);
    DTO_FIELD(vsoa::Vector<vsoa::Object<TriggerSourceDto>>, triggers);
    DTO_FIELD(vsoa::Vector<vsoa::Object<ActionInstanceDto>>, actions);
};

/**
 * 规则触发关联DTO
 * 用于联动规则和触发源的关联关系
 */
class RuleTriggerDto :  public vsoa::DTO {
    DTO_INIT(RuleTriggerDto, DTO)

    DTO_FIELD(vsoa::UInt32, rule_id);         ///< 联动规则ID
    DTO_FIELD(vsoa::UInt32, trigger_id);      ///< 触发源ID
};

/**
 * 触发引用 DTO（用于在创建联动规则时引用已存在的触发源）
 */
class TriggerRefDto : public vsoa::DTO {
    DTO_INIT(TriggerRefDto, DTO)

    DTO_FIELD(vsoa::UInt32, source_id);       ///< 触发源 ID
};

/**
 * 联动规则创建（包含关联触发与动作）
 * 前端可以一次性提交 triggers（触发源引用数组）和 actions（动作实例创建数组）
 */
class LinkageRuleCreateFullDto : public vsoa::DTO {
    DTO_INIT(LinkageRuleCreateFullDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");
    DTO_FIELD(vsoa::String, logic_type, "logic_type");
    DTO_FIELD(vsoa::Boolean, enable, "enable");
    DTO_FIELD(vsoa::String, description, "description");
    DTO_FIELD(vsoa::Vector<vsoa::Object<TriggerRefDto>>, triggers, "triggers");
    DTO_FIELD(vsoa::Vector<vsoa::Object<ActionRefDto>>, actions, "actions");
};

/**
 * 联动日志DTO
 * 用于联动执行日志的传输和展示
 */
class LinkageLogDto :  public vsoa::DTO {
    DTO_INIT(LinkageLogDto, DTO)

    DTO_FIELD(vsoa::UInt32, id);              ///< 日志ID
    DTO_FIELD(vsoa::UInt32, rule_id);         ///< 联动规则ID
    DTO_FIELD(vsoa::String, rule_name);      ///< 联动规则名称
    DTO_FIELD(vsoa::String, triggered_triggers); ///< 触发的触发源
    DTO_FIELD(vsoa::UInt32, action_id);       ///< 动作ID
    DTO_FIELD(vsoa::String, action_name);    ///< 动作名称
    DTO_FIELD(vsoa::Int32, result);          ///< 执行结果
    DTO_FIELD(vsoa::String, trigger_time);   ///< 触发时间
    DTO_FIELD(vsoa::UInt32, duration_ms);     ///< 执行时长（毫秒）
    DTO_FIELD(vsoa::String, raw_payload);    ///< 原始负载
};


/**
 * 联动日志分页DTO
 * 用于分页展示联动执行日志
 */
class LinkageLogPageDto : public PageDto<vsoa::Object<LinkageLogDto>> {
    DTO_INIT(LinkageLogPageDto, PageDto<vsoa::Object<LinkageLogDto>>)
};

#include VSOA_CODEGEN_END(DTO)

#endif // LINKAGE_DTO_HPP
