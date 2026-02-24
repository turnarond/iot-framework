/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/ErrorHandler.cpp
 * @Description: Error Handler Implementation for Web Server
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "ErrorHandler.hpp"
#include <chrono>

ErrorHandler::ErrorHandler(const std::shared_ptr<vsoa::data::mapping::ObjectMapper>& objectMapper)
  : m_objectMapper(objectMapper)
{}

std::shared_ptr<vsoa::web::protocol::http::outgoing::Response> ErrorHandler::handleError(const vsoa::web::protocol::http::Status& status, const vsoa::String& message, const Headers& headers)
{
  
  auto error = StatusDto::createShared();
  error->message = message;
  error->code = status.code;
  error->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  
  auto response = vsoa::web::protocol::http::outgoing::ResponseFactory::createResponse(status, error, m_objectMapper);
  
  for(const auto& pair : headers.getAll())
  {
    response->putHeader(pair.first.toString(), pair.second.toString());
  }
  
  return response;
}
