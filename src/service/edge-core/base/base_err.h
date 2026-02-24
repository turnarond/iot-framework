/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: base_err.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#ifndef __BASE_ERR_H__
#define __BASE_ERR_H__

#include <string>

namespace aco {

enum class ErrorCode {
    OK = 0,
    UNKNOWN = 1,
    INVALID_ARGUMENT = 2,
    NOT_FOUND = 3,
    ALREADY_EXISTS = 4,
    PERMISSION_DENIED = 5,
    RESOURCE_EXHAUSTED = 6,
};
}
#endif