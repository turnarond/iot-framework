#ifndef LOG_SERVICE_H
#define LOG_SERVICE_H

#include <string>
#include <mutex>
#include "lwlog/lwlog.h"

namespace video_server {

class LogService {
public:
    enum LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        CRITICAL = 4
    };
    
    LogService();
    ~LogService();
    
    bool initialize(const std::string& logDir, LogLevel level = INFO);
    bool uninitialize();
    
    void log(LogLevel level, const char* format, ...);
    void logDebug(const char* format, ...);
    void logInfo(const char* format, ...);
    void logWarn(const char* format, ...);
    void logError(const char* format, ...);
    void logCritical(const char* format, ...);
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    bool setLogDirectory(const std::string& logDir);
    std::string getLogDirectory() const;
    
    static LogService& instance();
    
private:
    CLWLog* logger_;
    LogLevel logLevel_;
    std::string logDir_;
    std::mutex logMutex_;
};

#define LOG_DEBUG(...) LogService::instance().logDebug(__VA_ARGS__)
#define LOG_INFO(...) LogService::instance().logInfo(__VA_ARGS__)
#define LOG_WARN(...) LogService::instance().logWarn(__VA_ARGS__)
#define LOG_ERROR(...) LogService::instance().logError(__VA_ARGS__)
#define LOG_CRITICAL(...) LogService::instance().logCritical(__VA_ARGS__)

} // namespace video_server

#endif // LOG_SERVICE_H