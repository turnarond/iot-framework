/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: tracer.h .
*
* Date: 2024-04-22
*
* Author: Wang.xuan <wangxuan@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_TRACER_H_
#define _VSOA_SDK_TRACER_H_

namespace vsoa_sdk
{
    LWCOMM_API bool InitTracer(void);

    LWCOMM_API link_tracer::trace::Tracer *get_tracer(void);
}

#endif /* _VSOA_SDK_TRACER_H_ */
