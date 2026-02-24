/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: auth_dto.hpp .
*
* Date: 2024-06-28
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _AUTH_DTO_HPP_
#define _AUTH_DTO_HPP_

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

class AuthReqDto : public vsoa::DTO
{
    DTO_INIT(AuthReqDto, DTO)
    DTO_FIELD(String, server_name, "server_name");
    DTO_FIELD(String, password, "password");
    DTO_FIELD(UInt64, create_time, "create_time");
    DTO_FIELD(UInt64, authority, "authority");
};

class AuthResqDto : public vsoa::DTO
{
    DTO_INIT(AuthResqDto, DTO)
    DTO_FIELD(String, signature, "signature");
    DTO_FIELD(String, token, "token");
};

#include VSOA_CODEGEN_END(DTO)

#endif // _AUTH_DTO_HPP_

/*
 * end
 */