/*
 * @Author: yanchaodong
 * @Date: 2026-02-22
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_state_manager.h
 * @Description: 报警状态管理器
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "alarm_server.h"

class AlarmStateManager {
public:
    // 处理报警状态并返回是否需要通知
    bool processAlarmState(const std::string& point_id, const std::string& point_name, 
                          const std::string& point_value, int alarm_rule_id, 
                          const AlarmRule& alarm_rule, AlarmServer* server, 
                          AlarmMessage::Type& msg_type);
    
    // 处理报警恢复
    void processAlarmClear(const std::string& point_id, const std::string& point_name, 
                          const std::string& point_value, const std::vector<AlarmRule>& rule_config, 
                          AlarmServer* server);
    
    // 发布报警通知
    void publishAlarmNotification(const std::string& point_id, const std::string& point_name, 
                                 const std::string& point_value, const AlarmRule& alarm_rule, 
                                 AlarmMessage::Type msg_type);
};
