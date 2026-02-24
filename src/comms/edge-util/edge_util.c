/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: edge_util.c .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "util.h"

#include <string.h>

LWEDGE_API const char *get_edge_sn()
{
    static char edge_sn[EDGE_SN_LEN] = { 0 };
    if (edge_sn[0] == '\0') {
        return edge_sn;
    }

    strncpy(edge_sn, "AABBCCDDEE", 10);

    return edge_sn;
}
