/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/ErrorHandler.hpp
 * @Description: Error Handler for Web Server
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "dto/StatusDto.hpp"

#include "oatpp/web/server/handler/ErrorHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"

/**
 *  Custom Error Handler
 */
class ErrorHandler : public vsoa::web::server::handler::ErrorHandler {
private:
  std::shared_ptr<vsoa::data::mapping::ObjectMapper> m_objectMapper;
public:
  
  /**
   *  Constructor
   *  @param objectMapper - ObjectMapper to serialize ErrorInfo DTO
   */
  ErrorHandler(const std::shared_ptr<vsoa::data::mapping::ObjectMapper>& objectMapper);
  
  /**
   *  Handle error and return Error Response
   */
  std::shared_ptr<vsoa::web::protocol::http::outgoing::Response> handleError(const vsoa::web::protocol::http::Status& status, const vsoa::String& message, const Headers& headers) override;
  
};

#endif /* ERROR_HANDLER_HPP */
