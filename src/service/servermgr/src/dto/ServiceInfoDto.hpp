/*
 * @Author: yanchaodong
 * @Date: 2026-02-15 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-15 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/server_mgr/src/dto/ServiceInfoDto.hpp
 * @Description: Service Info DTO for Server Manager
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef SERVICE_INFO_DTO_HPP
#define SERVICE_INFO_DTO_HPP

#include "vsoa_dto/core/macro/codegen.hpp"
#include "vsoa_dto/core/Types.hpp"

#include VSOA_CODEGEN_BEGIN(DTO) //<- Begin DTO Codegen

/**
 * 服务信息DTO
 */
class ServiceInfoDto : public vsoa::DTO {
  DTO_INIT(ServiceInfoDto, DTO);
  
  DTO_FIELD(vsoa::String, name, "name"); ///< 服务名称
  DTO_FIELD(vsoa::Boolean, running, "running"); ///< 运行状态
  DTO_FIELD(vsoa::String, status, "status"); ///< 状态描述
  DTO_FIELD(vsoa::String, startTime, "startTime"); ///< 启动时间
  
  DTO_FIELD_INFO(name) {
    info->description = "服务名称";
  }
  
  DTO_FIELD_INFO(running) {
    info->description = "运行状态";
  }
  
  DTO_FIELD_INFO(status) {
    info->description = "状态描述";
  }
  
  DTO_FIELD_INFO(startTime) {
    info->description = "启动时间";
  }
};

#include VSOA_CODEGEN_END(DTO) //<- End DTO Codegen

#endif /* SERVICE_INFO_DTO_HPP */