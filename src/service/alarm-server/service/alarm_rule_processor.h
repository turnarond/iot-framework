/*
 * @Author: yanchaodong
 * @Date: 2026-02-22
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/service/alarm_rule_processor.h
 * @Description: 报警规则处理器接口
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "alarm_server.h"

class AlarmRuleProcessor {
public:
    virtual ~AlarmRuleProcessor() = default;
    
    // 处理报警规则
    virtual bool processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) = 0;
};

// 工厂类，用于创建不同类型的规则处理器
class AlarmRuleProcessorFactory {
public:
    static std::unique_ptr<AlarmRuleProcessor> createProcessor(int method);
};

// 具体的规则处理器实现
class ThresholdRuleProcessor : public AlarmRuleProcessor {
private:
    bool isHighThreshold_;
public:
    ThresholdRuleProcessor(bool isHighThreshold) : isHighThreshold_(isHighThreshold) {}
    bool processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) override;
};

class FixedValueRuleProcessor : public AlarmRuleProcessor {
public:
    bool processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) override;
};

class RateOfChangeRuleProcessor : public AlarmRuleProcessor {
public:
    bool processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) override;
};

class DurationRuleProcessor : public AlarmRuleProcessor {
public:
    bool processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) override;
};

class DeviationRuleProcessor : public AlarmRuleProcessor {
public:
    bool processRule(const AlarmRule& rule, const std::string& point_value, const std::string& point_id, AlarmServer* server) override;
};
