/*
 * @Author: yanchaodong
 * @Date: 2026-02-15 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-15 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/server_mgr/src/dto/StatusDto.hpp
 * @Description: Status DTO for Server Manager
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef STATUS_DTO_HPP
#define STATUS_DTO_HPP

#include "vsoa_dto/core/macro/codegen.hpp"
#include "vsoa_dto/core/Types.hpp"

#include VSOA_CODEGEN_BEGIN(DTO) //<- Begin DTO Codegen

/**
 * 状态响应DTO
 */
class StatusDto : public vsoa::DTO {
  DTO_INIT(StatusDto, DTO);
  
  DTO_FIELD(vsoa::Int32, code, "code"); ///< 状态码
  DTO_FIELD(vsoa::String, message, "message"); ///< 状态消息
  
  DTO_FIELD_INFO(code) {
    info->description = "状态码";
  }
  
  DTO_FIELD_INFO(message) {
    info->description = "状态消息";
  }
};

#include VSOA_CODEGEN_END(DTO) //<- End DTO Codegen

#endif /* STATUS_DTO_HPP */
