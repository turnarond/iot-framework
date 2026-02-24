/*
 * @Author: yanchaodong
 * @Date: 2025-11-05 09:55:29
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-05 14:37:36
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkevt_alarm/alarm_subscriber.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "util/plugin_interface.h"
#include "platform_sdk/client.h"
#include "platform_sdk/init.h"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"

class AlarmEventSource : public IEventSource 
{
public:
    AlarmEventSource() = default;
    ~AlarmEventSource() override {
        stop();
    }

    bool init(const std::string& configJson) override ;
    bool start(const EventCallback& cb) override ;
    void stop() override ;

    static void OnConnect(bool connect, std::string servername, 
                          vsoa_sdk::client::CLIENT_CONTEXT ctx, void *arg);

private:
    const std::string alarm_server_name_ = "alarm_server";
    std::string url_alarm_publish_ = "/v1/alarm_server/alarm_info/";

    vsoa_sdk::client::CLIENT_CONTEXT client_ctx_;
    std::shared_ptr<vsoa_sdk::client::ClientHandle> client_; 
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr;

    EventCallback user_callback_; // 产生事件的回调函数
};
