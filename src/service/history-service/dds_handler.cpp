/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: dds_handler.cpp .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "dds_handler.h"
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

DDSHandler::DDSHandler() : 
    publisher_(nullptr),
    subscriber_(nullptr),
    running_(false) {
}

DDSHandler::~DDSHandler() {
    Stop();
}

bool DDSHandler::Initialize() {
    // 创建发布者
    publisher_ = lwdistcomm_dds_simple_publisher_create("history_response");
    if (!publisher_) {
        std::cerr << "Failed to create DDS publisher" << std::endl;
        return false;
    }

    // 创建订阅者
    subscriber_ = lwdistcomm_dds_simple_subscriber_create("history_request");
    if (!subscriber_) {
        std::cerr << "Failed to create DDS subscriber" << std::endl;
        if (publisher_) {
            lwdistcomm_dds_simple_publisher_destroy(publisher_);
            publisher_ = nullptr;
        }
        return false;
    }

    std::cout << "DDS handler initialized successfully" << std::endl;
    return true;
}

bool DDSHandler::Start() {
    if (running_) {
        return true;
    }

    if (!publisher_ || !subscriber_) {
        if (!Initialize()) {
            return false;
        }
    }

    // 注册订阅回调
    if (lwdistcomm_dds_simple_subscriber_set_callback(subscriber_, HandleQueryRequest, this) != LWDISTCOMM_OK) {
        std::cerr << "Failed to set DDS subscriber callback" << std::endl;
        return false;
    }

    running_ = true;
    std::cout << "DDS handler started successfully" << std::endl;
    return true;
}

bool DDSHandler::Stop() {
    if (!running_) {
        return true;
    }

    running_ = false;

    // 销毁发布者和订阅者
    if (publisher_) {
        lwdistcomm_dds_simple_publisher_destroy(publisher_);
        publisher_ = nullptr;
    }

    if (subscriber_) {
        lwdistcomm_dds_simple_subscriber_destroy(subscriber_);
        subscriber_ = nullptr;
    }

    std::cout << "DDS handler stopped successfully" << std::endl;
    return true;
}

void DDSHandler::HandleQueryRequest(const char* data, size_t data_len, void* user_data) {
    DDSHandler* handler = static_cast<DDSHandler*>(user_data);
    std::string message(data, data_len);

    // 解析消息
    std::string action, payload;
    if (!handler->ParseDDSMessage(message, action, payload)) {
        std::cerr << "Failed to parse DDS message" << std::endl;
        return;
    }

    if (action == "query") {
        // 解析查询参数
        try {
            auto json = nlohmann::json::parse(payload);
            HistoryQueryParams params;

            if (json.contains("tag_name")) {
                params.tag_name = json["tag_name"];
            }
            if (json.contains("start_time")) {
                params.start_time = json["start_time"];
            }
            if (json.contains("end_time")) {
                params.end_time = json["end_time"];
            }
            if (json.contains("limit")) {
                params.limit = json["limit"];
            } else {
                params.limit = 1000;
            }
            if (json.contains("desc")) {
                params.desc = json["desc"];
            } else {
                params.desc = true;
            }

            // 执行查询
            std::vector<HistoryRecord> records = HISTORY_SERVICE->Query(params);

            // 构建响应数据
            nlohmann::json response_data = nlohmann::json::array();
            for (const auto& record : records) {
                nlohmann::json record_json;
                record_json["timestamp"] = record.timestamp;
                record_json["tag_name"] = record.tag_name;
                record_json["value"] = record.value;
                record_json["driver_name"] = record.driver_name;
                record_json["device_name"] = record.device_name;
                response_data.push_back(record_json);
            }

            // 发布响应
            std::string response = handler->BuildDDSResponse("query", response_data.dump());
            handler->PublishResponse("history_response", response);

        } catch (const std::exception& e) {
            std::cerr << "Failed to handle query request: " << e.what() << std::endl;
            std::string error_response = handler->BuildDDSResponse("query", "", false, e.what());
            handler->PublishResponse("history_response", error_response);
        }
    } else if (action == "stats") {
        // 处理统计请求
        try {
            HistoryStats stats = HISTORY_SERVICE->GetStats();

            // 构建响应数据
            nlohmann::json response_data;
            response_data["total_records"] = stats.total_records;
            response_data["total_size"] = stats.total_size;
            response_data["today_records"] = stats.today_records;
            response_data["today_size"] = stats.today_size;
            response_data["oldest_date"] = stats.oldest_date;
            response_data["newest_date"] = stats.newest_date;

            // 发布响应
            std::string response = handler->BuildDDSResponse("stats", response_data.dump());
            handler->PublishResponse("history_response", response);

        } catch (const std::exception& e) {
            std::cerr << "Failed to handle stats request: " << e.what() << std::endl;
            std::string error_response = handler->BuildDDSResponse("stats", "", false, e.what());
            handler->PublishResponse("history_response", error_response);
        }
    }
}

void DDSHandler::HandleStatsRequest(const char* data, size_t data_len, void* user_data) {
    // 这个方法可以留作扩展，目前主要通过HandleQueryRequest处理
    HandleQueryRequest(data, data_len, user_data);
}

bool DDSHandler::PublishResponse(const std::string& topic, const std::string& response) {
    if (!publisher_) {
        return false;
    }

    return lwdistcomm_dds_simple_publish(publisher_, response.c_str(), response.size()) == LWDISTCOMM_OK;
}

bool DDSHandler::ParseDDSMessage(const std::string& message, std::string& action, std::string& payload) {
    try {
        auto json = nlohmann::json::parse(message);
        if (json.contains("action")) {
            action = json["action"];
        }
        if (json.contains("payload")) {
            payload = json["payload"];
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse DDS message: " << e.what() << std::endl;
        return false;
    }
}

std::string DDSHandler::BuildDDSResponse(const std::string& action, const std::string& data, bool success, const std::string& error) {
    nlohmann::json response;
    response["action"] = action;
    response["success"] = success;
    if (success) {
        response["data"] = nlohmann::json::parse(data);
    } else {
        response["error"] = error;
    }
    return response.dump();
}
