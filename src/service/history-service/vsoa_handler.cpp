/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: vsoa_handler.cpp .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "vsoa_handler.h"
#include "platform_sdk/json.h"
#include <iostream>

VSOAHandler::VSOAHandler() : server_(nullptr) {
    // 初始化JSON对象映射器
    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::Builder().Build();
}

VSOAHandler::~VSOAHandler() {
}

bool VSOAHandler::Initialize(vsoa_sdk::server::ServerHandle* server) {
    server_ = server;
    return RegisterMethods();
}

bool VSOAHandler::RegisterMethods() {
    if (!server_) {
        return false;
    }

    // 注册查询方法
    int ret = server_->AddMethod("history.query", HandleQueryRequest, this);
    if (ret != VSOA_OK) {
        std::cerr << "Failed to register history.query method" << std::endl;
        return false;
    }

    // 注册统计方法
    ret = server_->AddMethod("history.stats", HandleStatsRequest, this);
    if (ret != VSOA_OK) {
        std::cerr << "Failed to register history.stats method" << std::endl;
        return false;
    }

    // 注册配置方法
    ret = server_->AddMethod("history.config", HandleConfigRequest, this);
    if (ret != VSOA_OK) {
        std::cerr << "Failed to register history.config method" << std::endl;
        return false;
    }

    // 注册清理方法
    ret = server_->AddMethod("history.cleanup", HandleCleanupRequest, this);
    if (ret != VSOA_OK) {
        std::cerr << "Failed to register history.cleanup method" << std::endl;
        return false;
    }

    std::cout << "VSOA methods registered successfully" << std::endl;
    return true;
}

int VSOAHandler::HandleQueryRequest(const char* path, const char* request, size_t request_len, 
                                  char** response, size_t* response_len, void* user_data) {
    VSOAHandler* handler = static_cast<VSOAHandler*>(user_data);
    std::string request_str(request, request_len);

    // 解析查询参数
    HistoryQueryParams params;
    if (!handler->ParseQueryParams(request_str, params)) {
        std::string error_response = handler->BuildErrorResponse(400, "Invalid query parameters");
        *response = strdup(error_response.c_str());
        *response_len = error_response.size();
        return VSOA_OK;
    }

    // 执行查询
    std::vector<HistoryRecord> records = HISTORY_SERVICE->Query(params);

    // 构建响应数据
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < records.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{";
        ss << "\"timestamp\":\"" << records[i].timestamp << "\",";
        ss << "\"tag_name\":\"" << records[i].tag_name << "\",";
        ss << "\"value\":\"" << records[i].value << "\",";
        ss << "\"driver_name\":\"" << records[i].driver_name << "\",";
        ss << "\"device_name\":\"" << records[i].device_name << "\"";
        ss << "}";
    }
    ss << "]";

    std::string success_response = handler->BuildSuccessResponse(ss.str());
    *response = strdup(success_response.c_str());
    *response_len = success_response.size();

    return VSOA_OK;
}

int VSOAHandler::HandleStatsRequest(const char* path, const char* request, size_t request_len, 
                                  char** response, size_t* response_len, void* user_data) {
    VSOAHandler* handler = static_cast<VSOAHandler*>(user_data);

    // 获取统计信息
    HistoryStats stats = HISTORY_SERVICE->GetStats();

    // 构建响应数据
    std::stringstream ss;
    ss << ":";
    ss << "{";
    ss << "\"total_records\":" << stats.total_records << ",";
    ss << "\"total_size\":" << stats.total_size << ",";
    ss << "\"today_records\":" << stats.today_records << ",";
    ss << "\"today_size\":" << stats.today_size << ",";
    ss << "\"oldest_date\":\"" << stats.oldest_date << "\",";
    ss << "\"newest_date\":\"" << stats.newest_date << "\"";
    ss << "}";

    std::string success_response = handler->BuildSuccessResponse(ss.str());
    *response = strdup(success_response.c_str());
    *response_len = success_response.size();

    return VSOA_OK;
}

