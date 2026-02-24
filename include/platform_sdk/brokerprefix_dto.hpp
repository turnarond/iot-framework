/*
* Copyright (c) 2025 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: brokerprefix_dto.hpp .
*
* Date: 2025-11-21
*
* Author: Zhang.lang <zhanglang@acoinfo.com>
*
*/

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)


/*
 * broker url prefix info DTO
 */
class BrokerUrlPrefixInfoDTO : public vsoa::DTO
{
    DTO_INIT(BrokerUrlPrefixInfoDTO, DTO)
    DTO_FIELD(String, prefix, "prefix");
};


#include VSOA_CODEGEN_END(DTO)
