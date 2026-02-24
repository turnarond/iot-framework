/*
 * @Author: yanchaodong
 * @Date: 2026-02-09 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-09 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/nodeserver/dto/HmiPointDto.hpp
 * @Description: HMI点位DTO定义，用于HMI接口的数据传输
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef HMI_POINT_DTO_HPP
#define HMI_POINT_DTO_HPP

#include "vsoa_dto/core/macro/codegen.hpp"
#include "vsoa_dto/core/Types.hpp"

#include VSOA_CODEGEN_BEGIN(DTO) //<- Begin Codegen

/**
 * 点位值DTO
 * 用于点位实时值的传输和展示
 */
class HmiPointValueDto : public vsoa::DTO {
    DTO_INIT(HmiPointValueDto, DTO)

    DTO_FIELD(vsoa::String, value, "value");        ///< 点位值
    DTO_FIELD(vsoa::String, quality, "quality");      ///< 数据质量
    DTO_FIELD(vsoa::Int64, ts, "ts");            ///< 时间戳
};

/**
 * 批量点位查询请求DTO
 * 用于批量查询点位值的请求
 */
class HmiBatchPointsRequestDto : public vsoa::DTO {
    DTO_INIT(HmiBatchPointsRequestDto, DTO)

    DTO_FIELD(vsoa::Vector<vsoa::String>, pointIds, "pointIds"); ///< 点位ID列表
};

/**
 * 批量点位查询响应DTO
 * 用于批量查询点位值的响应
 */
class HmiBatchPointsResponseDto : public vsoa::DTO {
    DTO_INIT(HmiBatchPointsResponseDto, DTO)

    DTO_FIELD(vsoa::Vector<vsoa::Object<HmiPointValueDto>>, points, "points"); ///< 点位值映射
};

/**
 * 控制命令DTO
 * 用于下发控制命令
 */
class HmiControlCommandDto : public vsoa::DTO {
    DTO_INIT(HmiControlCommandDto, DTO)

    DTO_FIELD(vsoa::String, pointId, "pointId");      ///< 点位ID
    DTO_FIELD(vsoa::String, value, "value");          ///< 控制值
};

/**
 * 控制命令响应DTO
 * 用于控制命令下发的响应
 */
class HmiControlResponseDto : public vsoa::DTO {
    DTO_INIT(HmiControlResponseDto, DTO)

    DTO_FIELD(vsoa::Boolean, success, "success");      ///< 控制是否成功
    DTO_FIELD(vsoa::String, message, "message");       ///< 响应消息
};

#include VSOA_CODEGEN_END(DTO) //<- End Codegen

#endif // HMI_POINT_DTO_HPP