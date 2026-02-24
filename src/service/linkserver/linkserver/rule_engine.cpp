/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:23:13
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-03 09:49:13
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/rule_engine.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "rule_engine.h"
#include "linkact_manager.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Session.h"
#include "Poco/Exception.h"
#include <sstream>

#include "lwlog/lwlog.h"

using namespace Poco::Data;
using namespace Poco::Data::Keywords;

extern CLWLog g_lwlog;

namespace {
    // 辅助函数：拼接参数为 JSON 字符串（简化版）
    std::string paramsToJson(const std::string& p1, const std::string& p2,
                             const std::string& p3, const std::string& p4) {
        std::ostringstream oss;
        oss << "{";
        if (!p1.empty()) oss << "\"param1\":\"" << p1 << "\"";
        if (!p2.empty()) oss << (oss.str().size() > 1 ? "," : "") << "\"param2\":\"" << p2 << "\"";
        if (!p3.empty()) oss << (oss.str().size() > 1 ? "," : "") << "\"param3\":\"" << p3 << "\"";
        if (!p4.empty()) oss << (oss.str().size() > 1 ? "," : "") << "\"param4\":\"" << p4 << "\"";
        oss << "}";
        return oss.str();
    }
}
// rule_engine.cpp
int CRuleEngine::loadRules(const std::string& dbPath) 
{
    try {
        Poco::Data::SQLite::Connector::registerConnector();
        Session session("SQLite", dbPath);

        // 清空旧数据
        triggerStorage_.clear();
        actionStorage_.clear();
        ruleInstances_.clear();
        triggerToRules_.clear();
        activeRules_.clear();

        // Step 1: 加载所有启用的联动规则 + 触发器绑定
        int rule_id, action_id, trigger_id;
        std::string rule_name, logic_type;
        std::map<int, RuleInstance> tempRules;
    
        Statement select(session);
        select << "SELECT r.id, r.name, r.action_id, r.logic_type, rt.trigger_id "
               << "FROM t_linkage_rule r "
               << "JOIN t_linkage_rule_trigger rt ON r.id = rt.rule_id "
               << "WHERE r.enabled = 1 "
               << "ORDER BY r.id, rt.trigger_id";

        select.execute();
        RecordSet rs(select);
        bool more = rs.moveFirst();

        while (more) {
            rule_id = rs["id"].convert<int>();
            rule_name = rs["name"].convert<std::string>();
            action_id = rs["action_id"].convert<int>();
            logic_type = rs["logic_type"].convert<std::string>();
            trigger_id = rs["trigger_id"].convert<int>();

            if (tempRules.find(rule_id) == tempRules.end()) {
                tempRules[rule_id] = RuleInstance{
                    rule_id, rule_name, action_id, logic_type, true, {}, {}
                };
            }
            tempRules[rule_id].required_trigger_ids.insert(trigger_id);
            more = rs.moveNext();
        }

        // 转存到 unique_ptr
        for (auto& kv : tempRules) {
            ruleInstances_.push_back(std::make_unique<RuleInstance>(kv.second));
            activeRules_.push_back(ruleInstances_.back().get());
            for (int tid : kv.second.required_trigger_ids) {
                triggerToRules_[tid].push_back(ruleInstances_.back().get());
            }
        }

        // Step 2: 加载触发器 (t_linkage_trigger)
        loadTriggersFromDB(session);

        // Step 3: 加载动作实例 (t_action_instance + t_action_type)
        loadActionsFromDB(session);

        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Loaded %zu rules, %zu triggers, %zu actions",
                     ruleInstances_.size(), triggerStorage_.size(), actionStorage_.size());

        return 0;

    } catch (Poco::Exception& ex) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load rules from DB: %s", ex.displayText().c_str());
        return -1;
    }
}

/**
 * @brief OnEvent, 收到触发器发过来的事件
 * @param trigger_id
 * @param payload
 */
