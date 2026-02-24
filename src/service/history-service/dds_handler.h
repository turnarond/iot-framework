/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: dds_handler.h .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "comms/lwdistcomm/include/dds/dds_simple.h"
#include "history_service.h"

class DDSHandler {
public:
    DDSHandler();
    ~DDSHandler();

    // 初始化DDS handler
    bool Initialize();

    // 启动DDS handler
    bool Start();

    // 停止DDS handler
    bool Stop();

private:
    lwdistcomm_dds_simple_publisher_t* publisher_;
    lwdistcomm_dds_simple_subscriber_t* subscriber_;
    std::atomic<bool> running_;

    // 处理查询请求
    static void HandleQueryRequest(const char* data, size_t data_len, void* user_data);

    // 处理统计请求
    static void HandleStatsRequest(const char* data, size_t data_len, void* user_data);

    // 发布响应
    bool PublishResponse(const std::string& topic, const std::string& response);

    // 解析DDS消息
    bool ParseDDSMessage(const std::string& message, std::string& action, std::string& payload);

    // 构建DDS响应
    std::string BuildDDSResponse(const std::string& action, const std::string& data, bool success = true, const std::string& error = "");
};
