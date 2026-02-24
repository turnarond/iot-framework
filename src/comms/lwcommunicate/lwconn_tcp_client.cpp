/*
* Copyright (c) 2026 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwconn_tcp_client.cpp .
*
* Date: 2026-02-05
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "lwconn_tcp_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// TCP客户端连接类实现
LWTcpClient::LWTcpClient(const std::string& name, const std::string& host, int port, int reconnect_interval)
    : LWConnBase(LWConnType::TCP_CLIENT, name),
      host_(host),
      port_(port),
      reconnect_interval_(reconnect_interval),
      socket_(0),
      recv_buffer_(4096) {
}

LWTcpClient::~LWTcpClient() {
    stop();
}

LWConnError LWTcpClient::start() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_) {
        return LWConnError::SUCCESS;
    }

    updateStatus(LWConnStatus::CONNECTING);
    
    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        log(LWLogLevel::ERROR, "LWTcpClient::start: socket creation failed");
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::CONNECTION_FAILED;
    }

    // 设置地址
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) {
        log(LWLogLevel::ERROR, "LWTcpClient::start: invalid address: %s", host_.c_str());
        close(sock);
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::INVALID_PARAM;
    }

    // 连接
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log(LWLogLevel::ERROR, "LWTcpClient::start: connect failed: %s", host_.c_str());
        close(sock);
        updateStatus(LWConnStatus::DISCONNECTED);
        return LWConnError::CONNECTION_FAILED;
    }

    socket_ = sock;
    updateStatus(LWConnStatus::CONNECTED);
    notifyConnChange(true, host_ + ":" + std::to_string(port_));
    
    log(LWLogLevel::DEBUG, "LWTcpClient::start: connected to %s:%d", host_.c_str(), port_);
    return LWConnError::SUCCESS;
}

void LWTcpClient::stop() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_) {
        int sock = socket_;
        close(sock);
        socket_ = 0;
        updateStatus(LWConnStatus::DISCONNECTED);
        notifyConnChange(false, "");
        log(LWLogLevel::DEBUG, "LWTcpClient::stop: disconnected");
    }
}

LWConnError LWTcpClient::send(const char* data, size_t length, int timeout_ms) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (!socket_) {
        return LWConnError::NOT_CONNECTED;
    }

    int sock = (int)socket_;
    ssize_t sent = ::send(sock, data, length, 0);
    
    if (sent < 0) {
        log(LWLogLevel::ERROR, "LWTcpClient::send: send failed");
        return LWConnError::SEND_FAILED;
    }

    if ((size_t)sent != length) {
        log(LWLogLevel::ERROR, "LWTcpClient::send: partial send");
        return LWConnError::SEND_FAILED;
    }

    return LWConnError::SUCCESS;
}

LWConnError LWTcpClient::receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (!socket_) {
        return LWConnError::NOT_CONNECTED;
    }

    int sock = (int)socket_;
    
    // 设置超时
    if (timeout_ms > 0) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sock, &read_set);
        
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        
        int result = select(sock + 1, &read_set, nullptr, nullptr, &timeout);
        if (result < 0) {
            return LWConnError::RECEIVE_FAILED;
        } else if (result == 0) {
            return LWConnError::TIMEOUT;
        }
    }

    ssize_t received = recv(sock, buffer, buffer_size, 0);
    if (received < 0) {
        log(LWLogLevel::ERROR, "LWTcpClient::receive: recv failed");
        return LWConnError::RECEIVE_FAILED;
    } else if (received == 0) {
        // 连接关闭
        stop();
        return LWConnError::NOT_CONNECTED;
    }

    received_size = (size_t)received;
    return LWConnError::SUCCESS;
}

LWConnError LWTcpClient::disconnect() {
    stop();
    return LWConnError::SUCCESS;
}

LWConnError LWTcpClient::reconnect(int timeout_ms) {
    stop();
    return start();
}

bool LWTcpClient::isConnected() const {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    return socket_ != 0;
}

void LWTcpClient::clearReceiveBuffer() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    recv_buffer_.clear();
    recv_buffer_.resize(4096);
}
