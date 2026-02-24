/*
* Copyright (c) 2026 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwconn_serial.cpp .
*
* Date: 2026-02-05
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "lwconn_serial.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <cstring>

// 串口连接类实现
LWSerial::LWSerial(const std::string& name, const std::string& port, int baudrate, char parity, int databits, int stopbits)
    : LWConnBase(LWConnType::SERIAL, name),
      port_(port),
      baudrate_(baudrate),
      parity_(parity),
      databits_(databits),
      stopbits_(stopbits),
      serial_handle_(0) {
}

LWSerial::~LWSerial() {
    stop();
}

LWConnError LWSerial::start() {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    
    if (serial_handle_) {
        return LWConnError::SUCCESS;
    }

    updateStatus(LWConnStatus::CONNECTING);
    
    // 打开串口
    int fd = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        log(LWLogLevel::ERROR, "LWSerial::start: open failed: %s", port_.c_str());
        updateStatus(LWConnStatus::ERROR);
        return LWConnError::CONNECTION_FAILED;
    }

    // 配置串口
    struct termios options;
    tcgetattr(fd, &options);
    
    // 设置波特率
    speed_t speed;
    switch (baudrate_) {
        case 9600:
            speed = B9600;
            break;
        case 19200:
            speed = B19200;
            break;
        case 38400:
            speed = B38400;
            break;
        case 57600:
            speed = B57600;
            break;
        case 115200:
            speed = B115200;
            break;
        default:
            speed = B9600;
            break;
    }
    
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
    
    // 设置数据位
    options.c_cflag &= ~CSIZE;
    switch (databits_) {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
        default:
            options.c_cflag |= CS8;
            break;
    }
    
    // 设置奇偶校验
    switch (parity_) {
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'E':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 'N':
        default:
            options.c_cflag &= ~PARENB;
            break;
    }
    
    // 设置停止位
    if (stopbits_ == 2) {
        options.c_cflag |= CSTOPB;
    } else {
        options.c_cflag &= ~CSTOPB;
    }
    
    // 应用配置
    tcsetattr(fd, TCSANOW, &options);
    
    serial_handle_ = fd;
    updateStatus(LWConnStatus::CONNECTED);
    notifyConnChange(true, port_);
    
    log(LWLogLevel::DEBUG, "LWSerial::start: opened: %s, baudrate: %d", port_.c_str(), baudrate_);
    return LWConnError::SUCCESS;
}

void LWSerial::stop() {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    
    if (serial_handle_) {
        int fd = (int)serial_handle_;
        close(fd);
        serial_handle_ = 0;
        updateStatus(LWConnStatus::DISCONNECTED);
        notifyConnChange(false, "");
        log(LWLogLevel::DEBUG, "LWSerial::stop: closed: %s", port_.c_str());
    }
}

LWConnError LWSerial::send(const char* data, size_t length, int timeout_ms) {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    
    if (!serial_handle_) {
        return LWConnError::NOT_CONNECTED;
    }

    int fd = (int)serial_handle_;
    ssize_t sent = write(fd, data, length);
    if (sent < 0) {
        log(LWLogLevel::ERROR, "LWSerial::send: write failed");
        return LWConnError::SEND_FAILED;
    }

    if ((size_t)sent != length) {
        log(LWLogLevel::ERROR, "LWSerial::send: partial write");
        return LWConnError::SEND_FAILED;
    }

    return LWConnError::SUCCESS;
}

LWConnError LWSerial::receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms) {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    
    if (!serial_handle_) {
        return LWConnError::NOT_CONNECTED;
    }

    int fd = (int)serial_handle_;
    
    // 设置超时
    if (timeout_ms > 0) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);
        
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        
        int result = select(fd + 1, &read_set, nullptr, nullptr, &timeout);
        if (result < 0) {
            return LWConnError::RECEIVE_FAILED;
        } else if (result == 0) {
            return LWConnError::TIMEOUT;
        }
    }

    ssize_t received = read(fd, buffer, buffer_size);
    if (received < 0) {
        log(LWLogLevel::ERROR, "LWSerial::receive: read failed");
        return LWConnError::RECEIVE_FAILED;
    }

    received_size = (size_t)received;
    return LWConnError::SUCCESS;
}

LWConnError LWSerial::disconnect() {
    stop();
    return LWConnError::SUCCESS;
}

LWConnError LWSerial::reconnect(int timeout_ms) {
    stop();
    return start();
}

bool LWSerial::isConnected() const {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    return serial_handle_ != 0;
}

void LWSerial::clearReceiveBuffer() {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    
    if (serial_handle_) {
        int fd = (int)serial_handle_;
        tcflush(fd, TCIFLUSH);
    }
}