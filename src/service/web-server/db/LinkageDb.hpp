/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/db/LinkageDb.hpp
 * @Description: Linkage Database Client
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef LINKAGE_DB_HPP
#define LINKAGE_DB_HPP

#include "dto/LinkageDto.hpp"
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * LinkageDb client definitions.
 */
class LinkageDb : public vsoa::orm::DbClient {
public:

  LinkageDb(const std::shared_ptr<vsoa::orm::Executor>& executor)
    : vsoa::orm::DbClient(executor)
  {

//     vsoa::orm::SchemaMigration migration(executor);
//     std::string database_migrations = LWComm::GetDataPath();
//     migration.addFile(1 /* start from version 1 */, database_migrations + LW_OS_DIR_SEPARATOR + "001_init.sql");
//     // TODO - Add more migrations here.
//     migration.migrate(); // <-- run migrations. This guy will throw on error.

//     auto version = executor->getSchemaVersion();
//     VSOA_LOGD("LinkageDb", "Migration - OK. Version=%ld.", version);

  }

  // Event type related queries
  QUERY(getEventTypes,
        "SELECT * FROM t_event_type "
        "ORDER BY id;")

  QUERY(getEventTypesWithPagination,
        "SELECT * FROM t_event_type "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getEventTypeCount,
        "SELECT COUNT(*) as count FROM t_event_type;")

  // Expression related queries
  QUERY(getExpressions,
        "SELECT * FROM t_expressions "
        "ORDER BY id;")

  QUERY(getExpressionsWithPagination,
        "SELECT * FROM t_expressions "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getExpressionById,
        "SELECT * FROM t_expressions "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createExpression,
        "INSERT INTO t_expressions "
        "(name, expression, enable, description) VALUES "
        "(:expr.name, :expr.expression, :expr.enable, :expr.description);",
        PARAM(vsoa::Object<ExpressionCreateDto>, expr))

  QUERY(updateExpression,
        "UPDATE t_expressions SET "
        "name = :expr.name, "
        "expression = :expr.expression, "
        "enable = :expr.enable, "
        "description = :expr.description "
        "WHERE id = :expr.id;",
        PARAM(vsoa::Object<ExpressionDto>, expr))

  QUERY(deleteExpression,
        "DELETE FROM t_expressions "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getExpressionCount,
        "SELECT COUNT(*) as count FROM t_expressions;")

  // Trigger source related queries
  QUERY(getTriggerSources,
        "SELECT * FROM t_linkage_trigger "
        "ORDER BY id;")

  QUERY(getTriggerSourcesWithPagination,
        "SELECT * FROM t_linkage_trigger "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getTriggerSourceById,
        "SELECT * FROM t_linkage_trigger "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createTriggerSource,
        "INSERT INTO t_linkage_trigger "
        "(name, event_type_id, alarm_rule_id, expr_id, custom_event_key, enable, description) VALUES "
        "(:trigger.name, :trigger.event_type_id, :trigger.alarm_rule_id, :trigger.expr_id, :trigger.custom_event_key, :trigger.enable, :trigger.description);",
        PARAM(vsoa::Object<TriggerSourceCreateDto>, trigger))

  QUERY(updateTriggerSource,
        "UPDATE t_linkage_trigger SET "
        "name = :trigger.name, "
        "event_type_id = :trigger.event_type_id, "
        "alarm_rule_id = :trigger.alarm_rule_id, "
        "expr_id = :trigger.expr_id, "
        "custom_event_key = :trigger.custom_event_key, "
        "enable = :trigger.enable, "
        "description = :trigger.description "
        "WHERE id = :trigger.id;",
        PARAM(vsoa::Object<TriggerSourceDto>, trigger))

  QUERY(deleteTriggerSource,
        "DELETE FROM t_linkage_trigger "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getTriggerSourceCount,
        "SELECT COUNT(*) as count FROM t_linkage_trigger;")

      // Action type related queries
  QUERY(getActionTypes,
        "SELECT * FROM t_action_type "
        "ORDER BY id;")

  QUERY(getActionTypesWithPagination,
        "SELECT * FROM t_action_type "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getActionTypeCount,
        "SELECT COUNT(*) as count FROM t_action_type;")

  // Action instance related queries (linkage actions)
  QUERY(getActionInstances,
        "SELECT * FROM t_linkage_action "
        "ORDER BY id;")

  QUERY(getActionInstancesWithPagination,
        "SELECT * FROM t_linkage_action "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getActionInstanceById,
        "SELECT * FROM t_linkage_action "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createActionInstance,
        "INSERT INTO t_linkage_action "
        "(name, action_type_id, param1, param2, param3, param4, need_confirm) VALUES "
        "(:action.name, :action.action_type_id, :action.param1, :action.param2, :action.param3, :action.param4, :action.need_confirm);",
        PARAM(vsoa::Object<ActionInstanceCreateDto>, action))

