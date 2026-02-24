/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: data_publisher.h .
*
* Date: 2025-07-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <mutex>

class TagData {
public:
    std::string name;
    std::string value;
    std::string time;
    std::string driver_name;
    std::string device_name;
public:
    TagData() : name(""),
        value(""),
        time(""),
        driver_name(""),
        device_name("") {
    };
    TagData(const char* name, const char* value, const char* time):
        name(name),
        value(value),
        time(time) {
    }
    void SetDriverName(const char* driver_name) {
        this->driver_name = driver_name;
    }
    void SetDeviceName(const char* device_name) {
        this->device_name = device_name;
    }
    ~TagData(){};
} ;

// Forward declaration
namespace nodeserver {
    class RTDB;
}

class DataCenter
{
public:
    DataCenter();
    ~DataCenter();

    // Direct access to underlying RTDB instance
    nodeserver::RTDB* GetRTDB();

    // Stats & health
    size_t Size() const;
    bool HealthCheck(std::string* outReason = nullptr) const;

    static DataCenter* GetInstance();

private:
    static DataCenter instance_;
    // 使用新的 RTDB 实现
    std::unique_ptr<nodeserver::RTDB> rtdb_;
};

#define DATA_CENTER DataCenter::GetInstance()
