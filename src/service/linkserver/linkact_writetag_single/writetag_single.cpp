/*
 * @Author: yanchaodong
 * @Date: 2025-10-31 11:41:17
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 16:23:05
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkact_writetag_single/writetag_single.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "util/plugin_interface.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "lwlog/lwlog.h"  // 假设你可以访问 lwlog，否则用 std::cout

class WriteTagSingleAction : public IAction {
public:
    WriteTagSingleAction() = default;
    ~WriteTagSingleAction() override = default;

    bool init(const std::string& configJson) override 
    {
        // 解析 JSON 配置：提取 param1 (tag), param2 (value)
        try {
            // 简易 JSON 解析（生产环境建议用 Poco::JSON 或 nlohmann/json）
            // 这里为简化，假设格式为 {"param1":"...", "param2":"..."}
            size_t p1_start = configJson.find("\"param1\":\"");
            if (p1_start != std::string::npos) {
                size_t val_start = p1_start + 10;
                size_t val_end = configJson.find("\"", val_start);
                if (val_end != std::string::npos) {
                    tag_name_ = configJson.substr(val_start, val_end - val_start);
                }
            }

            size_t p2_start = configJson.find("\"param2\":\"");
            if (p2_start != std::string::npos) {
                size_t val_start = p2_start + 10;
                size_t val_end = configJson.find("\"", val_start);
                if (val_end != std::string::npos) {
                    write_value_ = configJson.substr(val_start, val_end - val_start);
                }
            }

            if (tag_name_.empty()) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "WriteTagSingle: param1 (tag name) is empty");
                return false;
            }

            logger_->LogMessage(LW_LOGLEVEL_INFO,
                "WriteTagSingleAction initialized: tag='%s', value='%s'",
                tag_name_.c_str(), write_value_.c_str());
            return true;
        } catch (...) {
            logger_->LogMessage(LW_LOGLEVEL_ERROR, "WriteTagSingle: failed to parse config JSON");
            return false;
        }
    }

    bool execute(const std::string& payload) override 
    {
        // payload 可选：可用于覆盖写入值（例如 payload = {"value": "100"}）
        std::string final_value = write_value_;

        // 简单判断 payload 是否包含新值（可选增强）
        if (!payload.empty()) {
            // 示例：如果 payload 是 {"value":"xxx"}，则提取
            size_t val_pos = payload.find("\"value\":\"");
            if (val_pos != std::string::npos) {
                size_t start = val_pos + 9;
                size_t end = payload.find("\"", start);
                if (end != std::string::npos) {
                    final_value = payload.substr(start, end - start);
                }
            }
        }

        // 🔧 实际写入逻辑（此处为模拟）
        // 例如：CDriverManager::GetInstance()->WriteTag(tag_name_, final_value);
        bool success = simulateWriteTag(tag_name_, final_value);

        if (success) {
            logger_->LogMessage(LW_LOGLEVEL_INFO,
                "WriteTagSingle: successfully wrote '%s' to tag '%s'",
                final_value.c_str(), tag_name_.c_str());
        } else {
            logger_->LogMessage(LW_LOGLEVEL_ERROR,
                "WriteTagSingle: failed to write '%s' to tag '%s'",
                final_value.c_str(), tag_name_.c_str());
        }

        return success;
    }

private:
    std::string tag_name_;
    std::string write_value_;

    // 模拟写入函数（替换为真实逻辑）
    bool simulateWriteTag(const std::string& tag, const std::string& value) {
        // TODO: 替换为实际写入驱动、OPC UA、Modbus 等
        // 临时返回 true 模拟成功
        return true;
    }
};

// =============== 插件导出函数 ===============
extern "C" {

IAction* create_action() {
    try {
        return new WriteTagSingleAction();
    } catch (...) {
        return nullptr;
    }
}

void destroy_action(IAction* p) {
    delete p;
}

} // extern "C"