/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 10:50:28
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-22
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/api/alarm_node_subscriber.h
 * @Description: 侦听报警节点信息（DDS实现）
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "lwdistcomm/lwdistcomm.h"
#include "lwdistcomm/address.h"
#include "lwdistcomm/message.h"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "lwcomm/lwcomm.h"

class AlarmNodeSubscriber {
public:
    int InitAlarmNodeSubscriber();
    int OnStop();
private:
    static void OnSubscriptionCb(void *arg, const char *url, const lwdistcomm_message_t *msg);
    static void OnRpcResponseCb(void *arg, int status, const lwdistcomm_message_t *resp);
private:
    const std::string node_server_name_ = "node_server";
    std::string publish_url_ = "/tags/update";
    std::string dds_address_ = "/tmp/nodeserver_dds.sock";
    lwdistcomm_client_t *client_ = nullptr;
    lwdistcomm_address_t *addr_ = nullptr;
private:
    static AlarmNodeSubscriber* s_instance;
public:
    static AlarmNodeSubscriber* GetInstance();
private:
    AlarmNodeSubscriber() = default;
    ~AlarmNodeSubscriber() = default;
};

#define ALARM_NODE_SUBSCRIBER AlarmNodeSubscriber::GetInstance()
