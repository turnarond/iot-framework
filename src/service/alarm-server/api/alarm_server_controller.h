/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 10:56:54
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-04 16:54:53
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/api/alarm_server_controller.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "platform_sdk/server.h"
#include "../dto/alarm_node_dto.hpp"

class AlarmServerController {
public:
    int InitAlarmServer(std::string node_name, unsigned short port);
    void UnInitAlarmServer();

    int PublishAlarm(vsoa::Object<AlarmMessageDto> alarm_message);

    void OnConnectCb(vsoa_sdk::server::ServerHandle *server, uint32_t cid, bool connected, void *arg);
private:
    void InitializeListener ();
private:
    unsigned short server_port_ = 13000;
    std::string server_name_ = "alarm_server";

    std::string url_alarm_publish_ = "/v1/alarm_server/alarm_info/";

    std::shared_ptr<vsoa_sdk::server::ServerHandle> sh_ = nullptr;

    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr;
private:
    static AlarmServerController *s_pInstance_;
public:
    static AlarmServerController* GetInstance();
private:
    AlarmServerController();
    ~AlarmServerController();
};

#define ALARM_SERVER_CONTROLLER AlarmServerController::GetInstance()