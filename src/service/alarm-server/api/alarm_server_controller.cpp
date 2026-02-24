/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 11:17:22
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-04 17:45:05
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/api/alarm_server_controller.cpp
 * @Description: 对外发布报警信息的接口实现
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_server_controller.h"
#include "lwlog/lwlog.h"

extern CLWLog g_lwlog;

AlarmServerController* AlarmServerController::s_pInstance_ = nullptr;

AlarmServerController* AlarmServerController::GetInstance()
{
    if (s_pInstance_ == nullptr)
    {
        s_pInstance_ = new AlarmServerController();
    }
    return s_pInstance_;
}

void AlarmServerController::OnConnectCb(vsoa_sdk::server::ServerHandle *server, uint32_t cid, bool connected, void *arg)
{
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServerController::OnConnectCb cid %d, connected %d", 
        cid, connected);
}

int AlarmServerController::InitAlarmServer(std::string node_name, unsigned short port)
{
    if (node_name.size() > 0) server_name_ = node_name;
    server_port_ = port;

    sh_ = std::make_shared<vsoa_sdk::server::ServerHandle>(server_name_);

    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;

    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);

    sh_->SetServerPort(server_port_);

    sh_->SetOnConnectCb([](vsoa_sdk::server::ServerHandle *server, uint32_t cid, bool connected, void *arg) {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmServerController::OnConnectCb cid %d, connected %d", 
                cid, connected);
    }, this);

    int ret = sh_->CreateServer();
    if (ret != 0) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmServerController::InitAlarmServer CreateServer failed, ret %d",
            ret);
        return -1;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmServerController::InitAlarmServer CreateServer success");

    sh_->SrvSpinAsync(1000);
    return 0;
}

int AlarmServerController::PublishAlarm(vsoa::Object<AlarmMessageDto> alarm_message)
{
    vsoa::String str;
    try {
        str = obj_mapper_->writeToString(alarm_message);
    } catch (vsoa::parser::ParsingError &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmServerController::PublishAlarm writeToString failed: %s",
            e.what());
        return -1;
    } catch (std::exception &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmServerController::PublishAlarm writeToString failed: %s",
            e.what());
        return -1;
    }
    int ret = sh_->Publish(url_alarm_publish_, str);
    if (ret != 0) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmServerController::PublishAlarm Publish failed, ret %d",
            ret);
        return -1;
    }
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmServerController::PublishAlarm Publish success");
    return 0;
}

void AlarmServerController::InitializeListener ()
{

}


AlarmServerController::AlarmServerController()
{
}

AlarmServerController::~AlarmServerController()
{
}