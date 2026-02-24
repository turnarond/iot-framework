/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/PointDto.hpp
 * @Description: 点位DTO定义，用于点位实体的数据传输
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef POINT_DTO_HPP
#define POINT_DTO_HPP

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * 点位实体DTO
 * 用于点位信息的传输和展示
 */
class PointDto : public vsoa::DTO {
    DTO_INIT(PointDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 点位ID
    DTO_FIELD(String, name, "name");                  ///< 点位名称
    DTO_FIELD(String, address, "address");               ///< 点位地址
    DTO_FIELD(UInt32, device_id, "device_id");              ///< 设备ID
    DTO_FIELD(Int32, datatype, "datatype");               ///< 数据类型
    DTO_FIELD(Int32, point_type, "point_type");             ///< 点位类型
    DTO_FIELD(Int32, transfer_type, "transfer_type");          ///< 转换类型
    DTO_FIELD(Float32, linear_raw_min, "linear_raw_min");       ///< 线性转换原始最小值
    DTO_FIELD(Float32, linear_raw_max, "linear_raw_max");       ///< 线性转换原始最大值
    DTO_FIELD(Float32, linear_eng_min, "linear_eng_min");       ///< 线性转换工程最小值
    DTO_FIELD(Float32, linear_eng_max, "linear_eng_max");       ///< 线性转换工程最大值
    DTO_FIELD(String, advanced_algo_lib, "advanced_algo_lib");     ///< 高级算法库
    DTO_FIELD(String, advanced_param1, "advanced_param1");       ///< 高级参数1
    DTO_FIELD(String, advanced_param2, "advanced_param2");       ///< 高级参数2
    DTO_FIELD(String, advanced_param3, "advanced_param3");       ///< 高级参数3
    DTO_FIELD(String, advanced_param4, "advanced_param4");       ///< 高级参数4
    DTO_FIELD(Boolean, enable_control, "enable_control");       ///< 是否启用控制
    DTO_FIELD(Boolean, enable_history, "enable_history");       ///< 是否启用历史记录
    DTO_FIELD(Int32, poll_rate, "poll_rate");              ///< 轮询速率
    DTO_FIELD(String, description, "description");           ///< 点位描述
};

/**
 * 点位分页DTO
 * 用于分页展示点位列表
 */
class PointPageDto : public PageDto<vsoa::Object<PointDto>> {
    DTO_INIT(PointPageDto, PageDto<vsoa::Object<PointDto>>)
};

/**
 * 点位创建DTO
 * 用于创建新点位时的数据传输
 */
class PointCreateDto : public vsoa::DTO {
    DTO_INIT(PointCreateDto, DTO)

    DTO_FIELD(String, name, "name");                        ///< 点位名称
    DTO_FIELD(String, address, "address");                  ///< 点位地址
    DTO_FIELD(Int32, device_id, "device_id");               ///< 设备ID
    DTO_FIELD(Int32, datatype, "datatype");                 ///< 数据类型
    DTO_FIELD(Int32, point_type, "point_type");             ///< 点位类型
    DTO_FIELD(Int32, transfer_type, "transfer_type");       ///< 转换类型
    DTO_FIELD(Float32, linear_raw_min, "linear_raw_min");    ///< 线性转换原始最小值
    DTO_FIELD(Float32, linear_raw_max, "linear_raw_max");    ///< 线性转换原始最大值
    DTO_FIELD(Float32, linear_eng_min, "linear_eng_min");    ///< 线性转换工程最小值
    DTO_FIELD(Float32, linear_eng_max, "linear_eng_max");    ///< 线性转换工程最大值
    DTO_FIELD(String, advanced_algo_lib, "advanced_algo_lib"); ///< 高级算法库
    DTO_FIELD(String, advanced_param1, "advanced_param1");   ///< 高级参数1
    DTO_FIELD(String, advanced_param2, "advanced_param2");   ///< 高级参数2
    DTO_FIELD(String, advanced_param3, "advanced_param3");   ///< 高级参数3
    DTO_FIELD(String, advanced_param4, "advanced_param4");   ///< 高级参数4
    DTO_FIELD(Boolean, enable_control, "enable_control");    ///< 是否启用控制
    DTO_FIELD(Boolean, enable_history, "enable_history");    ///< 是否启用历史记录
    DTO_FIELD(Int32, poll_rate, "poll_rate");               ///< 轮询速率
    DTO_FIELD(String, description, "description");          ///< 点位描述
};

/**
 * 带设备信息的点位DTO
 * 用于同时展示点位和关联的设备信息
 */
class PointWithDeviceDto : public vsoa::DTO {
    DTO_INIT(PointWithDeviceDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 点位ID
    DTO_FIELD(String, name, "name");                  ///< 点位名称
    DTO_FIELD(String, address, "address");               ///< 点位地址
    DTO_FIELD(UInt32, device_id, "device_id");             ///< 设备ID
    DTO_FIELD(String, device_name, "device_name");           ///< 设备名称
    DTO_FIELD(Int32, datatype_id, "datatype_id");                 ///< 数据类型ID
    DTO_FIELD(String, datatype_cname, "datatype_cname");         ///< 数据类型名称
    DTO_FIELD(UInt32, point_type_id, "point_type_id");             ///< 点位类型ID
    DTO_FIELD(String, point_type_cname, "point_type_cname"); ///< 点位类型名称
    DTO_FIELD(Boolean, enable_control, "enable_control");       ///< 是否启用控制
    DTO_FIELD(Boolean, enable_history, "enable_history");       ///< 是否启用历史记录
    DTO_FIELD(String, description, "description");           ///< 点位描述
};

/**
 * 带设备信息的点位分页DTO
 * 用于分页展示点位列表，同时包含关联的设备信息
 */
class PointWithDevicePageDto : public PageDto<vsoa::Object<PointWithDeviceDto>> {
    DTO_INIT(PointWithDevicePageDto, PageDto<vsoa::Object<PointWithDeviceDto>>)
};
/**
 * 点位值DTO
 * 用于点位实时值的传输和展示
 */
class PointValueDto : public vsoa::DTO {
    DTO_INIT(PointValueDto, DTO)

    DTO_FIELD(String, name, "name");                  ///< 点位名称
    DTO_FIELD(String, value, "value");                 ///< 点位值
    DTO_FIELD(Int64, timestamp, "timestamp");              ///< 时间戳
    DTO_FIELD(Boolean, quality, "quality");              ///< 数据质量
};

/**
 * 点位写入DTO
 * 用于写入点位值时的数据传输
 */
class PointWriteDto : public vsoa::DTO {
    DTO_INIT(PointWriteDto, DTO)

    DTO_FIELD(String, value, "value");        ///< 要写入的点位值
};

#include VSOA_CODEGEN_END(DTO)

#endif // POINT_DTO_HPP
