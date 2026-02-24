/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 11:00:38
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-05 18:19:32
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_server.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "Poco/NotificationQueue.h"
#include "Poco/Notification.h"
#include "Poco/Thread.h"

#include <unordered_map>

/*
 * 消息队列传输的报警点位的信息
 */
class AlarmPointMessage : public Poco::Notification
{
public:
    AlarmPointMessage(std::string point_id, std::string point_value)
        : point_id_(point_id), point_value_(point_value)
    {
    }

    std::string point_id() const
    {
        return point_id_;
    }

    std::string point_value() const
    {
        return point_value_;
    }

private:
    std::string point_id_;
    std::string point_value_;
};

/*
 * 单条报警规则（支持多条同类型规则，尤其是定值）
 */
struct AlarmRule {
    int id;                      // 规则唯一ID（来自数据库）
    std::string name;            // 报警名称，如 "高温报警"
    int method;                  // 1~8
    float threshold;             // 阈值
    float param1;                // 变化率规则：时间窗口（秒）
    float param2;                // 持续时间规则：持续时间（秒）
    float param3;                // 偏差规则：参考值
    bool enabled = true;
    std::string point_id;
    std::string point_name;
    // 用于排序的优先级（越小越先判断）
    static int getPriority(int method) {
        switch (method) {
            case 5: return 0; // 定值
            case 1: return 1; // 高高限
            case 2: return 2; // 高限
            case 3: return 3; // 低限
            case 4: return 4; // 低低限
            case 6: return 5; // 变化率
            case 7: return 6; // 持续时间
            case 8: return 7; // 偏差
            default: return 99;
        }
    }
    bool operator<(const AlarmRule& other) const {
        int p1 = getPriority(method);
        int p2 = getPriority(other.method);
        if (p1 != p2) return p1 < p2;
        return id < other.id; // 保证稳定排序
    }
};

/*
 * 发布的报警信息 
 */
class AlarmMessage {
public:
    enum class Type { CLEAR = 0, TRIGGER  };
    AlarmMessage(
        const std::string& point_name,
        const std::string& alarm_name,
        float value,
        float threshold,
        int method,
        Type type
    ) : point_name_(point_name),
        alarm_name_(alarm_name),
        value_(value),
        threshold_(threshold),
        method_(method),
        type_(type) {}
        // getters...
    std::string point_name() const { return point_name_; }
    std::string alarm_name() const { return alarm_name_; }
    float value() const { return value_; }
    float threshold() const { return threshold_; }
    int method() const { return method_; }
    Type type() const { return type_; }

private:
    std::string point_name_;
    std::string alarm_name_;
    float value_;
    float threshold_;
    int method_;
    Type type_;
};

class AlarmServer : Poco::Runnable 
{

public:
    int InitAlarmServerQueue();
    int PushAlarmMsg(std::string tag_name, std::string tag_value);

    struct AlarmState {
        bool active = false;
        long long last_trigger_time = 0; // ms
        long long activation_time = 0;    // 激活时间（用于持续时间规则）
        AlarmState(bool active, long long last_trigger_time) : active(active), last_trigger_time(last_trigger_time), activation_time(last_trigger_time) {}
        AlarmState() = default;
    };
    // 点位值历史记录（用于变化率规则）：point_id -> list of (value, timestamp)
    struct ValueHistory {
        float value;
        long long timestamp; // ms
        ValueHistory(float v, long long t) : value(v), timestamp(t) {}
    };
    // 访问器方法，用于规则处理器和状态管理器
    std::unordered_map<std::string, std::vector<AlarmRule>>& getPointAlarmConfigs() { return point_alarm_configs_; }
    std::unordered_map<std::string, std::vector<ValueHistory>>& getValueHistories() { return value_histories_; }
    std::unordered_map<std::string, std::unordered_map<int, AlarmState>>& getAlarmStates() { return alarm_states_; }

public:
    static AlarmServer* GetInstance()
    {
        static AlarmServer instance;
        return &instance;
    }

    void run() override;

private:
    Poco::NotificationQueue queue_;
    size_t queue_size_ = 1000;

private:

    // 配置映射：point_id -> PointAlarmConfig
    std::unordered_map<std::string, std::vector<AlarmRule> > point_alarm_configs_;

    // 告警信息缓存：point_id -> rule.id -> AlarmState
    std::unordered_map<std::string, std::unordered_map<int, AlarmState>> alarm_states_;

    
    std::unordered_map<std::string, std::vector<ValueHistory>> value_histories_;

    // 数据库相关
    const std::string db_file_ = "bas-business.db";

    // 消息队列处理线程
    Poco::Thread msg_queue_thread_;
private:
    AlarmServer() = default;
    ~AlarmServer() = default;
};

#define ALARM_SERVER AlarmServer::GetInstance()
