/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:01:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 17:02:05
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/linkevt_manager.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "linkevt_manager.h"
#include "Poco/SharedLibrary.h"
#include "rule_engine.h"
#include "util/plugin_interface.h"

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Stringifier.h"

#include "lwlog/lwlog.h"

static CLinkEvtManager* g_instance = nullptr;
extern CLWLog g_lwlog;

struct _TriggerWithLibFunc {
    IEventSource* pInstance;
    std::shared_ptr<Poco::SharedLibrary> lib;
    std::string libpath;
    CreateEventSource create;
    DestroyEventSource destroy;
};

CLinkEvtManager* CLinkEvtManager::GetInstance() {
    if (!g_instance) g_instance = new CLinkEvtManager();
    return g_instance;
}
int CLinkEvtManager::loadAndStartAllTriggers() 
{
    for (CTrigger* trigger : RULE_ENGINE->getRequiredTriggers()) {

        auto load_lib_path = std::find_if(loadedLibs_.begin(), loadedLibs_.end(), 
            [&](const std::pair<CTrigger*, TriggerWithLibFunc>& pair) {
                return pair.second.libpath == trigger->source_;
            }
        );
        if (load_lib_path != loadedLibs_.end()) {
            // 已加载，但 POCO SharedLibrary 不支持重复 getSymbol
            // 更安全做法：每次重新加载（或缓存 SharedLibrary 对象）
            g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Started trigger: %s (ID=%d) from plugin: %s success.", 
                trigger->trigger_name_.c_str(), trigger->trigger_id_, load_lib_path->second.libpath.c_str());
            continue;
        }

        TriggerWithLibFunc lib_func;
        try {
            lib_func.lib = std::make_shared<Poco::SharedLibrary>(trigger->source_);
            lib_func.libpath = trigger->source_;
            lib_func.create = (CreateEventSource)lib_func.lib->getSymbol("create_event_source");
            lib_func.destroy = (DestroyEventSource)lib_func.lib->getSymbol("destroy_event_source");
            loadedLibs_[trigger] = lib_func;
        } catch (Poco::Exception& e) {
            // log error
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "load plugin %s failed: %s", 
                trigger->source_.c_str(), e.displayText().c_str());
            continue;
        }
        
        if (!lib_func.create) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "create_event_source not found in %s", 
                trigger->source_.c_str());
            continue;
        }

        lib_func.pInstance = lib_func.create();
        if (!lib_func.pInstance) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "create_event_source failed in %s", 
                trigger->source_.c_str());
            continue;
        }
        // 构造 Poco::JSON 配置
        Poco::JSON::Object::Ptr config = new Poco::JSON::Object();
        if (!trigger->param1_.empty()) config->set("param1", trigger->param1_);
        if (!trigger->param2_.empty()) config->set("param2", trigger->param2_);
        if (!trigger->param3_.empty()) config->set("param3", trigger->param3_);
        if (!trigger->param4_.empty()) config->set("param4", trigger->param4_);

        std::ostringstream oss;
        Poco::JSON::Stringifier::stringify(config, oss);
        std::string configJson = oss.str();

        lib_func.pInstance->setLogger(&g_lwlog);
        if (!lib_func.pInstance->init(configJson)) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Plugin init failed: %s", 
                trigger->trigger_name_.c_str());
            lib_func.destroy(lib_func.pInstance);
            continue;
        }

        auto callback = [](int trigger_id, const std::string& payload) {
            g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Received event from trigger (ID=%d), payload: %s", 
                trigger_id, payload.c_str());
            RULE_ENGINE->OnEvent(trigger_id, payload);
        };

        if (!lib_func.pInstance->start(callback)) {
            g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "Plugin start failed: %s", 
                trigger->trigger_name_.c_str());
            lib_func.destroy(lib_func.pInstance);
            continue;
        }

        trigger->pInstance_ = lib_func.pInstance;
        trigger->isRunning_ = true;
        g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Started trigger: %s (ID=%d) from plugin: %s success.", 
            trigger->trigger_name_.c_str(), trigger->trigger_id_, lib_func.libpath.c_str());
    }
    return 0;
}

void CLinkEvtManager::stopAllTriggers() 
{
    // 卸载 .so（可选，POCO 会在析构时 unload）
    for (auto& kv : loadedLibs_) {
        kv.second.destroy(kv.second.pInstance);
    }
    loadedLibs_.clear();
}