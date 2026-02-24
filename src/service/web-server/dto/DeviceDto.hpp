/*
 * @Author: yanchaodong
 * @Date: 2026-01-27 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/dto/DeviceDto.hpp
 * @Description: 设备DTO定义，用于设备实体的数据传输
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DEVICE_DTO_HPP
#define DEVICE_DTO_HPP

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * 设备实体DTO
 * 用于设备信息的传输和展示
 */
class DeviceDto : public vsoa::DTO {
    DTO_INIT(DeviceDto, DTO)

    DTO_FIELD(UInt32, id, "id");             ///< 设备ID
    DTO_FIELD(String, name, "name");           ///< 设备名称
    DTO_FIELD(UInt32, driver_id, "driver_id");       ///< 驱动ID
    DTO_FIELD(UInt32, conn_type, "conn_type");       ///< 连接类型
    DTO_FIELD(String, connparam, "connparam");      ///< 连接参数
    DTO_FIELD(String, description, "description");    ///< 设备描述
    DTO_FIELD(String, param1, "param1");         ///< 参数1值
    DTO_FIELD(String, param2, "param2");         ///< 参数2值
    DTO_FIELD(String, param3, "param3");         ///< 参数3值
    DTO_FIELD(String, param4, "param4");         ///< 参数4值
};

/**
 * 设备创建DTO
 * 用于创建新设备时的数据传输
 */
class DeviceCreateDto : public vsoa::DTO {
    DTO_INIT(DeviceCreateDto, DTO)

    DTO_FIELD(String, name, "name");               ///< 设备名称
    DTO_FIELD(UInt32, driver_id, "driver_id");      ///< 驱动ID
    DTO_FIELD(UInt32, conn_type, "conn_type");      ///< 连接类型
    DTO_FIELD(String, connparam, "connparam");     ///< 连接参数
    DTO_FIELD(String, description, "description"); ///< 设备描述
    DTO_FIELD(String, param1, "param1");           ///< 参数1值
    DTO_FIELD(String, param2, "param2");           ///< 参数2值
    DTO_FIELD(String, param3, "param3");           ///< 参数3值
    DTO_FIELD(String, param4, "param4");           ///< 参数4值
};

/**
 * 设备更新DTO
 * 用于更新设备信息时的数据传输
 */
class DeviceUpdateDto : public vsoa::DTO {
    DTO_INIT(DeviceUpdateDto, DTO)

    DTO_FIELD(String, name, "name");               ///< 设备名称
    DTO_FIELD(UInt32, driver_id, "driver_id");      ///< 驱动ID
    DTO_FIELD(UInt32, conn_type, "conn_type");      ///< 连接类型
    DTO_FIELD(String, connparam, "connparam");     ///< 连接参数
    DTO_FIELD(String, description, "description"); ///< 设备描述
    DTO_FIELD(String, param1, "param1");           ///< 参数1值
    DTO_FIELD(String, param2, "param2");           ///< 参数2值
    DTO_FIELD(String, param3, "param3");           ///< 参数3值
    DTO_FIELD(String, param4, "param4");           ///< 参数4值
};

/**
 * 带驱动信息的设备DTO
 * 用于同时展示设备和关联的驱动信息
 */
class DeviceWithDriverDto : public vsoa::DTO {
    DTO_INIT(DeviceWithDriverDto, DTO)

    DTO_FIELD(UInt32, id, "id");             ///< 设备ID
    DTO_FIELD(String, name, "name");           ///< 设备名称
    DTO_FIELD(UInt32, driver_id, "driver_id");       ///< 驱动ID
    DTO_FIELD(String, driver_name, "driver_name");    ///< 驱动名称
    DTO_FIELD(UInt32, conn_type, "conn_type");       ///< 连接类型
    DTO_FIELD(String, connparam, "connparam");      ///< 连接参数
    DTO_FIELD(String, description, "description");    ///< 设备描述
    DTO_FIELD(String, param1, "param1");         ///< 参数1值
    DTO_FIELD(String, param2, "param2");         ///< 参数2值
    DTO_FIELD(String, param3, "param3");         ///< 参数3值
    DTO_FIELD(String, param4, "param4");         ///< 参数4值
};

/**
 * 设备分页DTO
 * 用于分页展示设备列表
 */
class DevicesPageDto : public PageDto<vsoa::Object<DeviceDto>> {
    DTO_INIT(DevicesPageDto, PageDto<vsoa::Object<DeviceDto>>)
};

/**
 * 带驱动信息的设备分页DTO
 * 用于分页展示设备列表，同时包含关联的驱动信息
 */
class DevicesWithDriverPageDto : public PageDto<vsoa::Object<DeviceWithDriverDto>> {
    DTO_INIT(DevicesWithDriverPageDto, PageDto<vsoa::Object<DeviceWithDriverDto>>)
};

#include VSOA_CODEGEN_END(DTO)

#endif // DEVICE_DTO_HPP
