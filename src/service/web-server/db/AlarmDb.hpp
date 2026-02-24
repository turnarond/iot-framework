/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/db/AlarmDb.hpp
 * @Description: Alarm Database Client
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef ALARM_DB_HPP
#define ALARM_DB_HPP

#include "dto/AlarmDto.hpp"
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * AlarmDb client definitions.
 */
class AlarmDb : public vsoa::orm::DbClient {
public:

  AlarmDb(const std::shared_ptr<vsoa::orm::Executor>& executor)
    : vsoa::orm::DbClient(executor)
  {

//     vsoa::orm::SchemaMigration migration(executor);
//     std::string database_migrations = LWComm::GetDataPath();
//     migration.addFile(1 /* start from version 1 */, database_migrations + LW_OS_DIR_SEPARATOR + "lw_monitor.sql");
//     // TODO - Add more migrations here.
//     migration.migrate(); // <-- run migrations. This guy will throw on error.

//     auto version = executor->getSchemaVersion();
//     VSOA_LOGD("AlarmDb", "Migration - OK. Version=%ld.", version);

  }

  // Alarm rule related queries
  QUERY(getAlarmRules,
        "SELECT * FROM t_alarm_rules "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getAlarmRulesWithPagination,
        "SELECT * FROM t_alarm_rules "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getAlarmRulesWithPoint,
        "SELECT ar.*, p.name as point_name, d.name as device_name "
        "FROM t_alarm_rules ar "
        "JOIN t_points p ON ar.point_id = p.id "
        "JOIN t_devices d ON p.device_id = d.id "
        "ORDER BY ar.id;")

  QUERY(getAlarmRulesWithPointWithPagination,
        "SELECT ar.*, p.name as point_name, d.name as device_name "
        "FROM t_alarm_rules ar "
        "JOIN t_points p ON ar.point_id = p.id "
        "JOIN t_devices d ON p.device_id = d.id "
        "ORDER BY ar.id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getAlarmRuleById,
        "SELECT * FROM t_alarm_rules "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createAlarmRuleWithPoint,
        "INSERT INTO t_alarm_rules "
        "(point_id, name, method, threshold, restore_threshold, hysteresis, enable, enable_restore) VALUES "
        "(:rule.point_id, :rule.name, :rule.method, :rule.threshold, :rule.restore_threshold, :rule.hysteresis, :rule.enable, :rule.enable_restore);",
        PARAM(vsoa::Object<AlarmRuleWithPointCreateDto>, rule))

  QUERY(getAlarmRuleWithPointById,
        "SELECT ar.*, p.name as point_name, d.name as device_name "
        "FROM t_alarm_rules ar "
        "JOIN t_points p ON ar.point_id = p.id "
        "JOIN t_devices d ON p.device_id = d.id "
        "WHERE ar.id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createAlarmRule,
        "INSERT INTO t_alarm_rules "
        "(point_id, name, method, threshold, restore_threshold, hysteresis, enable, enable_restore) VALUES "
        "(:rule.point_id, :rule.name, :rule.method, :rule.threshold, :rule.restore_threshold, :rule.hysteresis, :rule.enable, :rule.enable_restore);",
        PARAM(vsoa::Object<AlarmRuleCreateDto>, rule))

  QUERY(updateAlarmRule,
        "UPDATE t_alarm_rules SET "
        "point_id = :rule.point_id, "
        "name = :rule.name, "
        "method = :rule.method, "
        "threshold = :rule.threshold, "
        "restore_threshold = :rule.restore_threshold, "
        "hysteresis = :rule.hysteresis, "
        "enable = :rule.enable, "
        "enable_restore = :rule.enable_restore "
        "WHERE id = :rule.id;",
        PARAM(vsoa::Object<AlarmRuleDto>, rule))

  QUERY(deleteAlarmRule,
        "DELETE FROM t_alarm_rules "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getAlarmRuleCount,
        "SELECT COUNT(*) as count FROM t_alarm_rules;")

  // Alarm related queries
  QUERY(getAlarms, 
        "SELECT * FROM t_alarm_log "
        "ORDER BY id DESC;")

  QUERY(getAlarmsWithPagination,
        "SELECT * FROM t_alarm_log "
        "ORDER BY id DESC "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getActiveAlarms,
        "SELECT * FROM t_alarm_log "
        "WHERE event_type = 1 AND ack_status = 0 "
        "ORDER BY id DESC;")

  QUERY(getActiveAlarmsWithPagination,
        "SELECT * FROM t_alarm_log "
        "WHERE event_type = 1 AND ack_status = 0 "
        "ORDER BY id DESC "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getAlarmRuleByPointId,
        "SELECT * FROM t_alarm_rules "
        "WHERE point_id = :point_id;",
        PARAM(vsoa::UInt32, point_id))

  QUERY(getAlarmsByPoint,
        "SELECT * FROM t_alarm_log "
        "WHERE point_id = :point_id "
        "ORDER BY id DESC;",
        PARAM(vsoa::UInt32, point_id))

  QUERY(getAlarmCount,
        "SELECT COUNT(*) as count FROM t_alarm_log;")

  QUERY(getActiveAlarmCount,
        "SELECT COUNT(*) as count FROM t_alarm_log WHERE event_type = 1 AND ack_status = 0;")

};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen

#endif /* ALARM_DB_HPP */
