/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/App.cpp
 * @Description: Application Main Entry
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "AppComponent.hpp"

#include "controller/DriverController.hpp"
#include "controller/DeviceController.hpp"
#include "controller/PointController.hpp"
#include "controller/AlarmController.hpp"
#include "controller/LinkageController.hpp"
#include "controller/DictController.hpp"
#include "controller/UserController.hpp"
#include "controller/VideoController.hpp"
// #include "controller/SystemConfigController.hpp"

#include "controller/StaticController.hpp"
#include "oatpp-swagger/Controller.hpp"

#include "oatpp/network/Server.hpp"

#include "lwlog/lwlog.h"

#include <iostream>

CLWLog g_logger;

void run()
{
  AppComponent components; // Create scope Environment components
  
  /* Get router component */
  VSOA_COMPONENT(std::shared_ptr<vsoa::web::server::HttpRouter>, router);

  vsoa::web::server::api::Endpoints docEndpoints;

  // Add controllers
  docEndpoints.append(router->addController(DriverController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(DeviceController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(PointController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(AlarmController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(LinkageController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(DictController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(UserController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(VideoController::createShared())->getEndpoints());

  // docEndpoints.append(router->addController(SystemConfigController::createShared())->getEndpoints());

  // Add Swagger controller
  router->addController(vsoa::swagger::Controller::createShared(docEndpoints));
  router->addController(StaticController::createShared());

  /* Get connection handler component */
  VSOA_COMPONENT(std::shared_ptr<vsoa::network::ConnectionHandler>, connectionHandler);

  /* Get connection provider component */
  VSOA_COMPONENT(std::shared_ptr<vsoa::network::ServerConnectionProvider>, connectionProvider);

  /* create server */
  vsoa::network::Server server(connectionProvider,
                                connectionHandler);
  
  g_logger.LogMessage(LW_LOGLEVEL_INFO, "Server Running on port %s...", 
    connectionProvider->getProperty("port").toString()->c_str());
  
  server.run();

  /* Get database connection provider and stop it */
  VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, dbConnectionProvider);
  dbConnectionProvider->stop();
}


/**
 *  main
 */
int main(int argc, const char * argv[])
{
  vsoa::base::Environment::init();

  g_logger.SetLogFileName();

  run();
  
  /* Print how many objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << vsoa::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << vsoa::base::Environment::getObjectsCreated() << "\n\n";
  
  vsoa::base::Environment::destroy();
  
  return 0;
}

