/*
 * @Author: yanchaodong
 * @Date: 2026-02-15 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-15 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/server_mgr/src/App.cpp
 * @Description: Application Main Entry
 *
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved.
 */

#include "AppComponent.hpp"
#include "controller/ServiceManagerController.hpp"
#include "lwcomm/lwcomm.h"
#include "lwlog/lwlog.h"
#include "oatpp/network/Server.hpp"
#include "servermgr.h"

CLWLog g_logger;

void run(ServerManager &serverManager)
{
    AppComponent components; // Create scope Environment components

    /* Get router component */
    VSOA_COMPONENT(std::shared_ptr<vsoa::web::server::HttpRouter>, router);

    // Add controllers
    VSOA_COMPONENT(std::shared_ptr<vsoa::data::mapping::ObjectMapper>,
                   objectMapper);
    router->addController(
        ServiceManagerController::createShared(objectMapper, serverManager));

    /* Get connection handler component */
    VSOA_COMPONENT(std::shared_ptr<vsoa::network::ConnectionHandler>,
                   connectionHandler);

    /* Get connection provider component */
    VSOA_COMPONENT(std::shared_ptr<vsoa::network::ServerConnectionProvider>,
                   connectionProvider);

    /* create server */
    vsoa::network::Server server(connectionProvider, connectionHandler);

    server.run();
}

/**
 *  main
 */
int main(int argc, const char *argv[]) 
{

  vsoa::base::Environment::init();
  g_logger.SetLogFileName();

  try {
    // Initialize server manager
    std::string executablePath = argv[0];
    ServerManager serverMgr(executablePath);
    std::string configDir = LWComm::GetConfigPath();
    configDir = configDir + LW_OS_DIR_SEPARATOR + "server_mgr";
    // Load configuration from directory (multiple files)
    if (!serverMgr.loadConfigDirectory(configDir)) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load configuration %s", configDir.c_str());
        return 1;
    }

    // Start all services
    if (!serverMgr.startAllServices()) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Some services failed to start");
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "All services started successfully");
    }

    // Run server
    run(serverMgr);

  } catch (const std::exception &e) {
    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Error: %s", e.what());
    return 1;
  }

  /* Print how many objects were created during app running, and what have
   * left-probably leaked */
  /* Disable object counting for release builds using '-D
   * OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  g_logger.LogMessage(LW_LOGLEVEL_INFO, "Environment:");
  g_logger.LogMessage(LW_LOGLEVEL_INFO, "ObjectsCount = %d", vsoa::base::Environment::getObjectsCount());
  g_logger.LogMessage(LW_LOGLEVEL_INFO, "ObjectsCreated = %d", vsoa::base::Environment::getObjectsCreated());

  vsoa::base::Environment::destroy();
  g_logger.LogMessage(LW_LOGLEVEL_INFO, "Server manager stopped");

  return 0;
}
