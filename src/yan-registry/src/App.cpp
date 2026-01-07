
#include "AppComponent.hpp"

#include "controller/DeviceRepositoryController.hpp"
#include "controller/StaticController.hpp"

#include "oatpp-swagger/Controller.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>
#include <thread>

void run() {

  AppComponent components; // Create scope Environment components

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  oatpp::web::server::api::Endpoints docEndpoints;

  docEndpoints.append(
      router->addController(RegistrationController::createShared())
          ->getEndpoints());

  router->addController(oatpp::swagger::Controller::createShared(docEndpoints));
  router->addController(StaticController::createShared());

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                  connectionHandler);

  auto httpConnectionProvider =
      oatpp::network::tcp::server::ConnectionProvider::createShared(
          {"0.0.0.0", 8080});
  auto httpsConnectionProvider =
      oatpp::network::tcp::server::ConnectionProvider::createShared(
          {"0.0.0.0", 8443});

//   // 加载 TLS 证书（需提前生成）
//   auto config = oatpp::openssl::Config::createDefaultServerConfig("server.pem",
//                                                                   "server.key");
//   auto tlsConnectionProvider =
//       oatpp::openssl::server::ConnectionProvider::createShared(
//           config, httpsConnectionProvider);

  // 启动两个服务器
  std::thread httpThread([&] {
    oatpp::network::Server server(httpConnectionProvider, connectionHandler);
    server.run();
  });
//   std::thread httpsThread([&] {
//     oatpp::network::Server server(tlsConnectionProvider, connectionHandler);
//     server.run();
//   });

  /* stop db connection pool */
  OATPP_COMPONENT(
      std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>,
      dbConnectionProvider);
  dbConnectionProvider->stop();
}

/**
 *  main
 */
int main(int argc, const char *argv[]) {

  oatpp::base::Environment::init();

  run();

  /* Print how many objects were created during app running, and what have
   * left-probably leaked */
  /* Disable object counting for release builds using '-D
   * OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount()
            << "\n";
  std::cout << "objectsCreated = "
            << oatpp::base::Environment::getObjectsCreated() << "\n\n";

  oatpp::base::Environment::destroy();

  return 0;
}
