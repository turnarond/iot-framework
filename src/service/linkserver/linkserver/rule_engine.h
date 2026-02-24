/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:23:10
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-30 17:12:03
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/rule_engine.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "link_define.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <set>

#include "Poco/Data/Session.h"

// （内存规则状态）
struct RuleInstance {
    int rule_id;
    std::string rule_name;
    int action_id;
    std::string logic_type; // "AND" or "OR"
    bool enabled;
    std::set<int> required_trigger_ids; // 来自 t_rule_trigger
    std::set<int> matched_triggers;     // 已命中的 trigger_id

    bool isSatisfied() const {
        if (logic_type == "OR") {
            return !matched_triggers.empty();
        } else { // AND
            return matched_triggers.size() == required_trigger_ids.size();
        }
    }

    void reset() {
        matched_triggers.clear();
    }
};

class CRuleEngine
{
public:
    static CRuleEngine* GetInstance()
    {
        static CRuleEngine instance;
        return &instance;
    }
public:

    // 从数据库加载所有启用的规则
    int loadRules(const std::string& dbPath);

    int reloadTriggers(int id, bool enable);

    // 事件触发入口（由 EventSource 调用）
    void OnEvent(int trigger_id, const std::string& payload = "");

    // 获取需要加载的触发器和动作（供 Manager 使用）
    const std::vector<CTrigger*>& getRequiredTriggers() const { return triggers_; }
    const std::vector<CLinkAction*>& getRequiredActions() const { return actions_; }

private:
    CRuleEngine() = default;
    ~CRuleEngine() = default;
    CRuleEngine(const CRuleEngine&) = delete;

    // 新：规则实例列表（每个 t_linkage_rule 对应一个）
    std::vector<std::unique_ptr<RuleInstance>> ruleInstances_;
    std::vector<RuleInstance*> activeRules_; // 启用的规则指针

    // 内存规则映射：event_name -> [action_name1, action_name2, ...]
    std::unordered_map<std::string, std::vector<std::string>> ruleMap_;

    // 所有触发器和动作（从 DB 加载）
    std::vector<std::unique_ptr<CTrigger>> triggerStorage_;
    std::vector<std::unique_ptr<CLinkAction>> actionStorage_;

    // 提供给外部的指针视图（避免暴露 unique_ptr）
    std::vector<CTrigger*> triggers_;
    std::vector<CLinkAction*> actions_;

    mutable std::mutex mtx_;

    // 新增：trigger_id → 所属规则列表（加速匹配）
    std::unordered_map<int, std::vector<RuleInstance*>> triggerToRules_;

private:
    void loadActionsFromDB(Poco::Data::Session& session);
    void loadTriggersFromDB(Poco::Data::Session& session);
};

#define RULE_ENGINE CRuleEngine::GetInstance()