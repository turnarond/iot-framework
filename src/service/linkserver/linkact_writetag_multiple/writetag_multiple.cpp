/*
 * @Author: yanchaodong
 * @Date: 2025-10-31 16:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 16:34:35
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkact_writetag_multiple/writetag_multiple.cpp
 * @Description: 批量写入标签插件
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "util/plugin_interface.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "lwlog/lwlog.h"

// 写入项结构
struct WriteItem {
    std::string tag;
    std::string value;
};

class WriteTagMultipleAction : public IAction {
public:
    WriteTagMultipleAction() = default;
    ~WriteTagMultipleAction() override = default;

    bool init(const std::string& configJson) override {

        // 示例期望的 configJson 格式：
        // {"param1":"[{\"tag\":\"T1\",\"value\":\"100\"},{\"tag\":\"T2\",\"value\":\"200\"}]"}
        try {
            // 简易提取 param1（应为 JSON 数组字符串）
            size_t p1_start = configJson.find("\"param1\":\"");
            if (p1_start == std::string::npos) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "WriteTagMultiple: missing param1");
                return false;
            }

            size_t val_start = p1_start + 10;
            size_t val_end = configJson.find("\"", val_start);
            if (val_end == std::string::npos) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "WriteTagMultiple: invalid param1 format");
                return false;
            }

            std::string itemsJson = configJson.substr(val_start, val_end - val_start);

            // 🔧 简易解析 JSON 数组（生产建议用 Poco::JSON）
            // 假设格式：[{"tag":"T1","value":"V1"},{"tag":"T2","value":"V2"}]
            parseWriteItems(itemsJson);

            if (write_items_.empty()) {
                logger_->LogMessage(LW_LOGLEVEL_ERROR, "WriteTagMultiple: no valid write items found");
                return false;
            }

            std::ostringstream oss;
            for (const auto& item : write_items_) {
                oss << "(" << item.tag << "=" << item.value << ") ";
            }
            logger_->LogMessage(LW_LOGLEVEL_INFO,
                "WriteTagMultipleAction initialized with %zu items: %s",
                write_items_.size(), oss.str().c_str());

            return true;
        } catch (...) {
            logger_->LogMessage(LW_LOGLEVEL_ERROR, "WriteTagMultiple: exception during init");
            return false;
        }
    }

    bool execute(const std::string& payload) override {
        // payload 可选：用于动态覆盖值，格式如：
        // {"overrides": {"T1": "150", "T2": "250"}}
        std::unordered_map<std::string, std::string> overrides;
        if (!payload.empty()) {
            parseOverrides(payload, overrides);
        }

        bool all_success = true;
        for (auto& item : write_items_) {
            std::string final_value = item.value;
            if (!overrides.empty()) {
                auto it = overrides.find(item.tag);
                if (it != overrides.end()) {
                    final_value = it->second;
                }
            }

            bool success = simulateWriteTag(item.tag, final_value);
            if (!success) {
                all_success = false;
                logger_->LogMessage(LW_LOGLEVEL_ERROR,
                    "WriteTagMultiple: failed to write tag '%s' with value '%s'",
                    item.tag.c_str(), final_value.c_str());
            } else {
                logger_->LogMessage(LW_LOGLEVEL_DEBUG,
                    "WriteTagMultiple: wrote tag '%s' = '%s'",
                    item.tag.c_str(), final_value.c_str());
            }
        }

        if (all_success) {
            logger_->LogMessage(LW_LOGLEVEL_INFO,
                "WriteTagMultiple: all %zu tags written successfully",
                write_items_.size());
        }

        return all_success;
    }

private:
    std::vector<WriteItem> write_items_;

    // 简易解析 [{"tag":"xxx","value":"yyy"},...]
    void parseWriteItems(const std::string& json) {
        write_items_.clear();
        size_t pos = 0;
        while ((pos = json.find("\"tag\":\"", pos)) != std::string::npos) {
            size_t tag_start = pos + 7;
            size_t tag_end = json.find("\"", tag_start);
            if (tag_end == std::string::npos) break;

            std::string tag = json.substr(tag_start, tag_end - tag_start);

            size_t val_pos = json.find("\"value\":\"", tag_end);
            if (val_pos == std::string::npos) break;

            size_t val_start = val_pos + 9;
            size_t val_end = json.find("\"", val_start);
            if (val_end == std::string::npos) break;

            std::string value = json.substr(val_start, val_end - val_start);

            write_items_.push_back({tag, value});
            pos = val_end;
        }
    }

    // 解析 payload 中的 overrides: {"overrides": {"T1":"100"}}
    void parseOverrides(const std::string& payload, std::unordered_map<std::string, std::string>& out) {
        size_t overrides_start = payload.find("\"overrides\":{");
        if (overrides_start == std::string::npos) return;

        size_t content_start = overrides_start + 13;
        size_t content_end = payload.find("}", content_start);
        if (content_end == std::string::npos) return;

        std::string inner = payload.substr(content_start, content_end - content_start);
        size_t pos = 0;
        while ((pos = inner.find("\"", pos)) != std::string::npos) {
            size_t key_start = pos + 1;
            size_t key_end = inner.find("\"", key_start);
            if (key_end == std::string::npos) break;

            std::string key = inner.substr(key_start, key_end - key_start);
            size_t colon = inner.find(":", key_end);
            if (colon == std::string::npos) break;

            size_t val_start = inner.find("\"", colon);
            if (val_start == std::string::npos) break;
            val_start++; // skip quote
            size_t val_end = inner.find("\"", val_start);
            if (val_end == std::string::npos) break;

            std::string val = inner.substr(val_start, val_end - val_start);
            out[key] = val;

            pos = val_end + 1;
        }
    }

    // 模拟写入（替换为真实驱动调用）
    bool simulateWriteTag(const std::string& tag, const std::string& value) {
        // TODO: 替换为实际写入逻辑，如：
        // return CDriverManager::GetInstance()->WriteTag(tag, value);
        return true; // 模拟成功
    }
};

// =============== 插件导出函数 ===============
extern "C" {

IAction* create_action() {
    try {
        return new WriteTagMultipleAction();
    } catch (...) {
        return nullptr;
    }
}

void destroy_action(IAction* p) {
    delete p;
}

} // extern "C"