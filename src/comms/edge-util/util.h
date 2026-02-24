/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: edge_util.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#ifndef _EDGE_UTIL_H_
#define _EDGE_UTIL_H_ 

#ifdef _WIN32
#ifdef lwcomm_EXPORTS
#define LWEDGE_API __declspec(dllexport)
#else
#define LWEDGE_API __declspec(dllimport)
#endif
#else //_WIN32
#define LWEDGE_API __attribute__((visibility("default")))
#endif //_WIN32

#ifdef __cplusplus
extern "C" {
#endif 

#define EDGE_SN_LEN 32

LWEDGE_API const char* get_edge_sn();

#ifdef __cplusplus
}
#endif

#endif // _EDGE_UTIL_H_