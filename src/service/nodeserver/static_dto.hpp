/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: static_dto.hpp .
*
* Date: 2025-07-28
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

class VsoaDtoHelp : public vsoa::DTO
{
    DTO_INIT(VsoaDtoHelp, DTO);
    DTO_FIELD(String, url, "url");
    DTO_FIELD(String, msg, "msg");
};

#include VSOA_CODEGEN_END(DTO)