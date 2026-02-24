/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:00:30
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 10:55:39
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/linkact_manager.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#pragma once

#include "link_define.h"
#include <unordered_map>

typedef IAction* (*CreateAction)();
typedef void (*DestroyAction)(IAction*);

typedef struct _ActionWithLibFunc ActionWithLibFunc;

class CLinkActManager {
public:
    static CLinkActManager* GetInstance();

    // 执行指定动作
    bool executeAction(const std::string& actionName, const std::string& payload);

    // 加载所有动作插件（来自 RuleEngine）
    int loadAllActions();

    void unLoadAction(const std::string& actionName);
private:
    CLinkActManager() = default;
    ~CLinkActManager() = default;
    CLinkActManager(const CLinkActManager&) = delete;

    IAction* loadPlugin(const std::string& libPath);

    std::unordered_map<std::string, IAction*> actionMap_; // name_ -> instance
    std::unordered_map<std::string, ActionWithLibFunc> loadedLibs_;
    bool isInitialized_ = false;
};

#define LINKACT_MANAGER (CLinkActManager::GetInstance())