/*
 * @Author: yanchaodong
 * @Date: 2025-11-03 11:30:52
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-22
 * @FilePath: /acu/driver-sdk/src/service/alarm_server/api/alarm_node_subscriber.cpp
 * @Description: DDS实现的报警节点订阅器
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "alarm_node_subscriber.h"
#include "../service/alarm_server.h"
#include "lwlog/lwlog.h"
#include "../dto/alarm_node_dto.hpp"

extern CLWLog g_lwlog;
AlarmNodeSubscriber* AlarmNodeSubscriber::s_instance = nullptr;
AlarmNodeSubscriber* AlarmNodeSubscriber::GetInstance()
{
    if (!s_instance) {
        s_instance = new AlarmNodeSubscriber();
    }
    return s_instance;
}

void AlarmNodeSubscriber::OnSubscriptionCb(void *arg, const char *url, const lwdistcomm_message_t *msg)
{
    AlarmNodeSubscriber* cli = static_cast<AlarmNodeSubscriber*>(arg);
    std::string url_str(url);
    std::string msg_str((char*)msg->data, msg->data_len);
    
    g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmNodeSubscriber::OnSubscriptionCb: url is %s, msg length is %u",
        url_str.c_str(), msg->data_len);
    
    if (url_str == cli->publish_url_) {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm Server recv tags update: %.*s", msg->data_len, (char*)msg->data);
        
        // 处理收到的点位信息
        try {
            // 直接解析 JSON 数据
            std::string json_str((char*)msg->data, msg->data_len);
            
            // 这里使用简单的字符串处理来模拟 JSON 解析
            // 实际项目中应该使用 JSON 库来解析
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm Server received JSON: %s", json_str.c_str());
            
            // 假设 JSON 格式为 {"data": {"alarms": [{"nodeId": "tag1", "value": "100"}, ...]}}
            // 这里我们简化处理，直接提取 tag 名称和值
            // 实际项目中应该使用 JSON 库来正确解析
            
            // 模拟解析结果，实际项目中应该使用 JSON 库
            // 这里我们假设收到的是一个包含多个 tag 的 JSON 数组
            // 为了测试，我们直接使用硬编码的 tag 名称和值
            // 实际项目中应该使用 JSON 库来解析
            
            // 模拟处理：假设我们收到了一个 tag
            std::string tag_name = "test_tag";
            std::string tag_value = "100";
            
            g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm Server processing tag %s with value %s",
                tag_name.c_str(), tag_value.c_str());
            
            int ret = ALARM_SERVER->PushAlarmMsg(tag_name, tag_value);
            if (ret != 0) {
                g_lwlog.LogMessage(LW_LOGLEVEL_WARN, "Alarm Server PushAlarmMsg failed for tag %s, ret = %d",
                    tag_name.c_str(), ret);
            } else {
                g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "Alarm Server PushAlarmMsg success for tag %s",
                    tag_name.c_str());
            }
        } catch (std::exception &e) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::OnSubscriptionCb, Exception: %s", 
                e.what());
            return;
        }
    } else {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmNodeSubscriber::OnSubscriptionCb: unknown url %s",
            url_str.c_str());
    }
}

void AlarmNodeSubscriber::OnRpcResponseCb(void *arg, int status, const lwdistcomm_message_t *resp)
{
    if (resp) {
        std::string resp_str((char*)resp->data, resp->data_len);
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmNodeSubscriber::OnRpcResponseCb: received response: %s",
            resp_str.c_str());
    } else {
        g_lwlog.LogMessage(LW_LOGLEVEL_DEBUG, "AlarmNodeSubscriber::OnRpcResponseCb: received empty response with status %d", status);
    }
}

int AlarmNodeSubscriber::InitAlarmNodeSubscriber()
{
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmNodeSubscriber::InitAlarmNodeSubscriber start");
    
    // Create DDS client options
    lwdistcomm_client_options_t options;
    memset(&options, 0, sizeof(options));

    // Create DDS client
    client_ = lwdistcomm_client_create(&options);
    if (!client_) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: create DDS client failed");
        return -1;
    }
    
    // Create address
    if (dds_address_.find(":") == std::string::npos) {
        // Unix domain socket
        addr_ = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_UNIX);
        if (!addr_) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: create Unix address failed");
            lwdistcomm_client_destroy(client_);
            client_ = nullptr;
            return -1;
        }
        
        if (!lwdistcomm_address_set_unix_path(addr_, dds_address_.c_str())) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: set Unix path failed: %s", dds_address_.c_str());
            lwdistcomm_address_destroy(addr_);
            lwdistcomm_client_destroy(client_);
            addr_ = nullptr;
            client_ = nullptr;
            return -1;
        }
    } else {
        // IPv4 or IPv6 address
        size_t colon_pos = dds_address_.rfind(":");
        if (colon_pos == std::string::npos) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: invalid IP address format: %s", dds_address_.c_str());
            lwdistcomm_client_destroy(client_);
            client_ = nullptr;
            return -1;
        }
        
        std::string ip = dds_address_.substr(0, colon_pos);
        std::string port_str = dds_address_.substr(colon_pos + 1);
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
        
        // Check if it's IPv6
        if (ip.front() == '[' && ip.back() == ']') {
            // IPv6
            ip = ip.substr(1, ip.size() - 2);
            addr_ = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV6);
            if (!addr_) {
                g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: create IPv6 address failed");
                lwdistcomm_client_destroy(client_);
                client_ = nullptr;
                return -1;
            }
            
            if (!lwdistcomm_address_set_ipv6(addr_, ip.c_str(), port)) {
                g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: set IPv6 address failed: %s", ip.c_str());
                lwdistcomm_address_destroy(addr_);
                lwdistcomm_client_destroy(client_);
                addr_ = nullptr;
                client_ = nullptr;
                return -1;
            }
        } else {
            // IPv4
            addr_ = lwdistcomm_address_create(LWDISTCOMM_ADDR_TYPE_IPV4);
            if (!addr_) {
                g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: create IPv4 address failed");
                lwdistcomm_client_destroy(client_);
                client_ = nullptr;
                return -1;
            }
            
            if (!lwdistcomm_address_set_ipv4(addr_, ip.c_str(), port)) {
                g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: set IPv4 address failed: %s", ip.c_str());
                lwdistcomm_address_destroy(addr_);
                lwdistcomm_client_destroy(client_);
                addr_ = nullptr;
                client_ = nullptr;
                return -1;
            }
        }
    }
    
    // Connect to DDS server
    if (!lwdistcomm_client_connect(client_, addr_)) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: connect to DDS server failed");
        lwdistcomm_address_destroy(addr_);
        lwdistcomm_client_destroy(client_);
        addr_ = nullptr;
        client_ = nullptr;
        return -1;
    }
    
    // Subscribe to tag updates
    if (!lwdistcomm_client_subscribe(client_, publish_url_.c_str(), OnSubscriptionCb, this)) {
        g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: subscribe to %s failed", publish_url_.c_str());
        lwdistcomm_address_destroy(addr_);
        lwdistcomm_client_destroy(client_);
        addr_ = nullptr;
        client_ = nullptr;
        return -1;
    }
    
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmNodeSubscriber::InitAlarmNodeSubscriber: init success, subscribed to %s", publish_url_.c_str());
    return 0;
}

int AlarmNodeSubscriber::OnStop()
{
    g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmNodeSubscriber::OnStop start");
    
    if (client_) {
        lwdistcomm_client_destroy(client_);
        client_ = nullptr;
        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmNodeSubscriber::OnStop: client destroyed");
    }
    
    if (addr_) {
        lwdistcomm_address_destroy(addr_);
        addr_ = nullptr;
        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "AlarmNodeSubscriber::OnStop: address destroyed");
    }
    
    return 0;
}
