/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: vsoa_handler.h .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "platform_sdk/server.h"
#include "history_service.h"

class VSOAHandler {
public:
    VSOAHandler();
    ~VSOAHandler();

    // 初始化VSOA handler
    bool Initialize(vsoa_sdk::server::ServerHandle* server);

    // 处理查询请求
    static int HandleQueryRequest(const char* path, const char* request, size_t request_len, 
                                  char** response, size_t* response_len, void* user_data);

    // 处理统计请求
    static int HandleStatsRequest(const char* path, const char* request, size_t request_len, 
                                  char** response, size_t* response_len, void* user_data);

    // 处理配置请求
    static int HandleConfigRequest(const char* path, const char* request, size_t request_len, 
                                   char** response, size_t* response_len, void* user_data);

    // 处理清理请求
    static int HandleCleanupRequest(const char* path, const char* request, size_t request_len, 
                                    char** response, size_t* response_len, void* user_data);

private:
    vsoa_sdk::server::ServerHandle* server_;
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_;

    // 注册RPC方法
    bool RegisterMethods();

    // 解析查询参数
    bool ParseQueryParams(const std::string& request, HistoryQueryParams& params);

    // 解析配置参数
    bool ParseConfigParams(const std::string& request, HistoryConfig& config);

    // 构建成功响应
    std::string BuildSuccessResponse(const std::string& data);

    // 构建错误响应
    std::string BuildErrorResponse(int code, const std::string& message);
};
