/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:00:36
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-10-31 17:02:17
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/linkact_manager.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "linkact_manager.h"
#include "Poco/SharedLibrary.h"
#include "lwcomm/lwcomm.h"
#include "rule_engine.h"

#include "lwlog/lwlog.h"
#include <Poco/Exception.h>

extern CLWLog g_lwlog;

static CLinkActManager* g_instance = nullptr;

struct _ActionWithLibFunc {
    IAction* pInstance;
    std::shared_ptr<Poco::SharedLibrary> lib;
    std::string libpath;
    CreateAction create;
    DestroyAction destroy;
};

CLinkActManager* CLinkActManager::GetInstance() {
    if (!g_instance) g_instance = new CLinkActManager();
    return g_instance;
}

int CLinkActManager::loadAllActions() 
{
    if (isInitialized_) return 0;

    auto& actions = CRuleEngine::GetInstance()->getRequiredActions();
    for (auto* act : actions) {
        auto load_lib_path = std::find_if(
            loadedLibs_.begin(), loadedLibs_.end(),
            [&](const auto& lib) {
                return lib.second.libpath == act->plugin_name_;
            }
        );
        if (load_lib_path == loadedLibs_.end()) {
            ActionWithLibFunc action_ist;
            try {
                // std::string path = LWComm::GetLibPath();
                // path = path + LW_OS_DIR_SEPARATOR + "linux/x86_64/plugins/lib" + act->plugin_name_ + ".so";
                action_ist.lib = std::make_shared<Poco::SharedLibrary>(act->plugin_name_, Poco::SharedLibrary::SHLIB_GLOBAL);
                action_ist.libpath = act->plugin_name_;
                action_ist.create = (CreateAction)action_ist.lib->getSymbol("create_action");
                action_ist.destroy = (DestroyAction)action_ist.lib->getSymbol("destroy_action");
                if (!action_ist.create || !action_ist.destroy) return -1;

                action_ist.pInstance = action_ist.create();
                if (!action_ist.pInstance) return -1;

                action_ist.pInstance->setLogger(&g_lwlog);
            } catch (Poco::Exception &e) {
                g_lwlog.LogMessage(LW_LOGLEVEL_ERROR, "load action %s, library %s failed, msg %s", 
                    act->name_.c_str(), act->plugin_name_.c_str(), e.displayText().c_str());
                return -1;
            }

            std::string config = "{...}"; // 构造 JSON
            if (action_ist.pInstance->init(config)) {
                // 保存到 CLinkAction::pInstance_（可选）
                act->pInstance_ = action_ist.pInstance;
            }
            loadedLibs_[act->name_] = action_ist;
            g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Action %s load successfully, plugin is %s.", 
                act->name_.c_str(), act->plugin_name_.c_str());
        } else {
            g_lwlog.LogMessage(LW_LOGLEVEL_INFO, "Actions %s , plugin %s has loaded  successfully.", 
                act->name_.c_str(), load_lib_path->second.libpath.c_str());
        }
    }
    isInitialized_ = true;
    return 0;
}

bool CLinkActManager::executeAction(const std::string& actionName, const std::string& payload) 
{
    auto it = loadedLibs_.find(actionName);
    if (it != loadedLibs_.end()) {
        try {
            return it->second.pInstance->execute(payload);
        } catch (...) {
            // log error
            return false;
        }
    }
    return false;
}

void CLinkActManager::unLoadAction(const std::string& actionName)
{
    auto it = loadedLibs_.find(actionName);
    if (it != loadedLibs_.end()) {
        try {
            it->second.destroy(it->second.pInstance);
        } catch (...) {
            // log error
        }
        loadedLibs_.erase(it);
    }
}