/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: base.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#ifndef _EDGE_CORE_BASE_BASE_H_
#define _EDGE_CORE_BASE_BASE_H_ 

#include "version.h"
#include "base_err.h"
#include <bits/types/time_t.h>
#include <cstdint>
#include <string>
#include <chrono>

// 模块ID枚举 (用于通知特定模块)
enum class ModuleId {
    REGM,           // 注册管理模块
    RESM,           // 资源管理模块
    LOGM,           // 日志管理模块
    TOKEN_SERVICE,  // 令牌服务
    FIRMWARE_MGR,   // 固件管理
    NETWORK_MGR,    // 网络管理
    SYSTEM          // 系统模块
};

// 服务凭证
struct ServiceCredential {
    std::string serviceBrokerEndpoint; // 服务broker地址
    uint16_t serviceBrokerPort;         // 服务broker端口
    std::string deviceCert;           // 设备证书
    std::string caCert;               // CA证书
    uint64_t issuedAt;    // 签发时间
    uint64_t expiresAt;   // 过期时间
    
    bool isValid() const {
        return !serviceBrokerEndpoint.empty() && 
               !deviceCert.empty() &&
               !caCert.empty() &&
               time(NULL) < expiresAt;
    }
    
    bool isAboutToExpire() const {
        auto timeToExpiry = expiresAt - time(NULL);   
        return timeToExpiry < 24 * 60 * 60; // 24小时内过期
    }
};

#endif