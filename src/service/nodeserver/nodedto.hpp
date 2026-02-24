/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: nodedto.hpp .
*
* Date: 2025-07-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)


class DataValueDto : public vsoa::DTO
{ 
    DTO_INIT(DataValueDto, DTO);
    DTO_FIELD(String, name, "name");
    DTO_FIELD(String, value, "value");
    DTO_FIELD(UInt64, time, "time");
    DTO_FIELD(String, quality, "quality");
};

class DeviceTagsDto : public vsoa::DTO
{ 
    DTO_INIT(DeviceTagsDto, DTO);
    DTO_FIELD(String, device_name, "device_name");
    DTO_FIELD(Vector<String>, taglist, "taglist");
};

class DriverTagsDto : public vsoa::DTO
{ 
    DTO_INIT(DriverTagsDto, DTO);
    DTO_FIELD(String, driver_name, "driver_name");
    DTO_FIELD(Vector<vsoa::Object<DeviceTagsDto> >, devtags, "devtags");
};

class ControlValueDto : public vsoa::DTO
{
    DTO_INIT(ControlValueDto, DTO);
    DTO_FIELD(String, name, "name");
    DTO_FIELD(String, value, "value");
};

#include VSOA_CODEGEN_END(DTO)