int VSOAHandler::HandleConfigRequest(const char* path, const char* request, size_t request_len, 
                                   char** response, size_t* response_len, void* user_data) {
    VSOAHandler* handler = static_cast<VSOAHandler*>(user_data);
    std::string request_str(request, request_len);

    // 解析配置参数
    HistoryConfig config;
    if (!handler->ParseConfigParams(request_str, config)) {
        // 如果解析失败，返回当前配置
        const HistoryConfig& current_config = HISTORY_SERVICE->GetConfig();
        std::stringstream ss;
        ss << ":";
        ss << "{";
        ss << "\"storage_path\":\"" << current_config.storage_path << "\",";
        ss << "\"max_days\":" << current_config.max_days << ",";
        ss << "\"batch_size\":" << current_config.batch_size << ",";
        ss << "\"flush_interval\":" << current_config.flush_interval << ",";
        ss << "\"max_file_size\":" << current_config.max_file_size << "";
        ss << "}";

        std::string success_response = handler->BuildSuccessResponse(ss.str());
        *response = strdup(success_response.c_str());
        *response_len = success_response.size();
        return VSOA_OK;
    }

    // 更新配置
    if (!HISTORY_SERVICE->UpdateConfig(config)) {
        std::string error_response = handler->BuildErrorResponse(500, "Failed to update config");
        *response = strdup(error_response.c_str());
        *response_len = error_response.size();
        return VSOA_OK;
    }

    // 返回更新后的配置
    const HistoryConfig& updated_config = HISTORY_SERVICE->GetConfig();
    std::stringstream ss;
    ss << ":";
    ss << "{";
    ss << "\"storage_path\":\"" << updated_config.storage_path << "\",";
    ss << "\"max_days\":" << updated_config.max_days << ",";
    ss << "\"batch_size\":" << updated_config.batch_size << ",";
    ss << "\"flush_interval\":" << updated_config.flush_interval << ",";
    ss << "\"max_file_size\":" << updated_config.max_file_size << "";
    ss << "}";

    std::string success_response = handler->BuildSuccessResponse(ss.str());
    *response = strdup(success_response.c_str());
    *response_len = success_response.size();

    return VSOA_OK;
}

int VSOAHandler::HandleCleanupRequest(const char* path, const char* request, size_t request_len, 
                                    char** response, size_t* response_len, void* user_data) {
    VSOAHandler* handler = static_cast<VSOAHandler*>(user_data);

    // 执行清理
    size_t deleted = HISTORY_SERVICE->Cleanup();

    // 构建响应数据
    std::stringstream ss;
    ss << ":";
    ss << "{";
    ss << "\"deleted_files\":" << deleted << "";
    ss << "}";

    std::string success_response = handler->BuildSuccessResponse(ss.str());
    *response = strdup(success_response.c_str());
    *response_len = success_response.size();

    return VSOA_OK;
}

bool VSOAHandler::ParseQueryParams(const std::string& request, HistoryQueryParams& params) {
    try {
        auto json = vsoa::parser::json::Json::Parse(request);
        if (json.IsObject()) {
            if (json.HasMember("tag_name") && json["tag_name"].IsString()) {
                params.tag_name = json["tag_name"].GetString();
            }
            if (json.HasMember("start_time") && json["start_time"].IsString()) {
                params.start_time = json["start_time"].GetString();
            }
            if (json.HasMember("end_time") && json["end_time"].IsString()) {
                params.end_time = json["end_time"].GetString();
            }
            if (json.HasMember("limit") && json["limit"].IsNumber()) {
                params.limit = static_cast<size_t>(json["limit"].GetInt64());
            } else {
                params.limit = 1000;
            }
            if (json.HasMember("desc") && json["desc"].IsBool()) {
                params.desc = json["desc"].GetBool();
            } else {
                params.desc = true;
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse query params: " << e.what() << std::endl;
        return false;
    }
}

bool VSOAHandler::ParseConfigParams(const std::string& request, HistoryConfig& config) {
    try {
        auto json = vsoa::parser::json::Json::Parse(request);
        if (json.IsObject()) {
            if (json.HasMember("storage_path") && json["storage_path"].IsString()) {
                config.storage_path = json["storage_path"].GetString();
            }
            if (json.HasMember("max_days") && json["max_days"].IsNumber()) {
                config.max_days = static_cast<size_t>(json["max_days"].GetInt64());
            }
            if (json.HasMember("batch_size") && json["batch_size"].IsNumber()) {
                config.batch_size = static_cast<size_t>(json["batch_size"].GetInt64());
            }
            if (json.HasMember("flush_interval") && json["flush_interval"].IsNumber()) {
                config.flush_interval = static_cast<size_t>(json["flush_interval"].GetInt64());
            }
            if (json.HasMember("max_file_size") && json["max_file_size"].IsNumber()) {
                config.max_file_size = static_cast<size_t>(json["max_file_size"].GetInt64());
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config params: " << e.what() << std::endl;
        return false;
    }
}

std::string VSOAHandler::BuildSuccessResponse(const std::string& data) {
    return "{\"code\":0,\"message\":\"success\",\"data\"" + data + "}";
}

std::string VSOAHandler::BuildErrorResponse(int code, const std::string& message) {
    std::stringstream ss;
    ss << "{";
    ss << "\"code\":" << code << ",";
    ss << "\"message\":\"" << message << "\",";
    ss << "\"data\":null";
    ss << "}";
    return ss.str();
}
