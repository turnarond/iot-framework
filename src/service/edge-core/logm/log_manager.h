/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: log_manager.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include "log_types.h"
#include "../cfgm/config_types.h"
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include "cloud/cloud_connector.h"

namespace edge {
namespace core {

class LogManager {
public:
    static LogManager *GetInstance();
    
    // 初始化
    bool Init();
    
    // 记录日志
    void Log(LogLevel level, const char* source, uint32_t line, const char* format, ...);
    
    // 记录事件日志
    void LogEvent(const std::string& eventType, 
                 const std::string& source, 
                 const std::string& target,
                 const std::string& action,
                 const std::unordered_map<std::string, std::string>& attributes = {});
    
    // 查询日志
    std::vector<LogEntry> QueryLogs(const LogQuery& query) const;
    
    // 查询事件日志
    std::vector<EventLog> QueryEventLogs(
        std::chrono::system_clock::time_point startTime,
        std::chrono::system_clock::time_point endTime,
        const std::string& eventTypePattern = "") const;
    
    // 上传日志到云端
    bool UploadLogs();
    
    // 设置日志上传回调
    using UploadCallback = std::function<void(bool success, const std::string& message)>;
    void SetUploadCallback(UploadCallback callback);

private:
    LogManager() = default;
    ~LogManager() = default;
    
    mutable std::mutex mutex_;
    LoggingConfig config_;
    LogLevel currentLevel_;
    std::string logFilePath_;
    FILE* logFile_;
    std::queue<LogEntry> uploadQueue_;
    std::queue<EventLog> eventQueue_;
    bool uploadEnabled_;
    bool running_;
    std::thread uploadThread_;
    UploadCallback uploadCallback_;
    // 是否上云？
    // edge::cloud::CloudConnector& cloudConnector_;

    // 清理旧日志
    void CleanupOldLogs();

    // 写入日志到文件
    void WriteToFile(const LogEntry& entry);
    
    // 轮转日志文件
    void RotateLogFile();
    
    // 上传线程
    void UploadThreadFunc();
    
    // 上传单个日志条目
    bool UploadLogEntry(const LogEntry& entry);
    
    // 上传事件日志
    bool UploadEventLog(const EventLog& event);
    
    // 获取日志级别字符串
    static std::string LevelToString(LogLevel level);
    
    // 获取当前时间字符串
    static std::string GetCurrentTimeString();
    
    // 确保目录存在
    static bool EnsureDirectoryExists(const std::string& path);

private:
    static LogManager instance_;
};
#define LOG_MANAGER LogManager::GetInstance()

// 便捷日志宏
#define ELOG_DEBUG(fmt, ...) LOG_MANAGER->Log(\
    edge::core::LogLevel::DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ELOG_INFO(fmt, ...) LOG_MANAGER->Log(\
    edge::core::LogLevel::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ELOG_WARNING(fmt, ...) LOG_MANAGER->Log(\
    edge::core::LogLevel::WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ELOG_ERROR(fmt, ...) LOG_MANAGER->Log(\
    edge::core::LogLevel::ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ELOG_CRITICAL(fmt, ...) LOG_MANAGER->Log(\
    edge::core::LogLevel::CRITICAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ELOG_EVENT(eventType, source, target, action, attrs) \
    LOG_MANAGER->LogEvent(eventType, source, target, action, attrs)

} // namespace core
} // namespace edge