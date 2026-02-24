/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: main.cpp .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "base/base.h"
#include "cfgm/config_manager.h"
#include "logm/log_manager.h"
#include "regm/registry_manager.h"
#include "resm/resource_monitor.h"
#include "security/security_context.h"

#include "lwlog/lwlog.h"
#include "lwcomm/lwcomm.h"

#include "platform_sdk/init.h"

CLWLog g_logger;

using namespace edge::core;

void initSignalHandlers()
{

}
int main(int argc, char** argv) 
{
    // 1. 基础初始化
    vsoa_sdk::init();
    g_logger.SetLogFileName();
    
    initSignalHandlers();

    // 2. 配置模块初始化
    if (!CONFIG_MANAGER->Init()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize configuration manager.");
        return -1;
    }
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Configuration manager initialized.");
    
    // 3. 初始化日志模块 (最先初始化，以便记录其他模块的初始化过程)
    if (!LOG_MANAGER->Init()) {
        // 日志初始化失败，使用备用日志机制
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize log manager");
        return EXIT_FAILURE;
    }
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Edge-Core starting up...");
    
    // 4. 初始化资源监控模块
    if (!RESOURCE_MONITOR->Init()) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Failed to initialize resource monitor, continuing without it");
    } else {
        RESOURCE_MONITOR->StartMonitoring();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Resource monitoring started");
    }

    // 6. 初始化注册模块
    if (!REGISTRY_MANAGER->Init()) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize registry manager");
        // 继续启动，但标记为未注册状态
    } else {
        // 异步进行设备注册
        std::thread registerThread([]() {
            g_logger.LogMessage(LW_LOGLEVEL_INFO, "Starting device registration...");
            while (true) {
                if (REGISTRY_MANAGER->CheckRegister()) {
                    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Device registered successfully");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                } else {
                    g_logger.LogMessage(LW_LOGLEVEL_ERROR,"Device registration failed");
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                }
            }
        });
        registerThread.detach();
    }
    
    // 7. 启动主服务 (IPC/VSOA)
    // if (!startMainService()) {
    //     g_logger.LogMessage(LW_LOGLEVEL_ERROR,"Failed to start main service");
    //     return EXIT_FAILURE;
    // }
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Edge-Core started successfully");
    
    // // 8. 主循环
    // runMainLoop();
    while(1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // // 9. 清理资源
    // cleanup();
    
    return EXIT_SUCCESS;
}