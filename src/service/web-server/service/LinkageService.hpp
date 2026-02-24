/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/LinkageService.hpp
 * @Description: Linkage Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef LINKAGE_SERVICE_HPP
#define LINKAGE_SERVICE_HPP

#include "dto/LinkageDto.hpp"
#include "dto/StatusDto.hpp"

#include "db/LinkageDb.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/data/mapping/type/Type.hpp"

class LinkageService {
private:
  typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<LinkageDb>, linkageDb);

public:
    // 单例模式
    static LinkageService& getInstance() {
        static LinkageService instance;
        return instance;
    }

    /**
     * 获取所有事件类型
     * @return 事件类型列表
     */
    vsoa::Object<EventTypePageDto> getEventTypes(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 获取所有表达式
     * @return 表达式列表
     */
    vsoa::Object<ExpressionPageDto> getExpressions(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取表达式
     * @param id 表达式ID
     * @return 表达式信息
     */
    vsoa::Object<ExpressionDto> getExpressionById(vsoa::UInt32 id);

    /**
     * 创建表达式
     * @param expressionDto 表达式创建数据
     * @return 创建的表达式信息
     */
    vsoa::Object<ExpressionDto> createExpression(vsoa::Object<ExpressionCreateDto> expressionDto);

    /**
     * 更新表达式
     * @param expressionDto 表达式更新数据
     * @return 更新后的表达式信息
     */
    vsoa::Object<ExpressionDto> updateExpression(vsoa::Object<ExpressionDto> expressionDto);

    /**
     * 删除表达式
     * @param id 表达式ID
     * @return 删除结果
     */
    vsoa::Object<StatusDto> deleteExpression(vsoa::UInt32 id);

    /**
     * 获取所有触发源
     * @return 触发源列表
     */
    vsoa::Object<TriggerSourcePageDto> getTriggerSources(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取触发源
     * @param id 触发源ID
     * @return 触发源信息
     */
    vsoa::Object<TriggerSourceDto> getTriggerSourceById(vsoa::UInt32 id);

    /**
     * 创建触发源
     * @param triggerDto 触发源创建数据
     * @return 创建的触发源信息
     */
    vsoa::Object<TriggerSourceDto> createTriggerSource(vsoa::Object<TriggerSourceCreateDto> triggerDto);

    /**
     * 更新触发源
     * @param id 触发源ID
     * @param triggerDto 触发源更新数据
     * @return 更新后的触发源信息
     */
    vsoa::Object<TriggerSourceDto> updateTriggerSource(vsoa::Object<TriggerSourceDto> triggerDto);

    /**
     * 删除触发源
     * @param id 触发源ID
     * @return 删除结果
     */
    vsoa::Object<StatusDto> deleteTriggerSource(vsoa::UInt32 id);
    vsoa::Vector<vsoa::Object<TriggerSourceDto>> getTriggersByAlarmRuleId(vsoa::UInt32 alarmRuleId);

    /**
     * 获取所有动作类型
     * @return 动作类型列表
     */
    vsoa::Object<ActionTypePageDto> getActionTypes(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 获取所有动作实例
     * @return 动作实例列表
     */
    vsoa::Object<ActionInstancePageDto> getActionInstances(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取动作实例
     * @param id 动作实例ID
     * @return 动作实例信息
     */
    vsoa::Object<ActionInstanceDto> getActionInstanceById(vsoa::UInt32 id);

    /**
     * 创建动作实例
     * @param actionDto 动作实例创建数据
     * @return 创建的动作实例信息
     */
    vsoa::Object<ActionInstanceDto> createActionInstance(vsoa::Object<ActionInstanceCreateDto> actionDto);

    /**
     * 更新动作实例
     * @param id 动作实例ID
     * @param actionDto 动作实例更新数据
     * @return 更新后的动作实例信息
     */
    vsoa::Object<ActionInstanceDto> updateActionInstance(vsoa::Object<ActionInstanceDto> actionDto);

    /**
     * 删除动作实例
     * @param id 动作实例ID
     * @return 删除结果
     */
    vsoa::Object<StatusDto> deleteActionInstance(vsoa::UInt32 id);

    /**
     * 获取所有联动规则
     * @param page 页码
     * @param size 每页数量
     * @return 联动规则列表
     */
    vsoa::Object<LinkageRulePageDto> getLinkageRules(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 获取所有联动规则（包含详细信息）
     * @param page 页码
     * @param size 每页数量
     * @return 联动规则列表
     */
    vsoa::Object<LinkageRuleWithDetailPageDto> getLinkageRulesWithDetails(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取联动规则
     * @param id 联动规则ID
     * @return 联动规则信息
     */
    vsoa::Object<LinkageRuleDto> getLinkageRuleById(vsoa::UInt32 id);

    /**
     * 获取联动规则完整信息（包含触发源详情与动作实例详情）
     */
    vsoa::Object<LinkageRuleFullDto> getLinkageRuleWithDetails(vsoa::UInt32 id);

    /**
     * 创建联动规则
     * @param ruleDto 联动规则创建数据
     * @return 创建的联动规则信息
     */
    vsoa::Object<LinkageRuleDto> createLinkageRule(vsoa::Object<LinkageRuleCreateDto> ruleDto);

    /**
     * 创建联动规则并同时关联触发源与动作（原子式批量创建/关联）
     * @param ruleDto 包含 triggers (source_id 数组) 与 actions (ActionInstanceCreateDto 数组)
     * @return 创建的联动规则信息
     */
    vsoa::Object<LinkageRuleDto> createLinkageRuleWithRelations(vsoa::Object<LinkageRuleCreateFullDto> ruleDto);

    /**
     * 更新联动规则
     * @param id 联动规则ID
     * @param ruleDto 联动规则更新数据
     * @return 更新后的联动规则信息
     */
    vsoa::Object<LinkageRuleDto> updateLinkageRule(vsoa::Object<LinkageRuleDto> ruleDto);

    /**
     * 删除联动规则
     * @param id 联动规则ID
     * @return 删除结果
     */
    vsoa::Object<StatusDto> deleteLinkageRule(vsoa::UInt32 id);

    /**
     * 为联动规则添加触发源
     * @param ruleId 联动规则ID
     * @param triggerId 触发源ID
     * @return 添加结果
     */
    vsoa::Object<StatusDto> addTriggerToRule(vsoa::UInt32 ruleId, vsoa::UInt32 triggerId);

    /**
     * 为联动规则添加动作关联
     */
    vsoa::Object<StatusDto> addActionToRule(vsoa::UInt32 ruleId, vsoa::UInt32 actionId);

    /**
     * 从联动规则中移除触发源
     * @param ruleId 联动规则ID
     * @param triggerId 触发源ID
     * @return 移除结果
     */
    vsoa::Object<StatusDto> removeTriggerFromRule(vsoa::UInt32 ruleId, vsoa::UInt32 triggerId);

    /**
     * 获取所有联动日志
     * @param page 页码
     * @param size 每页数量
     * @return 联动日志列表
     */
    vsoa::Object<LinkageLogPageDto> getLinkageLogs(vsoa::UInt32 page, vsoa::UInt32 size);

private:
    // 私有构造函数，防止外部创建实例
    LinkageService() = default;
    ~LinkageService() = default;
    // 禁用拷贝和赋值
    LinkageService(const LinkageService&) = delete;
    LinkageService& operator=(const LinkageService&) = delete;
};

#endif // LINKAGE_SERVICE_HPP