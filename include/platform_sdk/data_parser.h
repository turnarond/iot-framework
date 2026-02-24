/*
* Copyright (c) 2024 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: data_parser.h .
*
* Date: 2025-03-18
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "platform_sdk/macro.h"

struct vsoa_sdk_payload_t
{
    void *param;
    size_t param_len;
    void *data;
    size_t data_len;
};

namespace vsoa_sdk
{
namespace data_parser
{
    void setDataMode (DataMode mode);

    void setData (vsoa_sdk_payload_t *payload, void *data, size_t len);

    void setParam (vsoa_sdk_payload_t *payload, void *param, size_t len);

    void* getData (vsoa_sdk_payload_t *payload);

    size_t getDataLen (vsoa_sdk_payload_t *payload);

    void* getParam (vsoa_sdk_payload_t *payload);

    size_t getParamLen (vsoa_sdk_payload_t *payload);
}
}