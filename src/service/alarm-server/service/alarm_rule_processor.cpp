/*
 * @Author: yanchaodong
 * @Date: 2026-02-22
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_rule_processor.cpp
 * @Description: 报警规则处理器实现
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_rule_processor.h"
#include "lwlog/lwlog.h"
#include "Poco/Timestamp.h"

extern CLWLog g_lwlog;

// 工厂方法实现
std::unique_ptr<AlarmRuleProcessor> AlarmRuleProcessorFactory::createProcessor(int method) {
    switch (method) {
        case 1: // HH
        case 2: // H
            return std::make_unique<ThresholdRuleProcessor>(true);
        case 3: // L
        case 4: // LL
            return std::make_unique<ThresholdRuleProcessor>(false);
        case 5: // 定值
            return std::make_unique<FixedValueRuleProcessor>();
        case 6: // 变化率
            return std::make_unique<RateOfChangeRuleProcessor>();
        case 7: // 持续时间
            return std::make_unique<DurationRuleProcessor>();
        case 8: // 偏差
            return std::make_unique<DeviationRuleProcessor>();
        default:
            g_lwlog.LogMessage(LW_LOGLEVEL_WARN, "AlarmRuleProcessorFactory::createProcessor: unknown method %d", method);
            return nullptr;
    }
}

// 阈值规则处理器
bool ThresholdRuleProcessor::processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) {
    float value_f = 0.0f;
    try {
        value_f = std::stof(point_value);
    } catch (const std::exception &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "ThresholdRuleProcessor::processRule: can not convert tag %s value %s to float",
            point_id.c_str(), point_value.c_str());
        return false;
    }
    
    if (isHighThreshold_) {
        if (value_f > rule.threshold) {
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "ThresholdRuleProcessor::processRule: value %f > threshold %f for tag %s", 
                value_f, rule.threshold, point_id.c_str());
            return true;
        }
    } else {
        if (value_f < rule.threshold) {
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "ThresholdRuleProcessor::processRule: value %f < threshold %f for tag %s", 
                value_f, rule.threshold, point_id.c_str());
            return true;
        }
    }
    return false;
}

// 定值规则处理器
bool FixedValueRuleProcessor::processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) {
    float value_f = 0.0f;
    try {
        value_f = std::stof(point_value);
    } catch (const std::exception &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "FixedValueRuleProcessor::processRule: can not convert tag %s value %s to float",
            point_id.c_str(), point_value.c_str());
        return false;
    }
    
    if (std::abs(value_f - rule.threshold) < 1e-6) {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "FixedValueRuleProcessor::processRule: value %f == threshold %f for tag %s", 
            value_f, rule.threshold, point_id.c_str());
        return true;
    }
    return false;
}

// 变化率规则处理器
bool RateOfChangeRuleProcessor::processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) {
    float value_f = 0.0f;
    try {
        value_f = std::stof(point_value);
    } catch (const std::exception &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "RateOfChangeRuleProcessor::processRule: can not convert tag %s value %s to float",
            point_id.c_str(), point_value.c_str());
        return false;
    }
    
    long long current_time = Poco::Timestamp().epochMicroseconds() / 1000; // ms
    
    // 添加当前值到历史记录
    auto& value_histories_ = server->getValueHistories();
    value_histories_[point_id].push_back({value_f, current_time});
    
    // 清理过期的历史记录
    long long window_ms = static_cast<long long>(rule.param1 * 1000);
    auto& history = value_histories_[point_id];
    while (!history.empty() && current_time - history.front().timestamp > window_ms) {
        history.erase(history.begin());
    }
    
    // 检查是否有足够的历史数据
    if (history.size() >= 2) {
        float first_value = history.front().value;
        float last_value = history.back().value;
        long long time_diff_ms = history.back().timestamp - history.front().timestamp;
        
        if (time_diff_ms > 0) {
            float rate_of_change = std::abs(last_value - first_value) / (time_diff_ms / 1000.0f);
            if (rate_of_change > rule.threshold) {
                g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "RateOfChangeRuleProcessor::processRule: rate of change %f > threshold %f for tag %s", 
                    rate_of_change, rule.threshold, point_id.c_str());
                return true;
            }
        }
    }
    return false;
}

// 持续时间规则处理器
bool DurationRuleProcessor::processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) {
    float value_f = 0.0f;
    try {
        value_f = std::stof(point_value);
    } catch (const std::exception &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "DurationRuleProcessor::processRule: can not convert tag %s value %s to float",
            point_id.c_str(), point_value.c_str());
        return false;
    }
    
    bool condition_met = (value_f > rule.threshold);
    
    if (condition_met) {
        // 检查是否已经有该规则的状态
        auto& alarm_states_ = server->getAlarmStates();
        auto alarm_status = alarm_states_.find(point_id);
        if (alarm_status != alarm_states_.end()) {
            auto rule_state = alarm_status->second.find(rule.id);
            if (rule_state != alarm_status->second.end()) {
                if (rule_state->second.active) {
                    // 已经激活，检查持续时间
                    long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
                    long long duration_ms = current_time - rule_state->second.activation_time;
                    long long required_ms = static_cast<long long>(rule.param2 * 1000);
                    
                    if (duration_ms >= required_ms) {
                        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "DurationRuleProcessor::processRule: duration %lld ms >= required %lld ms for tag %s", 
                            duration_ms, required_ms, point_id.c_str());
                        return true;
                    }
                } else {
                    // 首次满足条件，记录激活时间
                    long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
                    alarm_status->second[rule.id] = AlarmServer::AlarmState(false, current_time);
                    alarm_status->second[rule.id].activation_time = current_time;
                }
            } else {
                // 首次满足条件，创建状态
                long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
                alarm_status->second[rule.id] = AlarmServer::AlarmState(false, current_time);
                alarm_status->second[rule.id].activation_time = current_time;
            }
        } else {
            // 首次满足条件，创建状态
            long long current_time = Poco::Timestamp().epochMicroseconds() / 1000;
            alarm_states_[point_id][rule.id] = AlarmServer::AlarmState(false, current_time);
            alarm_states_[point_id][rule.id].activation_time = current_time;
        }
    } else {
        // 条件不满足，重置状态
        auto& alarm_states_ = server->getAlarmStates();
        auto alarm_status = alarm_states_.find(point_id);
        if (alarm_status != alarm_states_.end()) {
            auto rule_state = alarm_status->second.find(rule.id);
            if (rule_state != alarm_status->second.end()) {
                rule_state->second.active = false;
            }
        }
    }
    return false;
}

// 偏差规则处理器
bool DeviationRuleProcessor::processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) {
    float value_f = 0.0f;
    try {
        value_f = std::stof(point_value);
    } catch (const std::exception &e) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "DeviationRuleProcessor::processRule: can not convert tag %s value %s to float",
            point_id.c_str(), point_value.c_str());
        return false;
    }
    
    float deviation = std::abs(value_f - rule.param3);
    if (deviation > rule.threshold) {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "DeviationRuleProcessor::processRule: deviation %f > threshold %f for tag %s", 
            deviation, rule.threshold, point_id.c_str());
        return true;
    }
    return false;
}
