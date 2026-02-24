/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: resource_monitor.cpp .
 *
 * Date: 2025-12-18
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "resource_monitor.h"
#include "lwlog/lwlog.h"

using namespace edge::core;

extern CLWLog g_logger;

ResourceMonitor ResourceMonitor::instance_;
ResourceMonitor *ResourceMonitor::GetInstance()
{
    return &instance_;
}

bool ResourceMonitor::Init()
{
    return true;
}

// 开始监控
void ResourceMonitor::StartMonitoring()
{

}

// 停止监控
void ResourceMonitor::StopMonitoring()
{

}
