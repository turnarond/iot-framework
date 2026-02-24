/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 11:00:52
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-04 19:48:25
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/main.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "service/alarm_server.h"
#include "api/alarm_node_subscriber.h"
#include "api/alarm_server_controller.h"
#include "service/alarm_log.h"

#include "platform_sdk/init.h"
#include "lwlog/lwlog.h"

CLWLog g_lwlog;
int main(int argc, char *argv[])
{
    g_lwlog.SetLogFileName();
    vsoa_sdk::init();
    int ret = 0;

    ret = ALARM_SERVER_CONTROLLER->InitAlarmServer("alarm_server", 14000);
    if (ret != 0) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Init Alarm Server failed");
        return -1;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Init Alarm Server success");

    ret = ALARM_SERVER->InitAlarmServerQueue();
    if (ret != 0) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Init Alarm Server queue failed");
        return -1;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Init Alarm Server success");

    ret = ALARM_NODE_SUBSCRIBER->InitAlarmNodeSubscriber();
    if (ret != 0) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Init Alarm Node Subscriber failed");
        return -1;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Init Alarm Node Subscriber success");

    // 初始化报警日志保存器
    ret = ALARM_LOG_SAVER->Init();
    if (ret != 0) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Init Alarm Log Saver failed");
        return -1;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Init Alarm Log Saver success");

    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Alarm server started successfully");

    while(1);
    g_lwlog.LogMessage(LW_LOGLEVEL_WARN, "Alarm server exit");
}