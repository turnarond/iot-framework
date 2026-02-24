/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: init.h .
*
* Date: 2024-02-19
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_INIT_H_
#define _VSOA_SDK_INIT_H_

#include <string>
#include "lwcomm/lwcomm.h"
#include "platform_sdk/macro.h"

#ifdef _MSC_VER
    const std::string g_default_pos_file = "C:\\Windows\\System32\\drivers\\etc\\vsoa.pos";
#else
    const std::string g_default_pos_file = "/etc/vsoa.pos";
#endif 

namespace vsoa_sdk
{
    LWCOMM_API bool isStarted(void);

    LWCOMM_API void shutdown(void);

    LWCOMM_API void init(const std::string name = std::string());

    LWCOMM_API void setDataMode(DataMode mode);

    LWCOMM_API const std::string getPosFileURI(void);

    LWCOMM_API const std::string &getDefaultMasterURI(void);
}

#endif // _VSOA_SDK_INIT_H_

/*
 * end
 */