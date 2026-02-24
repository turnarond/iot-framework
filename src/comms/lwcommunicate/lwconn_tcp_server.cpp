/*
* Copyright (c) 2026 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwconn_tcp_server.cpp .
*
* Date: 2026-02-05
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "lwconn_tcp_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

// TCP服务端连接类实现
LWTcpServer::LWTcpServer(const std::string& name, const std::string& listen_address, int port)
    : LWConnBase(LWConnType::TCP_SERVER, name),
      listen_address_(listen_address),
      port_(port),
      server_socket_(0) {
}

LWTcpServer::~LWTcpServer() {
    stop();
}

LWConnError LWTcpServer::start() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    if (server_socket_) {
        return LWConnError::SUCCESS;
    }

    updateStatus(LWConnStatus::CONNECTING);
    
    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        log(LWLogLevel::ERROR, "LWTcpServer::start: socket creation failed");
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::CONNECTION_FAILED;
    }

    // 设置地址重用
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log(LWLogLevel::ERROR, "LWTcpServer::start: setsockopt failed");
        close(sock);
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::INTERNAL_ERROR;
    }

    // 绑定地址
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    
    if (listen_address_.empty() || listen_address_ == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, listen_address_.c_str(), &addr.sin_addr) <= 0) {
            log(LWLogLevel::ERROR, "LWTcpServer::start: invalid address: %s", listen_address_.c_str());
            close(sock);
            updateStatus(LWConnStatus::ERROR);
            return LWConnError::INVALID_PARAM;
        }
    }

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log(LWLogLevel::ERROR, "LWTcpServer::start: bind failed");
        close(sock);
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::CONNECTION_FAILED;
    }

    // 开始监听
    if (listen(sock, 10) < 0) {
        log(LWLogLevel::ERROR, "LWTcpServer::start: listen failed");
        close(sock);
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::CONNECTION_FAILED;
    }

    server_socket_ = sock;
    updateStatus(LWConnStatus::CONNECTED);
    notifyConnChange(true, listen_address_ + ":" + std::to_string(port_));
    
    log(LWLogLevel::DEBUG, "LWTcpServer::start: listening on %s:%d", listen_address_.c_str(), port_);
    return LWConnError::SUCCESS;
}

void LWTcpServer::stop() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    // 关闭所有客户端连接
    for (auto& client : client_sockets_) {
        close((int)client.second);
    }
    client_sockets_.clear();
    
    // 关闭服务器socket
    if (server_socket_) {
        int sock = (int)server_socket_;
        close(sock);
        server_socket_ = 0;
        updateStatus(LWConnStatus::DISCONNECTED);
        notifyConnChange(false, "");
        log(LWLogLevel::DEBUG, "LWTcpServer::stop: stopped");
    }
}

LWConnError LWTcpServer::send(const char* data, size_t length, int timeout_ms) {
    // TCP服务端默认发送到所有连接的客户端
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    if (client_sockets_.empty()) {
        return LWConnError::NOT_CONNECTED;
    }

    bool success = true;
    for (auto& client : client_sockets_) {
        int sock = (int)client.second;
        ssize_t sent = ::send(sock, data, length, 0);
        if (sent < 0 || (size_t)sent != length) {
            success = false;
        }
    }

    return success ? LWConnError::SUCCESS : LWConnError::SEND_FAILED;
}

LWConnError LWTcpServer::receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms) {
    // TCP服务端需要处理多客户端，这里实现简单版本，只接收第一个客户端的数据
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    if (client_sockets_.empty()) {
        return LWConnError::NOT_CONNECTED;
    }

    // 检查是否有新的客户端连接
    if (server_socket_) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET((int)server_socket_, &read_set);
        
        // 检查已连接的客户端
        for (auto& client : client_sockets_) {
            FD_SET((int)client.second, &read_set);
        }
        
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        
        int max_fd = (int)server_socket_;
        for (auto& client : client_sockets_) {
            if ((int)client.second > max_fd) {
                max_fd = (int)client.second;
            }
        }
        
        int result = select(max_fd + 1, &read_set, nullptr, nullptr, &timeout);
        if (result < 0) {
            return LWConnError::RECEIVE_FAILED;
        } else if (result == 0) {
            return LWConnError::TIMEOUT;
        }
        
        // 处理新连接
        if (FD_ISSET((int)server_socket_, &read_set)) {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_sock = accept((int)server_socket_, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_sock >= 0) {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                std::string client_address = std::string(client_ip) + ":" + std::to_string(ntohs(client_addr.sin_port));
                client_sockets_[client_address] = client_sock;
                log(LWLogLevel::DEBUG, "LWTcpServer::receive: new client connected: %s", client_address.c_str());
            }
        }
        
        // 处理客户端数据
        for (auto it = client_sockets_.begin(); it != client_sockets_.end();) {
            int client_sock = (int)it->second;
            if (FD_ISSET(client_sock, &read_set)) {
                ssize_t received = recv(client_sock, buffer, buffer_size, 0);
                if (received < 0) {
                    // 错误
                    close(client_sock);
                    it = client_sockets_.erase(it);
                } else if (received == 0) {
                    // 连接关闭
                    close(client_sock);
                    it = client_sockets_.erase(it);
                } else {
                    // 收到数据
                    received_size = (size_t)received;
                    return LWConnError::SUCCESS;
                }
            } else {
                ++it;
            }
        }
    }
    
    return LWConnError::RECEIVE_FAILED;
}

LWConnError LWTcpServer::disconnect() {
    stop();
    return LWConnError::SUCCESS;
}

LWConnError LWTcpServer::reconnect(int timeout_ms) {
    stop();
    return start();
}

bool LWTcpServer::isConnected() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return server_socket_ != 0;   
}

void LWTcpServer::clearReceiveBuffer() {
    // TCP服务端不需要清理接收缓冲区
}

LWConnError LWTcpServer::sendToClient(const std::string& client_address, const char* data, size_t length, int timeout_ms) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = client_sockets_.find(client_address);
    if (it == client_sockets_.end()) {
        return LWConnError::NOT_CONNECTED;
    }

    int sock = (int)it->second;
    ssize_t sent = ::send(sock, data, length, 0);
    if (sent < 0 || (size_t)sent != length) {
        return LWConnError::SEND_FAILED;
    }

    return LWConnError::SUCCESS;
}

std::vector<std::string> LWTcpServer::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    std::vector<std::string> clients;
    for (auto& client : client_sockets_) {
        clients.push_back(client.first);
    }
    return clients;
}