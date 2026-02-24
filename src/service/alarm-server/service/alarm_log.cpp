/*
 * @Author: yanchaodong
 * @Date: 2025-11-13 17:09:07
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-23
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_log.cpp
 * @Description: 报警日志记录模块实现
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_log.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Exception.h"
#include "Poco/Timestamp.h"
#include "lwlog/lwlog.h"
#include "lwcomm/lwcomm.h"

#include <exception>
#include <string>

// 全局日志对象
extern CLWLog g_lwlog;

// 单例实现
static AlarmLogSaver* s_instance = nullptr;

AlarmLogSaver* AlarmLogSaver::GetInstance()
{
    if (!s_instance) {
        s_instance = new AlarmLogSaver();
    }
    return s_instance;
}

AlarmLogSaver::AlarmLogSaver()
    : running_(false), db_file_("lw_monitor.db")
{
    // 注册SQLite连接器
    Poco::Data::SQLite::Connector::registerConnector();
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmLogSaver::AlarmLogSaver: SQLite connector registered");
}

AlarmLogSaver::~AlarmLogSaver()
{
    Stop();
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmLogSaver::~AlarmLogSaver: destroyed");
}

int AlarmLogSaver::Init()
{
    // 获取数据库路径
    std::string db_path = LWComm::GetConfigPath();
    db_file_ = db_path + LW_OS_DIR_SEPARATOR + db_file_;
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmLogSaver::Init: using database file: %s", db_file_.c_str());

    // 启动工作线程
    running_ = true;
    thread_.setName("AlarmLogThread");
    thread_.start(*this);
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmLogSaver::Init: log saver initialized successfully");

    return 0;
}

void AlarmLogSaver::Stop()
{
    if (running_) {
        running_ = false;
        queue_.wakeUpAll();
        thread_.join();
        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmLogSaver::Stop: log saver stopped");
    }
}

void AlarmLogSaver::AddAlarmLog(const std::string& point_id, const std::string& point_name, 
                              const std::string& point_value, int rule_id, 
                              const std::string& rule_name, int rule_method, 
                              int alarm_type)
{
    // 获取当前时间戳
    long long timestamp = Poco::Timestamp().epochMicroseconds() / 1000; // 毫秒
    
    // 创建日志消息并加入队列
    AlarmLogMessage* msg = new AlarmLogMessage(
        point_id, point_name, point_value, rule_id, 
        rule_name, rule_method, alarm_type, timestamp
    );
    
    queue_.enqueueNotification(msg);
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmLogSaver::AddAlarmLog: added log for point %s, rule %d, type %d", 
        point_id.c_str(), rule_id, alarm_type);
}

void AlarmLogSaver::run()
{
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmLogSaver::run: log saver thread started");
    
    while (running_) {
        try {
            // 等待通知，最多等待1秒
            AlarmLogMessage* msg = dynamic_cast<AlarmLogMessage*>(queue_.waitDequeueNotification(1000));
            if (msg) {
                // 处理日志消息
                saveLogToDatabase(msg);
                delete msg;
            }
        } catch (Poco::Exception& ex) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmLogSaver::run: Poco exception: %s", 
                ex.displayText().c_str());
        } catch (std::exception& ex) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmLogSaver::run: std exception: %s", 
                ex.what());
        } catch (...) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmLogSaver::run: unknown exception");
        }
    }
    
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmLogSaver::run: log saver thread stopped");
}

void AlarmLogSaver::saveLogToDatabase(const AlarmLogMessage* msg)
{
    try {
        // 创建数据库会话
        Poco::Data::Session session("SQLite", db_file_);
        
        // 开始事务
        session.begin();
        
        // 准备SQL语句
        Poco::Data::Statement insert(session);
        
        // 插入报警日志
        std::string sql = R"(
            INSERT INTO t_alarm_log (
                rule_id, point_id, point_name, event_type, 
                value, threshold, message, ack_status, 
                start_time, end_time
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";
        
        insert << sql;
        
        // 解析点位值为浮点数
        float value = 0.0f;
        try {
            value = std::stof(msg->point_value());
        } catch (const std::exception& e) {
            g_lwlog.LogMessage(LW_LOGLEVEL_WARN, "AlarmLogSaver::saveLogToDatabase: failed to convert value %s to float: %s", 
                msg->point_value().c_str(), e.what());
        }
        
        // 构建消息内容
        std::string message;
        if (msg->alarm_type() == static_cast<int>(AlarmLogType::TRIGGER)) {
            message = "报警触发: " + msg->point_name() + " - " + msg->rule_name();
        } else {
            message = "报警恢复: " + msg->point_name() + " - " + msg->rule_name();
        }
        
        // 绑定参数
        insert << msg->rule_id()
               << std::stoi(msg->point_id())  // 转换为整数
               << msg->point_name()
               << msg->alarm_type()
               << value
               << 0.0f  // 阈值暂时设为0，实际应该从规则中获取
               << message
               << 0     // 未确认
               << msg->timestamp();
        
        // 对于恢复事件，设置结束时间
        if (msg->alarm_type() == static_cast<int>(AlarmLogType::CLEAR)) {
            insert << msg->timestamp();
        } else {
            insert << 0;  // 触发事件，结束时间为0
        }
        
        // 执行插入
        insert.execute();
        
        // 提交事务
        session.commit();
        
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmLogSaver::saveLogToDatabase: log saved successfully for point %s, rule %d", 
            msg->point_name().c_str(), msg->rule_id());
        
    } catch (Poco::Exception& ex) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmLogSaver::saveLogToDatabase: Poco exception: %s", 
            ex.displayText().c_str());
    } catch (std::exception& ex) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmLogSaver::saveLogToDatabase: std exception: %s", 
            ex.what());
    } catch (...) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmLogSaver::saveLogToDatabase: unknown exception");
    }
}