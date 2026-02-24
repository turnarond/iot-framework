/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 12:12:42
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 16:22:09
 * @FilePath: /acu/driver-sdk/src/service/linkserver/util/plugin_interface.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#ifndef __PLUGIN_INTERFACE_H__
#define __PLUGIN_INTERFACE_H__

#include <string>
#include <functional>

#include "lwlog/lwlog.h"

// 事件回调：事件名 + 负载（JSON 字符串）
using EventCallback = std::function<void(int triggerid, const std::string& payload)>;

// =============== 事件源插件接口 ===============
class IEventSource {
public:
    virtual ~IEventSource() = default;

    // 初始化：传入 JSON 配置（包含 param1~param4，但不包含 trigger_id）
    // trigger_id 由外部通过回调闭包传入，插件无需知道
    virtual bool init(const std::string& configJson) = 0;

    // 启动监听，注册回调
    // 回调函数由外部提供，自动绑定 trigger_id
    virtual bool start(const EventCallback& cb) = 0;

    // 停止监听
    virtual void stop() = 0;

    virtual void setLogger(CLWLog* logger) { logger_ = logger; }
protected:
    CLWLog* logger_ = nullptr;
};

// =============== 动作插件接口 ===============
class IAction {
public:
    virtual ~IAction() = default;

    // 初始化：传入 JSON 配置（param1~param4）
    virtual bool init(const std::string& configJson) = 0;

    // 执行动作，payload 为事件携带的数据（JSON）
    virtual bool execute(const std::string& payload) = 0;

    virtual void setLogger(CLWLog* logger) { logger_ = logger; }
protected:
    CLWLog* logger_ = nullptr;
};

// =============== 插件工厂函数（必须导出）===============
extern "C" {
    // 必须用 C 链接，避免 name mangling
    IEventSource* create_event_source();
    void destroy_event_source(IEventSource* p);

    IAction* create_action();
    void destroy_action(IAction* p);
}

#endif