void CRuleEngine::OnEvent(int trigger_id, const std::string& payload) 
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = triggerToRules_.find(trigger_id);
    if (it == triggerToRules_.end()) {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "No rule bound to trigger_id=%d", trigger_id);
        return;
    }

    for (RuleInstance* rule : it->second) {
        if (!rule->enabled) continue;

        rule->matched_triggers.insert(trigger_id);

        if (rule->isSatisfied()) {
            // 查找对应动作
            auto actionIt = std::find_if(
                actionStorage_.begin(), 
                actionStorage_.end(),
                [rule](const auto& a) { return a->aciont_id_ == rule->action_id; }
            );

            if (actionIt != actionStorage_.end()) { // 找到动作,确定是否是联合动作?
                bool success = LINKACT_MANAGER->executeAction((*actionIt)->name_, payload);
                g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Executed action '%s' for rule '%s' (trigger_id=%d)",
                    (*actionIt)->name_.c_str(), rule->rule_name.c_str(), trigger_id);

                // 记录日志（可选）
                // logLinkage(rule->rule_id, rule->rule_name, trigger_id, payload, success);
            }

            // AND 规则执行后重置状态（防重复触发）
            if (rule->logic_type == "AND") {
                rule->reset();
            }
        }
    }
}

void CRuleEngine::loadTriggersFromDB(Session& session) 
{
    Statement select(session);
    select << "SELECT "
           << "ts.id, "
           << "ts.name, "
           << "ts.eventtype_name, "
           << "ts.description, "
           << "et.producer AS source, "          // ← 来自 t_trigger_type.producer
           << "ts.param1, ts.param2, ts.param3, ts.param4 "
           << "FROM t_linkage_trigger ts "
           << "JOIN t_trigger_type et ON ts.eventtype_name = et.name "
           << "WHERE ts.enable = 1";

    select.execute();
    RecordSet rs(select);
    bool more = rs.moveFirst();
    while (more) {
        auto trigger = std::make_unique<CTrigger>();
        trigger->trigger_id_ = rs["id"].convert<int>();
        trigger->trigger_name_ = rs["name"].convert<std::string>();
        trigger->eventtype_name_ = rs["eventtype_name"].convert<std::string>();
        trigger->source_ = rs["source"].convert<std::string>();

        if (!rs.isNull("description"))
            trigger->trigger_desc_ = rs["description"].convert<std::string>();
        if (!rs.isNull("param1"))
            trigger->param1_ = rs["param1"].convert<std::string>();
        if (!rs.isNull("param2"))
            trigger->param2_ = rs["param2"].convert<std::string>();
        if (!rs.isNull("param3"))
            trigger->param3_ = rs["param3"].convert<std::string>();
        if (!rs.isNull("param4"))
            trigger->param4_ = rs["param4"].convert<std::string>();
        
        if (trigger->source_.find("lib") == std::string::npos || trigger->source_.find("lib") != 0)
            trigger->source_ = "lib" + trigger->source_;
        if (trigger->source_.find(".so") == std::string::npos) {
            trigger->source_ += ".so";
        }

        triggers_.push_back(trigger.get());
        triggerStorage_.push_back(std::move(trigger));
        more = rs.moveNext();
    }
}

void CRuleEngine::loadActionsFromDB(Session& session) 
{
    // JOIN t_action_instance + t_action_type to get libname
    Statement select(session);
    select << "SELECT ai.id, ai.name, ai.param1, ai.param2, ai.param3, ai.param4, at.libname, at.cname "
           << "FROM t_linkage_action ai "
           << "JOIN t_action_type at ON ai.actiontype_id = at.id";

    select.execute();
    RecordSet rs(select);
    bool more = rs.moveFirst();
    while (more) {
        auto action = std::make_unique<CLinkAction>();
        action->aciont_id_ = rs["id"].convert<int>(); 
        action->name_ = rs["name"].convert<std::string>();
        
        if (!rs.isNull("cname"))
            action->desc_ = rs["cname"].convert<std::string>();

        if (!rs.isNull("param1"))
            action->param1_ = rs["param1"].convert<std::string>();
        if (!rs.isNull("param2"))
            action->param2_ = rs["param2"].convert<std::string>();
        if (!rs.isNull("param3"))
            action->param3_ = rs["param3"].convert<std::string>();
        if (!rs.isNull("param4"))
            action->param4_ = rs["param4"].convert<std::string>();
        action->plugin_name_ = rs["libname"].convert<std::string>();

        if (action->plugin_name_.find("lib") == std::string::npos || action->plugin_name_.find("lib") != 0)
            action->plugin_name_ = "lib" + action->plugin_name_;
        if (action->plugin_name_.find(".so") == std::string::npos) {
            action->plugin_name_ += ".so";
        }

        actions_.push_back(action.get());
        actionStorage_.push_back(std::move(action));
        more = rs.moveNext();
    }
}
