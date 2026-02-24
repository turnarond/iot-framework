#include "LogService.h"
#include <stdarg.h>
#include <cstdio>
#include <cstring>
#include <iostream>

namespace video_server {

LogService::LogService() 
    : logger_(nullptr), 
      logLevel_(INFO), 
      logDir_("./logs") {
}

LogService::~LogService() {
    uninitialize();
}

bool LogService::initialize(const std::string& logDir, LogLevel level) {
    logDir_ = logDir;
    logLevel_ = level;
    
    // Initialize lwlog
    logger_ = new CLWLog();
    if (!logger_) {
        std::cerr << "Failed to create logger instance" << std::endl;
        return false;
    }
    
    // Set log file name
    if (!logger_->SetLogFileName("video-server")) {
        std::cerr << "Failed to set log file name" << std::endl;
        delete logger_;
        logger_ = nullptr;
        return false;
    }
    
    LOG_INFO("LogService initialized. Log directory: %s, Log level: %d", logDir_.c_str(), logLevel_);
    
    return true;
}

bool LogService::uninitialize() {
    if (logger_) {
        LOG_INFO("LogService uninitialized");
        delete logger_;
        logger_ = nullptr;
    }
    return true;
}

void LogService::log(LogLevel level, const char* format, ...) {
    if (!logger_ || level < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    va_list args;
    va_start(args, format);
    
    switch (level) {
        case DEBUG:
            logger_->LogMessage(LW_LOGLEVEL_DEBUG, format, args);
            break;
        case INFO:
            logger_->LogMessage(LW_LOGLEVEL_INFO, format, args);
            break;
        case WARN:
            logger_->LogMessage(LW_LOGLEVEL_WARN, format, args);
            break;
        case ERROR:
            logger_->LogErrMessage(format, args);
            break;
        case CRITICAL:
            logger_->LogMessage(LW_LOGLEVEL_CRITICAL, format, args);
            break;
    }
    
    va_end(args);
}

void LogService::logDebug(const char* format, ...) {
    if (!logger_ || DEBUG < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    va_list args;
    va_start(args, format);
    logger_->LogMessage(LW_LOGLEVEL_DEBUG, format, args);
    va_end(args);
}

void LogService::logInfo(const char* format, ...) {
    if (!logger_ || INFO < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    va_list args;
    va_start(args, format);
    logger_->LogMessage(LW_LOGLEVEL_INFO, format, args);
    va_end(args);
}

void LogService::logWarn(const char* format, ...) {
    if (!logger_ || WARN < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    va_list args;
    va_start(args, format);
    logger_->LogMessage(LW_LOGLEVEL_WARN, format, args);
    va_end(args);
}

void LogService::logError(const char* format, ...) {
    if (!logger_ || ERROR < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    va_list args;
    va_start(args, format);
    logger_->LogErrMessage(format, args);
    va_end(args);
}

void LogService::logCritical(const char* format, ...) {
    if (!logger_ || CRITICAL < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    va_list args;
    va_start(args, format);
    logger_->LogMessage(LW_LOGLEVEL_CRITICAL, format, args);
    va_end(args);
}

void LogService::setLogLevel(LogLevel level) {
    logLevel_ = level;
    LOG_INFO("Log level set to: %d", level);
}

LogService::LogLevel LogService::getLogLevel() const {
    return logLevel_;
}

bool LogService::setLogDirectory(const std::string& logDir) {
    logDir_ = logDir;
    LOG_INFO("Log directory set to: %s", logDir_.c_str());
    return true;
}

std::string LogService::getLogDirectory() const {
    return logDir_;
}

LogService& LogService::instance() {
    static LogService instance;
    return instance;
}

} // namespace video_server