  QUERY(updateActionInstance,
        "UPDATE t_linkage_action SET "
        "name = :action.name, "
        "action_type_id = :action.action_type_id, "
        "param1 = :action.param1, "
        "param2 = :action.param2, "
        "param3 = :action.param3, "
        "param4 = :action.param4, "
        "need_confirm = :action.need_confirm "
        "WHERE id = :action.id;",
        PARAM(vsoa::Object<ActionInstanceDto>, action))

  QUERY(deleteActionInstance,
        "DELETE FROM t_linkage_action "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getActionInstanceCount,
        "SELECT COUNT(*) as count FROM t_linkage_action;")

  // Linkage rule related queries
  QUERY(getLinkageRules,
        "SELECT * FROM t_linkage_rule "
        "ORDER BY id;")

  QUERY(getLinkageRulesWithPagination,
        "SELECT * FROM t_linkage_rule "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getLinkageRulesWithDetails,
        "SELECT lr.id, lr.name, lr.logic_type, lr.enable, lr.description, "
        "GROUP_CONCAT(la.name, ';') as action_names, GROUP_CONCAT(la.id, ',') as action_ids "
        "FROM t_linkage_rule lr "
        "LEFT JOIN t_linkage_rule_action rra ON lr.id = rra.rule_id "
        "LEFT JOIN t_linkage_action la ON rra.action_id = la.id "
        "GROUP BY lr.id "
        "ORDER BY lr.id;")

  QUERY(getLinkageRulesWithDetailsWithPagination,
        "SELECT lr.id, lr.name, lr.logic_type, lr.enable, lr.description, "
        "GROUP_CONCAT(la.name, ';') as action_names, GROUP_CONCAT(la.id, ',') as action_ids "
        "FROM t_linkage_rule lr "
        "LEFT JOIN t_linkage_rule_action rra ON lr.id = rra.rule_id "
        "LEFT JOIN t_linkage_action la ON rra.action_id = la.id "
        "GROUP BY lr.id "
        "ORDER BY lr.id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getLinkageRuleById,
        "SELECT * FROM t_linkage_rule "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createLinkageRule,
        "INSERT INTO t_linkage_rule "
        "(name, logic_type, enable, description) VALUES "
        "(:rule.name, :rule.logic_type, :rule.enable, :rule.description);",
        PARAM(vsoa::Object<LinkageRuleCreateDto>, rule))

  QUERY(updateLinkageRule,
        "UPDATE t_linkage_rule SET "
        "name = :rule.name, "
        "logic_type = :rule.logic_type, "
        "enable = :rule.enable, "
        "description = :rule.description "
        "WHERE id = :rule.id;",
        PARAM(vsoa::Object<LinkageRuleDto>, rule))

  QUERY(deleteLinkageRule,
        "DELETE FROM t_linkage_rule "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getLinkageRuleCount,
        "SELECT COUNT(*) as count FROM t_linkage_rule;")

  // Rule trigger related queries
  QUERY(addTriggerToRule,
        "INSERT INTO t_linkage_rule_trigger "
        "(rule_id, trigger_id) VALUES "
        "(:rule_id, :trigger_id);",
        PARAM(vsoa::UInt32, rule_id),
        PARAM(vsoa::UInt32, trigger_id))

  QUERY(addActionToRule,
        "INSERT INTO t_linkage_rule_action "
        "(rule_id, action_id) VALUES "
        "(:rule_id, :action_id);",
        PARAM(vsoa::UInt32, rule_id),
        PARAM(vsoa::UInt32, action_id))

  QUERY(removeTriggerFromRule,
        "DELETE FROM t_linkage_rule_trigger "
        "WHERE rule_id = :rule_id AND trigger_id = :trigger_id;",
        PARAM(vsoa::UInt32, rule_id),
        PARAM(vsoa::UInt32, trigger_id))

  QUERY(getTriggersByRuleId,
        "SELECT trigger_id FROM t_linkage_rule_trigger "
        "WHERE rule_id = :rule_id;",
        PARAM(vsoa::UInt32, rule_id))

  QUERY(getTriggersByAlarmRuleId,
        "SELECT * FROM t_linkage_trigger "
        "WHERE alarm_rule_id = :alarm_rule_id "
        "ORDER BY id;",
        PARAM(vsoa::UInt32, alarm_rule_id))

  QUERY(getActionsByRuleId,
        "SELECT action_id FROM t_linkage_rule_action "
        "WHERE rule_id = :rule_id;",
        PARAM(vsoa::UInt32, rule_id))

  // Linkage log related queries
  QUERY(getLinkageLogs,
        "SELECT * FROM t_linkage_log "
        "ORDER BY id DESC "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, limit),
        PARAM(vsoa::UInt32, offset))

  QUERY(getLinkageLogCount,
        "SELECT COUNT(*) as count FROM t_linkage_log;")

};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen

#endif /* LINKAGE_DB_HPP */
