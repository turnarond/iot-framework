/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: node_server.h .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "driver_collector.h"
#include "data_center.h"

#include "platform_sdk/server.h"
#include <thread>

using namespace vsoa_sdk;
class NodeServer
{
public:
    int OnStart();
    static int MainTask(NodeServer* );
    int OnStop();
private:
    std::string server_name_ = "node_server";
    vsoa_sdk::server::ServerHandle *server_handler_ = nullptr;
    std::string rpc_request_path_ = "/tags/read";
    std::string rpc_control_path_ = "/tags/control";
    std::thread thread_main_;
    bool bstop_ = true;
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr;
public:
    static NodeServer node_server_;
    static NodeServer *GetInstance();
public:
    NodeServer();
    ~NodeServer();
};

#define NODE_SERVER NodeServer::GetInstance()
