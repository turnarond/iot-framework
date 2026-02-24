/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: version.cpp .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "version.h"
#include "lwlog/lwlog.h"

extern CLWLog g_logger;

const char *versio_logs_[] = {
    "Init edge-core base module"
};


void pirnt_version()
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "####################################");
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "# Edge-Core Base Version: %s#\n", EDGE_CORE_BASE_VERSION_STR); 
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "####################################");

}