/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: driver_collector.h .
*
* Date: 2025-07-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "lwipcssn/ipc_server.h"

#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include <string>
#include <thread>
#include <unordered_map>
#include <lwmsgq/lwmsgq.h>

class DriverCollector
{
public:
    int OnStart();
    int OnStop();

    static void SrvThreadMain(void* arg);
    int SetTagControl(std::string tag_name, std::string tag_value);

public:
    static DriverCollector* GetInstance();
    static DriverCollector instance_;

private:
    static void OnConnectedCb(void *arg, ipc_server_t *server, ipc_cli_id_t id, bool connect);
    static void OnDatagramCb(void *arg, ipc_server_t *server, ipc_cli_id_t id,
                                       ipc_url_t *url, ipc_payload_t *payload);
    static int OnDevCtrlCb(void* msg, int msglen, void* userctx);
private:
    std::string server_name_ = "node_server";
    std::string publish_url_ = "/tags/update";
    std::string taginit_url_ = "/tags/init";
    std::string control_url_ = "/control";
    ipc_url_t pub_url_ {
        .url = (char*)publish_url_.c_str(),
        .url_len = publish_url_.length()
    };
    bool bstop_ = false; // 线程停止标志
    std::thread main_thread_;
    ipc_server_t *server_;
    std::string sock_addr_;
    std::string collector_url_;

    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr;
    std::unordered_map<std::string, int> client_map_; // map of driver id to client id

    PLW_MSGQUE_S dev_ctrl_que_;
    std::thread ctrl_thread_;
};

#define DRIVER_COLLECTOR DriverCollector::GetInstance()
