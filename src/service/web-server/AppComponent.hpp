/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/AppComponent.hpp
 * @Description: Application Component for Web Server
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef APP_COMPONENT_HPP
#define APP_COMPONENT_HPP

#include "SwaggerComponent.hpp"
#include "DatabaseComponent.hpp"

#include "ErrorHandler.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/interceptor/AllowCorsGlobal.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

/**
 *  Class which creates and holds Application components and registers components in vsoa::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent {
public:
  
  /**
   *  Swagger component
   */
  SwaggerComponent swaggerComponent;

  /**
   * Database component
   */
  DatabaseComponent databaseComponent;

  /**
   * Create ObjectMapper component to serialize/deserialize DTOs in Controller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;

    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = true;
    auto objectMapper = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
    return objectMapper;
  }());
  
  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::network::ServerConnectionProvider>, serverConnectionProvider)([] {
    return vsoa::network::tcp::server::ConnectionProvider::createShared(
      {"0.0.0.0", 8080, vsoa::network::Address::IP_4}
    );
  }());
  
  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::web::server::HttpRouter>, httpRouter)([] {
    return vsoa::web::server::HttpRouter::createShared();
  }());
  
  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::network::ConnectionHandler>, serverConnectionHandler)([] {

    VSOA_COMPONENT(std::shared_ptr<vsoa::web::server::HttpRouter>, router); // get Router component
    VSOA_COMPONENT(std::shared_ptr<vsoa::data::mapping::ObjectMapper>, objectMapper); // get ObjectMapper component

    auto connectionHandler = vsoa::web::server::HttpConnectionHandler::createShared(router);
    connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));
    
    // 添加CORS支持
    auto cors = std::make_shared<vsoa::web::server::interceptor::AllowCorsGlobal>(
      "*",
      "GET, POST, PUT, DELETE, OPTIONS",
      "DNT, User-Agent, X-Requested-With, If-Modified-Since, Cache-Control, Content-Type, Range, Authorization",
      "1728000"
    );
    connectionHandler->addResponseInterceptor(cors);
    
    auto options = std::make_shared<vsoa::web::server::interceptor::AllowOptionsGlobal>();
    connectionHandler->addRequestInterceptor(options);
    
    return connectionHandler;

  }());

};

#endif /* APP_COMPONENT_HPP */
