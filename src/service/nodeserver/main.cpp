/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: main.cpp .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "driver_collector.h"
#include "node_server.h"
#include "lwlog/lwlog.h"
#include "hmi_server.hpp"
#include "websocket_server.hpp"
#include "rtdb.hpp"

CLWLog g_logger;

int main(int argc, char *argv[])
{
    g_logger.SetLogFileName();
    NodeServer node_server;
    DRIVER_COLLECTOR->OnStart();
    node_server.OnStart();
    
    // 启动HMI服务器，使用端口8081
    StartHmiServer(8081);

    // 启动 WebSocket 推送服务，监听 9000 端口
    nodeserver::WebSocketServer ws;
    if (ws.start(9000)) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] WebSocket server started on port 9000");
        
        // 注册 RTDB 更新回调，广播到 WebSocket 会话
        if (DATA_CENTER && DATA_CENTER->GetRTDB()) {
            DATA_CENTER->GetRTDB()->addUpdateCallback([&ws](const nodeserver::TagRecord& rec){
                ws.broadcastPointUpdate(rec);
            });
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "[WebSocketServer] Registered RTDB update callback");
        } else {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[WebSocketServer] DATA_CENTER or RTDB not available, cannot register update callback");
        }
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[WebSocketServer] Failed to start WebSocket server on port 9000");
    }

    while (true){}
    return 0;
}
