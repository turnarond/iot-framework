/*
* Copyright (c) 2026 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwconn_serial.h .
*
* Date: 2026-02-05
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef LWCONN_SERIAL_H
#define LWCONN_SERIAL_H

#include "lwconn_base.h"

// 串口连接类
class LWSerial : public LWConnBase {
public:
    LWSerial(const std::string& name, const std::string& port, int baudrate, 
             char parity = 'N', int databits = 8, int stopbits = 1);
    ~LWSerial() override;

    LWConnError start() override;
    void stop() override;
    LWConnError send(const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError disconnect() override;
    LWConnError reconnect(int timeout_ms = 0) override;
    bool isConnected() const override;
    void clearReceiveBuffer() override;

private:
    std::string port_;
    int baudrate_;
    char parity_;
    int databits_;
    int stopbits_;
    int serial_handle_; // 内部串口句柄
    mutable std::mutex serial_mutex_;
};

#endif // LWCONN_SERIAL_H