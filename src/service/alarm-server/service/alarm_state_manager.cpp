/*
 * @Author: yanchaodong
 * @Date: 2026-02-22
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_state_manager.cpp
 * @Description: 报警状态管理器实现
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_state_manager.h"
#include "lwlog/lwlog.h"
#include "api/alarm_server_controller.h"
#include "vsoa_dto/core/Types.hpp"
#include "alarm_log.h"

extern CLWLog g_lwlog;

bool AlarmStateManager::processAlarmState(const std::string& point_id, const std::string& point_name, 
                                         const std::string& point_value, int alarm_rule_id, 
                                         const AlarmRule& alarm_rule, AlarmServer* server, 
                                         AlarmMessage::Type& msg_type) {
    bool should_notify = false;
    msg_type = AlarmMessage::Type::TRIGGER;
    
    auto& alarm_states_ = server->getAlarmStates();
    auto alarm_status = alarm_states_.find(point_id);
    
    if (alarm_status == alarm_states_.end()) { // 当前不存在，第一次需要报警
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: tag %s first alarm, need notify", point_name.c_str());
        long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
        alarm_states_[point_id] = 
            {
                std::make_pair(alarm_rule_id, 
                        AlarmServer::AlarmState(true, current_time))
            };
        // 对于持续时间规则，确保激活时间被正确设置
        alarm_states_[point_id][alarm_rule_id].activation_time = current_time;
        should_notify = true;
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: created new alarm state for tag %s, rule %d", 
            point_id.c_str(), alarm_rule_id);
    } else {
        // 当前点位存在过告警；检查是否有处于激活状态的；
        // 如果当前告警处于激活状态，则不再触发；
        // 如果别的告警处于激活状态，则恢复；
        if (alarm_status->second.find(alarm_rule_id) == alarm_status->second.end()) {
            // 当前点位存在告警；当前报警规则 id 不存在，说明存在过报警信息；
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: tag %s alarm rule %d first alarm, need notify",
                point_name.c_str(), alarm_rule_id);
            long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
            alarm_status->second[alarm_rule_id] = AlarmServer::AlarmState(true, current_time);
            // 对于持续时间规则，确保激活时间被正确设置
            alarm_status->second[alarm_rule_id].activation_time = current_time;
            should_notify = true;
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: added new alarm rule %d to existing state for tag %s", 
                alarm_rule_id, point_id.c_str());
        } else {
            // 检查是否处于触发状态
            auto &state = alarm_status->second[alarm_rule_id];
            if (!state.active) {
                g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: Alarm rule %d not active, notify", alarm_rule_id);
                should_notify = true;
                state.active = true;
                long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
                state.last_trigger_time = current_time; // ms
                state.activation_time = current_time; // 重置激活时间
                g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: reactivated alarm rule %d for tag %s", 
                    alarm_rule_id, point_id.c_str());
            } else {
                g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: Tag %s alarm_rule %d is already active",
                    point_id.c_str(), alarm_rule_id);
            }
        }
        
        // 恢复除当前alarm_rule_id之外的其他的告警
        // 遍历所有的规则，并检查是否处于激活状态
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: checking other rules to clear for tag %s", 
            point_id.c_str());
        
        // 获取该点位的所有规则
        auto& point_alarm_configs_ = server->getPointAlarmConfigs();
        auto config_it = point_alarm_configs_.find(point_id);
        if (config_it != point_alarm_configs_.end()) {
            const auto& rule_config = config_it->second;
            for (auto &rule : rule_config) {
                if (rule.id != alarm_rule_id) {
                    auto alarm = alarm_status->second.find(rule.id);
                    if (alarm != alarm_status->second.end() && alarm->second.active) {
                        // 恢复当前告警
                        alarm->second.active = false;
                        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: Tag %s: %s alarm_rule %d is active, need recovery",
                            point_id.c_str(), point_name.c_str(), alarm->first);
                        vsoa::Object<AlarmMessageDto> dto = vsoa::Object<AlarmMessageDto>::createShared();
                        dto->point_id = point_id;
                        dto->point_name = point_name;
                        dto->point_value = point_value;
                        dto->rule_id = rule.id;
                        dto->rule_name = rule.name;
                        dto->rule_method = rule.method;
                        dto->message_type = AlarmMessageType::ALARM_CLEAR;
                        ALARM_SERVER_CONTROLLER->PublishAlarm(dto);
                        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmState: cleared alarm rule %d for tag %s", 
                            rule.id, point_id.c_str());
                    }
                }
            }
        }
    }
    
    return should_notify;
}

void AlarmStateManager::processAlarmClear(const std::string& point_id, const std::string& point_name, 
                                         const std::string& point_value, const std::vector<AlarmRule>& rule_config, 
                                         AlarmServer* server) {
    // 当前点位无任何报警，检查并发送恢复报警
    auto& alarm_states_ = server->getAlarmStates();
    auto alarm_status = alarm_states_.find(point_id);
    if (alarm_status == alarm_states_.end()) { 
        // 当前未产生过报警，无需发送
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmClear: Tag %s: %s no alarm_rule is active",
           point_id.c_str(), point_name.c_str());
        return;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmClear: checking all rules to clear for tag %s", 
        point_id.c_str());
    for (auto &rule : rule_config) {
        auto alarm = alarm_status->second.find(rule.id);
        if (alarm != alarm_status->second.end() && alarm->second.active) {
                // 恢复当前告警
            alarm->second.active = false;
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmClear: Tag %s: %s alarm_rule %d is active, need recovery",
                point_id.c_str(), point_name.c_str(), alarm->first);
            vsoa::Object<AlarmMessageDto> dto = vsoa::Object<AlarmMessageDto>::createShared();
            dto->point_id = point_id;
            dto->point_name = point_name;
            dto->point_value = point_value;
            dto->rule_id = rule.id;
            dto->rule_name = rule.name;
            dto->rule_method = rule.method;
            dto->message_type = AlarmMessageType::ALARM_CLEAR;
            ALARM_SERVER_CONTROLLER->PublishAlarm(dto);
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::processAlarmClear: cleared alarm rule %d for tag %s", 
                rule.id, point_id.c_str());
        }
    }
}

void AlarmStateManager::publishAlarmNotification(const std::string& point_id, const std::string& point_name, 
                                                const std::string& point_value, const AlarmRule& alarm_rule, 
                                                AlarmMessage::Type msg_type) {
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::publishAlarmNotification: preparing to publish alarm for tag %s", 
        point_id.c_str());
    vsoa::Object<AlarmMessageDto> dto = vsoa::Object<AlarmMessageDto>::createShared();
    dto->point_id = point_id;
    dto->point_name = alarm_rule.point_name;
    dto->point_value = point_value;
    dto->rule_id = alarm_rule.id;
    dto->rule_name = alarm_rule.name;
    dto->rule_method = alarm_rule.method;
    
    int alarm_type = 0;
    if (msg_type == AlarmMessage::Type::CLEAR) {
        dto->message_type = AlarmMessageType::ALARM_CLEAR;
        alarm_type = static_cast<int>(AlarmLogType::CLEAR);
        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmStateManager::publishAlarmNotification: Tag %s: %s, clearing alarm rule %d:%s", 
            alarm_rule.point_id.c_str(), alarm_rule.point_name.c_str(), alarm_rule.method, alarm_rule.name.c_str());
    } else if (msg_type == AlarmMessage::Type::TRIGGER) {
        dto->message_type = AlarmMessageType::ALARM_TRIGGER;
        alarm_type = static_cast<int>(AlarmLogType::TRIGGER);
        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmStateManager::publishAlarmNotification: Tag %s: %s, triggering alarm rule %d:%s", 
            alarm_rule.point_id.c_str(), alarm_rule.point_name.c_str(), alarm_rule.method, alarm_rule.name.c_str());
    }

    ALARM_SERVER_CONTROLLER->PublishAlarm(dto);
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::publishAlarmNotification: published alarm for tag %s", 
        point_id.c_str());
    
    // 记录报警日志到数据库
    ALARM_LOG_SAVER->AddAlarmLog(
        point_id, 
        alarm_rule.point_name, 
        point_value, 
        alarm_rule.id, 
        alarm_rule.name, 
        alarm_rule.method, 
        alarm_type
    );
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmStateManager::publishAlarmNotification: logged alarm for tag %s to database", 
        point_id.c_str());
}
