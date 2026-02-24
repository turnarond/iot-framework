/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 11:00:40
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-06 11:22:58
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_server.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_server.h"
#include "Poco/Thread.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Exception.h"
#include <algorithm>
#include <exception>
#include <string>

#include "lwcomm/lwcomm.h"
#include "lwlog/lwlog.h"
#include "../api/alarm_server_controller.h"
#include "vsoa_dto/core/Types.hpp"
#include "alarm_rule_processor.h"
#include "alarm_state_manager.h"

extern CLWLog g_lwlog;
using namespace Poco::Data;
using namespace Poco::Data::Keywords;

void AlarmServer::run()
{
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmServer::run() - starting alarm processing thread");
    
    AlarmStateManager stateManager;
    
    while (true) 
    {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::run() - waiting for notification");
        AlarmPointMessage* pMsg = dynamic_cast<AlarmPointMessage*>(queue_.waitDequeueNotification());
        if (!pMsg) {
            g_lwlog.LogMessage(LW_LOGLEVEL_WARN, "AlarmServer::run() - queue_.waitDequeueNotification() returned nullptr");
            continue;
        }

        std::string point_id = pMsg->point_id();
        std::string point_value = pMsg->point_value();
    
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server received tag %s with value %s", 
            point_id.c_str(), point_value.c_str());

        // 处理报警信息，判断是否触发报警
        auto alarm_config = point_alarm_configs_.find(point_id);
        if (alarm_config == point_alarm_configs_.end()) {
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server can not find alarm config for tag %s", 
                point_id.c_str());
            continue;
        }
        
        if (alarm_config->second.empty()) {
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server can not find alarm config for tag %s", 
                point_id.c_str());
            continue;
        }
        std::string point_name = alarm_config->second[0].point_name;

        const auto& rule_config = alarm_config->second;

        // 检查所有报警
        bool should_alarm = false;
        int alarm_rule_id = -1;
        AlarmRule alarm_rule;
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server checking %zu rules for tag %s", 
            rule_config.size(), point_id.c_str());
        
        for (size_t i = 0; i < rule_config.size(); i++) // 已经按照优先级进行排序，会优先定值报警；
        {
            const auto& rule = rule_config[i];
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server checking rule %d for tag %s: method=%d, threshold=%f", 
                rule.id, point_id.c_str(), rule.method, rule.threshold);
            
            // 使用工厂创建对应类型的规则处理器
            auto processor = AlarmRuleProcessorFactory::createProcessor(rule.method);
            if (processor) {
                if (processor->processRule(rule, point_value, point_id, this)) {
                    should_alarm = true;
                    alarm_rule_id = rule.id;
                    alarm_rule = rule;
                    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server found triggering rule %d for tag %s, breaking loop", 
                        alarm_rule_id, point_id.c_str());
                    break;
                }
            } else {
                g_lwlog.LogMessage(LW_LOGLEVEL_WARN, "Alarm server unknown method %d for rule %d", 
                    rule.method, rule.id);
            }
        }
        
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "current tag %s:%s should alarm: %s",
            point_id.c_str(), point_name.c_str(), should_alarm ? "true" : "false");

        // 处理报警状态和通知
        if (should_alarm) { // 当前点位需要报警
            AlarmMessage::Type msg_type;
            bool should_notify = stateManager.processAlarmState(point_id, point_name, point_value, 
                                                             alarm_rule_id, alarm_rule, this, msg_type);
            
            if (should_notify) {
                stateManager.publishAlarmNotification(point_id, point_name, point_value, alarm_rule, msg_type);
            }
        } else {
            // 当前点位无任何报警，检查并发送恢复报警
            stateManager.processAlarmClear(point_id, point_name, point_value, rule_config, this);
        }
    }
}

