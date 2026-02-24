/*
 * @Author: yanchaodong
 * @Date: 2025-11-13 17:09:07
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-23
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_log.h
 * @Description: 报警日志记录模块
 * 负责记录报警的产生、恢复等事件到数据库
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "Poco/NotificationQueue.h"
#include "Poco/Notification.h"
#include "Poco/Thread.h"
#include "Poco/Data/Data.h"
#include <string>

/**
 * @brief 报警日志消息类
 * 用于在通知队列中传递报警日志信息
 */
class AlarmLogMessage : public Poco::Notification
{
public:
    /**
     * @brief 构造函数
     * @param point_id 点位ID
     * @param point_name 点位名称
     * @param point_value 点位值
     * @param rule_id 规则ID
     * @param rule_name 规则名称
     * @param rule_method 规则方法
     * @param alarm_type 报警类型（1-触发，2-恢复）
     * @param timestamp 时间戳
     */
    AlarmLogMessage(const std::string& point_id, const std::string& point_name, 
                   const std::string& point_value, int rule_id, 
                   const std::string& rule_name, int rule_method, 
                   int alarm_type, long long timestamp)
        : point_id_(point_id), point_name_(point_name), point_value_(point_value),
          rule_id_(rule_id), rule_name_(rule_name), rule_method_(rule_method),
          alarm_type_(alarm_type), timestamp_(timestamp)
    {}

    void duplicate() const
    {}

    virtual std::string name() const
    {
        return "AlarmLogMessage";
    }

    // 获取属性
    std::string point_id() const { return point_id_; }
    std::string point_name() const { return point_name_; }
    std::string point_value() const { return point_value_; }
    int rule_id() const { return rule_id_; }
    std::string rule_name() const { return rule_name_; }
    int rule_method() const { return rule_method_; }
    int alarm_type() const { return alarm_type_; }
    long long timestamp() const { return timestamp_; }

private:
    std::string point_id_;      // 点位ID
    std::string point_name_;    // 点位名称
    std::string point_value_;   // 点位值
    int rule_id_;               // 规则ID
    std::string rule_name_;     // 规则名称
    int rule_method_;           // 规则方法
    int alarm_type_;            // 报警类型（1-触发，2-恢复）
    long long timestamp_;       // 时间戳
};

/**
 * @brief 报警日志保存器类
 * 负责将报警日志保存到数据库
 */
class AlarmLogSaver : public Poco::Runnable {

public:
    /**
     * @brief 获取单例实例
     * @return AlarmLogSaver* 单例指针
     */
    static AlarmLogSaver* GetInstance();
    
    /**
     * @brief 构造函数
     */
    AlarmLogSaver();
    
    /**
     * @brief 析构函数
     */
    ~AlarmLogSaver();
    
    /**
     * @brief 初始化日志保存器
     * @return 0 成功，非0 失败
     */
    int Init();
    
    /**
     * @brief 停止日志保存器
     */
    void Stop();
    
    /**
     * @brief 添加报警日志
     * @param point_id 点位ID
     * @param point_name 点位名称
     * @param point_value 点位值
     * @param rule_id 规则ID
     * @param rule_name 规则名称
     * @param rule_method 规则方法
     * @param alarm_type 报警类型（1-触发，2-恢复）
     */
    void AddAlarmLog(const std::string& point_id, const std::string& point_name, 
                    const std::string& point_value, int rule_id, 
                    const std::string& rule_name, int rule_method, 
                    int alarm_type);

private:
    /**
     * @brief 线程运行函数
     */
    void run() override;
    
    /**
     * @brief 保存日志到数据库
     * @param msg 报警日志消息
     */
    void saveLogToDatabase(const AlarmLogMessage* msg);

private:
    Poco::NotificationQueue queue_;      // 通知队列
    Poco::Thread thread_;               // 工作线程
    bool running_;                      // 运行标志
    std::string db_file_;               // 数据库文件路径
};

// 单例宏
#define ALARM_LOG_SAVER AlarmLogSaver::GetInstance()

// 报警类型定义
enum class AlarmLogType {
    TRIGGER = 1,  // 报警触发
    CLEAR = 2     // 报警恢复
};
