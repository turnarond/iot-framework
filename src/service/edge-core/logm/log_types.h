/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: log_types.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include <string>
#include <chrono>
#include <unordered_map>

namespace edge {
namespace core {

// 日志级别
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// 日志条目
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string source;        // 日志来源 (模块/文件)
    uint32_t line;             // 行号
    std::string message;       // 日志消息
    std::string threadId;      // 线程ID
    std::string eventId;       // 事件ID (可选)
    
    // 转换为字符串
    std::string toString() const;
    
    // 转换为JSON
    std::string toJson() const;
};

// 事件日志 (业务事件)
struct EventLog {
    std::string eventId;        // 唯一事件ID
    std::string eventType;      // 事件类型
    std::chrono::system_clock::time_point timestamp;
    std::string source;         // 事件来源
    std::string target;         // 事件目标
    std::string action;         // 事件动作
    std::unordered_map<std::string, std::string> attributes; // 事件属性
    
    // 转换为JSON
    std::string toJson() const;
};

// 日志查询条件
struct LogQuery {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    LogLevel minLevel;
    std::string sourcePattern;
    std::string messagePattern;
    uint32_t maxResults;
};

} // namespace core
} // namespace edge