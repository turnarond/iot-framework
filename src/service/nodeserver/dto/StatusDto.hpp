/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-27 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/StatusDto.hpp
 * @Description: Status DTO定义，用于API响应状态的传输和展示
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef CRUD_STATUSDTO_HPP
#define CRUD_STATUSDTO_HPP

#include "vsoa_dto/core/macro/codegen.hpp"
#include "vsoa_dto/core/Types.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * @brief Status DTO
 * 
 * @param status Status
 * @param data Response data
 * @param trace_id Request trace ID
 */
class StatusDto : public vsoa::DTO
{ 
    /**
     * @brief Status
     */
    DTO_INIT(StatusDto, DTO);
    DTO_FIELD(vsoa::Int32, code, "code"); ///< HTTP状态码
    DTO_FIELD(vsoa::String, message, "message"); ///< 状态消息
    DTO_FIELD(vsoa::Int64, timestamp, "timestamp"); ///< 响应时间戳
    DTO_FIELD(vsoa::String, trace_id, "trace_id"); ///< 请求跟踪ID

    DTO_FIELD_INFO(trace_id) {
        info->description = "Request trace ID";
        info->required = false;
    }
};

#include VSOA_CODEGEN_END(DTO)

#endif