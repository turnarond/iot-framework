/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/SwaggerComponent.hpp
 * @Description: Swagger Component for Web Server
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef SWAGGER_COMPONENT_HPP
#define SWAGGER_COMPONENT_HPP

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"
#include "lwcomm/lwcomm.h"

/**
 *  Swagger component
 */
class SwaggerComponent {
public:
  
  /**
   *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::swagger::Resources>, swaggerResources)([] {
    std::string datapath = LWComm::GetDataPath();
    std::string oatpp_swagger_res_path = datapath + LW_OS_DIR_SEPARATOR + "oatpp-swagger/res";
    // Make sure to specify correct full path to oatpp-swagger/res folder !!!
    return vsoa::swagger::Resources::loadResources(oatpp_swagger_res_path);
  }());
  
  /**
   *  Swagger UI API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::swagger::DocumentInfo>, swaggerDocumentInfo)([] {
    
    /* Create Swagger Controller */
    vsoa::swagger::DocumentInfo::Builder builder;
    builder
    .setTitle("Edge Framework API")
    .setDescription("Edge Framework Web Server API Documentation")
    .setVersion("1.0")
    .setContactName("ACOINFO")
    .setContactUrl("https://oatpp.io/")
    
    .setLicenseName("Apache License, Version 2.0")
    .setLicenseUrl("http://www.apache.org/licenses/LICENSE-2.0")
    
    .addServer("http://10.13.104.225:8081", "server on 10.13.104.225")
    .addServer("http://127.0.0.1:8081", "server on 127.0.0.1");
    
    return builder.build();
  }());
  
};

#endif /* SWAGGER_COMPONENT_HPP */
