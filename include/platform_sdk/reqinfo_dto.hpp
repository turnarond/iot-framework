/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: reqinfo_dto.hpp .
*
* Date: 2024-04-10
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_REQINFO_DTO_H_
#define _VSOA_SDK_REQINFO_DTO_H_

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

class TracerInfoDto : public vsoa::DTO
{
    DTO_INIT(TracerInfoDto, DTO);
    DTO_FIELD(String, name, "name");
    DTO_FIELD(String, trace_id, "trace_id");
    DTO_FIELD(String, parent_span_id, "parent_span_id");
    DTO_FIELD(String, start, "start");
    DTO_FIELD(String, duration, "duration");
    DTO_FIELD(String, description, "description");
    DTO_FIELD(String, kind, "kind");
    DTO_FIELD(String, status, "status");
    DTO_FIELD(List<vsoa::String>, attributes, "attributes");
    DTO_FIELD(String, events, "events");
    DTO_FIELD(String, links, "links");
    DTO_FIELD(List<vsoa::String>, resources, "resources");
    DTO_FIELD(String, instrlib, "instr-lib");
};

/*
 * Server Info DTO
 */
class HeaderInfoDTO : public vsoa::DTO
{
    DTO_INIT(HeaderInfoDTO, DTO)
    DTO_FIELD(Int32, request_time, "request_time");
    DTO_FIELD(String, url, "url");
    DTO_FIELD(Int32, method, "method");
    DTO_FIELD(String, server_name, "server_name");
    DTO_FIELD(UInt64, data_len, "data_len");
    DTO_FIELD(String, token, "token");
    DTO_FIELD(String, spans, "spans");
};

#include VSOA_CODEGEN_END(DTO)

#endif /* _VSOA_SDK_REQINFO_DTO_H_ */

/*
 * end
 */