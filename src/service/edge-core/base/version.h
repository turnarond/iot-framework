/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: version.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#ifndef _EDGE_CORE_BASE_VERSION_H_
#define _EDGE_CORE_BASE_VERSION_H_

#define EDGE_CORE_BASE_VERSION_MAJOR 1
#define EDGE_CORE_BASE_VERSION_MINOR 0
#define EDGE_CORE_BASE_VERSION_PATCH 0

#define EDGE_CORE_BASE_VERSION_STR "1.0.0"

#ifdef __cplusplus
extern "C" {
#endif

void pirnt_version();

#ifdef __cplusplus
}
#endif
#endif // _EDGE_CORE_BASE_VERSION_H_
