/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/DriverDto.hpp
 * @Description: 驱动DTO定义，用于驱动实体的数据传输
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DRIVER_DTO_HPP
#define DRIVER_DTO_HPP

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * 驱动实体DTO
 * 用于驱动信息的传输和展示
 */
class DriverDto : public vsoa::DTO {
    DTO_INIT(DriverDto, DTO)

    DTO_FIELD(vsoa::UInt32, id, "id");             ///< 驱动ID
    DTO_FIELD(vsoa::UInt32, type, "type");           ///< 驱动类型
    DTO_FIELD(vsoa::String, version, "version");        ///< 驱动版本
    DTO_FIELD(vsoa::String, name, "name");           ///< 驱动名称
    DTO_FIELD(vsoa::String, description, "description");    ///< 驱动描述
    DTO_FIELD(vsoa::String, param1_name, "param1_name");    ///< 参数1名称
    DTO_FIELD(vsoa::String, param1_desc, "param1_desc");    ///< 参数1描述
    DTO_FIELD(vsoa::String, param2_name, "param2_name");    ///< 参数2名称
    DTO_FIELD(vsoa::String, param2_desc, "param2_desc");    ///< 参数2描述
    DTO_FIELD(vsoa::String, param3_name, "param3_name");    ///< 参数3名称
    DTO_FIELD(vsoa::String, param3_desc, "param3_desc");    ///< 参数3描述
    DTO_FIELD(vsoa::String, param4_name, "param4_name");    ///< 参数4名称
    DTO_FIELD(vsoa::String, param4_desc, "param4_desc");    ///< 参数4描述
    DTO_FIELD(vsoa::UInt64, create_time, "create_time");    ///< 创建时间戳
};

/**
 * 驱动创建DTO
 * 用于创建新驱动时的数据传输
 */
class DriverCreateDto : public vsoa::DTO {
    DTO_INIT(DriverCreateDto, DTO)

    DTO_FIELD(vsoa::String, name, "name");              ///< 驱动名称
    DTO_FIELD(vsoa::UInt32, type, "type");              ///< 驱动类型
    DTO_FIELD(vsoa::String, version, "version");        ///< 驱动版本
    DTO_FIELD(vsoa::String, description, "description"); ///< 驱动描述
    DTO_FIELD(vsoa::String, param1_name, "param1_name"); ///< 参数1名称
    DTO_FIELD(vsoa::String, param1_desc, "param1_desc"); ///< 参数1描述
    DTO_FIELD(vsoa::String, param2_name, "param2_name"); ///< 参数2名称
    DTO_FIELD(vsoa::String, param2_desc, "param2_desc"); ///< 参数2描述
    DTO_FIELD(vsoa::String, param3_name, "param3_name"); ///< 参数3名称
    DTO_FIELD(vsoa::String, param3_desc, "param3_desc"); ///< 参数3描述
    DTO_FIELD(vsoa::String, param4_name, "param4_name"); ///< 参数4名称
    DTO_FIELD(vsoa::String, param4_desc, "param4_desc"); ///< 参数4描述
};

/**
 * 驱动分页DTO
 * 用于驱动列表的分页展示
 */
class DriversPageDto : public PageDto<vsoa::Object<DriverDto>> {
    DTO_INIT(DriversPageDto, PageDto<vsoa::Object<DriverDto>>)
};

#include VSOA_CODEGEN_END(DTO)

#endif // DRIVER_DTO_HPP
