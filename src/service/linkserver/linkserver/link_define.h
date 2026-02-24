/*
 * @Author: yanchaodong
 * @Date: 2025-10-29 11:42:47
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 11:56:31
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/link_define.h
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */


#ifndef __LINK_DEFINE_H__
#define __LINK_DEFINE_H__

#include <string>
#include <vector>
#include <memory>
#include "util/plugin_interface.h"

class IEventSource;
class IAction;
    
// 触发器(事件源)
class CTrigger 
{
public:
    int trigger_id_ = 0;
    std::string trigger_name_;
    std::string eventtype_name_;
    std::string trigger_desc_;
    std::string source_;
    std::string param1_, param2_, param3_, param4_; // 插件初始化参数

    // 运行时字段（非数据库）
    IEventSource* pInstance_ = nullptr;
    bool isRunning_ = false;

public:
    CTrigger() = default;
    CTrigger(const CTrigger &) = delete;
    ~CTrigger() = default;
};

class CLinkAction 
{
public:
    std::vector<int> trigger_ids_; // 哪些触发器会触发此动作（多对多）

    int aciont_id_ = 0;  // 动作类型ID
    std::string name_; // 动作名称
    std::string desc_; // 动作描述
    std::string param1_, param2_, param3_, param4_;
    std::string plugin_name_;
    IAction* pInstance_ = nullptr;
    uint64_t trigger_time_milli_; // 触发时间
    uint64_t end_time_milli_; // 结束时间

public:
    CLinkAction() = default;
    CLinkAction(const CLinkAction &) = delete;
    ~CLinkAction() = default;
};

// 规则绑定：事件名 → 动作列表
struct RuleBinding {
    std::string event_name;
    std::vector<std::string> action_names; // 动作 name_ 列表
};

#endif