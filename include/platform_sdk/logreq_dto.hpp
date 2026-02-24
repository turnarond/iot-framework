/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: logreq_dto.hpp .
*
* Date: 2024-05-16
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_LOGREQ_DTO_H_
#define _VSOA_SDK_LOGREQ_DTO_H_

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/*
 * Log request info DTO
 */
class LogRequestDto : public vsoa::DTO
{
    DTO_INIT(LogRequestDto, DTO)
    DTO_FIELD(String, loglevel, "log level");
    DTO_FIELD(String, begintime, "begin time");
    DTO_FIELD(String, endtime, "end time");
};

class LogSubReqDto : public vsoa::DTO
{
    DTO_INIT(LogSubReqDto, DTO)
    DTO_FIELD(String, model, "model");
    DTO_FIELD(String, loglevel, "log level");
    DTO_FIELD(String, begintime, "begin time");
    DTO_FIELD(String, endtime, "end time");
};

class LogSubReqPageDto : public vsoa::DTO
{
    DTO_INIT(LogSubReqPageDto, DTO)
    DTO_FIELD(List<Object<LogSubReqDto> >, subreqlist, "log list");
};

#include VSOA_CODEGEN_END(DTO)

#endif /* _VSOA_SDK_LOGREQ_DTO_H_ */

/*
 * end
 */