int AlarmServer::InitAlarmServerQueue()
{
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmServer::InitAlarmServerQueue() - starting initialization");
    // 获取报警点位配置信息
    std::string db_path = LWComm::GetDataPath();
    db_path = db_path + LW_OS_DIR_SEPARATOR + db_file_;
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmServer::InitAlarmServerQueue() - using database: %s", 
        db_path.c_str());
    
    try {
        // 注册 SQLite 连接器（只需一次）
        static bool initialized = false;
        if (!initialized) {
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - registering SQLite connector");
            Poco::Data::SQLite::Connector::registerConnector();
            initialized = true;
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - SQLite connector registered");
        }
        
        // 创建数据库会话
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - creating database session");
        Session session("SQLite", db_path);
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - database session created");

        Statement select(session);
        // SQL 查询：获取点位名及其所有报警规则
        std::string sql = R"(
            SELECT 
                p.name AS point_name,
                p.id AS point_id,
                ar.id AS rule_id,
                ar.name AS alarm_name,
                ar.method,
                ar.threshold,
                ar.param1,
                ar.param2,
                ar.param3,
                ar.enable
            FROM t_points p
            INNER JOIN t_alarm_rules ar ON p.id = ar.point_id
            WHERE ar.enable = 1
            ORDER BY p.name, ar.method, ar.id
        )";
        
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - executing SQL query");
        select << sql;
        select.execute();
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - SQL query executed");

        RecordSet rs(select);
        bool more = rs.moveFirst();

        // 清空旧配置
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - clearing old configurations");
        point_alarm_configs_.clear();
        alarm_states_.clear();
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - old configurations cleared");

        int rule_count = 0;
        while (more) {
            std::string point_name = rs["point_name"];
            std::string point_id = std::to_string(rs["point_id"].convert<int>());
            int rule_id = rs["rule_id"].convert<int>();
            std::string alarm_name = rs["alarm_name"];
            int method = rs["method"].convert<int>();
            float threshold = static_cast<float>(rs["threshold"].convert<double>());
            float param1 = rs["param1"].isEmpty() ? 0.0f : static_cast<float>(rs["param1"].convert<double>());
            float param2 = rs["param2"].isEmpty() ? 0.0f : static_cast<float>(rs["param2"].convert<double>());
            float param3 = rs["param3"].isEmpty() ? 0.0f : static_cast<float>(rs["param3"].convert<double>());
            bool enabled = rs["enable"].convert<bool>();

            // 跳过禁用规则（双重保险）
            if (!enabled) {
                g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - skipping disabled rule %d", rule_id);
                more = rs.moveNext();
                continue;
            }

            // 添加规则
            AlarmRule rule;
            rule.id = rule_id;
            rule.name = alarm_name;
            rule.method = method;
            rule.threshold = threshold;
            rule.param1 = param1;
            rule.param2 = param2;
            rule.param3 = param3;
            rule.enabled = true;
            rule.point_id = point_id;
            rule.point_name = point_name;

            point_alarm_configs_[point_id].push_back(rule);
            rule_count++;
            
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - added rule %d for point %s:%s, method=%d, threshold=%f", 
                rule_id, point_id.c_str(), point_name.c_str(), method, threshold);

            more = rs.moveNext();
        }

        // 对每个点位的规则按业务优先级排序
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - sorting rules by priority");
        for (auto& pair : point_alarm_configs_) {
            std::sort(pair.second.begin(), pair.second.end());
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - sorted %zu rules for point %s", 
                pair.second.size(), pair.first.c_str());
        }

        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "[AlarmServer] Loaded %lu points with %d alarm rules.", 
            point_alarm_configs_.size(), rule_count);

    } catch (Poco::Exception& ex) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "[AlarmServer] Init failed: %s", 
            ex.displayText().c_str());
        return -1;
    } catch (std::exception& ex) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "[AlarmServer] Init failed: %s", ex.what());
        return -1;
    }

    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServer::InitAlarmServerQueue() - starting message queue thread");
    msg_queue_thread_.setName("AlarmServerThread");
    msg_queue_thread_.start(this);
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmServer::InitAlarmServerQueue() - initialization completed successfully");

    return 0;
}

int AlarmServer::PushAlarmMsg(std::string tag_id, std::string tag_value)
{
    if (queue_.size() > queue_size_) {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm server queue is full, drop message");
        return -1;
    }
    queue_.enqueueNotification(new AlarmPointMessage(tag_id, tag_value));
    return 0;
} 
