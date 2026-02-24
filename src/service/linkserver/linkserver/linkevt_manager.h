/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:00:55
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 11:56:40
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/linkevt_manager.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "link_define.h"
#include "util/plugin_interface.h"
#include <unordered_map>
#include <string>

typedef IEventSource* (*CreateEventSource)();
typedef void (*DestroyEventSource)(IEventSource*);

typedef struct _TriggerWithLibFunc TriggerWithLibFunc;

class CLinkEvtManager
{
public:
    static CLinkEvtManager* GetInstance();

    // 加载并启动所有触发器（来自 RuleEngine）
    int loadAndStartAllTriggers();

    // 停止并释放所有触发器
    void stopAllTriggers();

private:
    CLinkEvtManager() = default;
    ~CLinkEvtManager() = default;
    CLinkEvtManager(const CLinkEvtManager&) = delete;

    std::unordered_map<CTrigger*, TriggerWithLibFunc> loadedLibs_; // trigger* -> lib func
    bool isInitialized_ = false;
};

#define LINKEVT_MANAGER CLinkEvtManager::GetInstance()

