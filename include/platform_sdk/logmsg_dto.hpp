/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: logmsg_dto.hpp .
*
* Date: 2024-04-10
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_LOGMSG_DTO_H_
#define _VSOA_SDK_LOGMSG_DTO_H_

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/*
 * Log message info DTO
 */
class LogMsgDto : public vsoa::DTO
{
    DTO_INIT(LogMsgDto, DTO)
    DTO_FIELD(UInt32, seqno, "seqno");
    DTO_FIELD(String, time, "log time");
    DTO_FIELD(String, model, "model name");
    DTO_FIELD(String, loglevel, "log level");
    DTO_FIELD(String, logmsg, "logmsg");
};

#include VSOA_CODEGEN_END(DTO)

#endif /* _VSOA_SDK_LOGMSG_DTO_H_ */

/*
 * end
 */