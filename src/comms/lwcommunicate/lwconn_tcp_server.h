/*
* Copyright (c) 2026 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwconn_tcp_server.h .
*
* Date: 2026-02-05
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef LWCONN_TCP_SERVER_H
#define LWCONN_TCP_SERVER_H

#include "lwconn_base.h"

#include <vector>
#include <map>

// TCP服务端连接类
class LWTcpServer : public LWConnBase {
public:
    LWTcpServer(const std::string& name, const std::string& listen_address, int port);
    ~LWTcpServer() override;

    LWConnError start() override;
    void stop() override;
    LWConnError send(const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError disconnect() override;
    LWConnError reconnect(int timeout_ms = 0) override;
    bool isConnected() const override;
    void clearReceiveBuffer() override;

    // 发送数据到指定客户端
    LWConnError sendToClient(const std::string& client_address, const char* data, size_t length, int timeout_ms = 0);

    // 获取连接的客户端列表
    std::vector<std::string> getConnectedClients() const;

private:
    std::string listen_address_;
    int port_;
    int server_socket_; // 内部服务器socket指针
    std::map<std::string, int> client_sockets_; // 客户端地址到socket的映射
    mutable std::mutex clients_mutex_;
};

#endif // LWCONN_TCP_SERVER_H