/*
 * @Author: yanchaodong
 * @Date: 2025-10-30 11:22:23
 * @LastEditors: yanchaodong
 * @LastEditTime: 2025-11-05 14:36:10
 * @FilePath: /acu/driver-sdk/src/service/linkserver/linkserver/main.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ACOINFO, All Rights Reserved. 
 */

#include "rule_engine.h"
#include "linkact_manager.h"
#include "linkevt_manager.h"

#include "lwlog/lwlog.h"
#include "lwcomm/lwcomm.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Data/SQLite/Connector.h"
#include "platform_sdk/init.h"

const std::string DB_FILENAME = "lw_monitor.db";
CLWLog g_lwlog;

class LinkServerApp : public Poco::Util::ServerApplication
{
protected:
    int main(const std::vector<std::string>& args)
    {
        g_lwlog.SetLogFileName();
        vsoa_sdk::init();
        // 1. RuleEngine 加载规则
        std::string db_path = LWComm::GetConfigPath();
        db_path = db_path + LW_OS_DIR_SEPARATOR + DB_FILENAME;
        RULE_ENGINE->loadRules(db_path);

        // 2. 加载并启动所有动作
        LINKACT_MANAGER->loadAllActions();

        // 3. 加载并启动所有触发器
        LINKEVT_MANAGER->loadAndStartAllTriggers();

        // 4. 等待退出信号（POCO ServerApplication）
        waitForTerminationRequest();

        // 5. 清理
        LINKEVT_MANAGER->stopAllTriggers();
        // LinkActManager 析构时自动释放

        return 0;
    }
};

POCO_SERVER_MAIN(LinkServerApp)