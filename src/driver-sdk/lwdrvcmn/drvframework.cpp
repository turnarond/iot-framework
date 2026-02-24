/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: drvframework.cpp .
*
* Date: 2025-07-04
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "drvframework.h"
#include "comm_helper.h"

int DrvFramework::GetTagDataTypeAndLen(std::string& datatype, int datalen, int &datatype_id, int &datalen_out)
{
    if (datatype == DT_NAME_BOOL) {
        datatype_id = TAG_DT_BOOL;
        datalen_out = 1;
    } else if (datatype == DT_NAME_CHAR) {
        datatype_id = TAG_DT_INT8;
        datalen_out = 8;
    } else if (datatype == DT_NAME_UCHAR) {
        datatype_id = TAG_DT_UINT8;
        datalen_out = 8;
    } else if (datatype == DT_NAME_INT16) {
        datatype_id = TAG_DT_INT16;
        datalen_out = 16;
    } else if (datatype == DT_NAME_UINT16) {
        datatype_id = TAG_DT_UINT16;
        datalen_out = 16;
    } else if (datatype == DT_NAME_INT32) {
        datatype_id = TAG_DT_INT32;
        datalen_out = 32;
    } else if (datatype == DT_NAME_UINT32) {
        datatype_id = TAG_DT_UINT32;
        datalen_out = 32;
    } else if (datatype == DT_NAME_INT64) {
        datatype_id = TAG_DT_INT64;
        datalen_out = 64;
    } else if (datatype == DT_NAME_UINT64) {
        datatype_id = TAG_DT_UINT64;
        datalen_out = 64;
    } else if (datatype == DT_NAME_FLOAT) {
        datatype_id = TAG_DT_FLOAT;
        datalen_out = 32;
    } else if (datatype == DT_NAME_DOUBLE) {
        datatype_id = TAG_DT_DOUBLE;
        datalen_out = 64;
    } else if (datatype == DT_NAME_TEXT) {
        datatype_id = TAG_DT_TEXT;
        datalen_out = datalen <= 0 ? 64 : datalen; // Default to 64 if not specified
    } else if (datatype == DT_NAME_BLOB) {
        datatype_id = TAG_DT_BLOB;
        datalen_out = datalen <= 0 ? 64 : datalen; // Default to 64 if not specified
    } else {
        return -1; // Unknown data type
    }

    return 0;
}

const std::string DrvFramework::GetTagDataValueToString(int datatype_id, int datalen, const char *data)
{
    std::string value;
    switch (datatype_id) {
    case TAG_DT_BOOL: {
        bool bool_value = *(bool*)data;
        value = bool_value ? "true" : "false";
        break;
    }
    case TAG_DT_INT8: {
        int8_t int8_value = *(int8_t*)data;
        value = std::to_string(int8_value);
        break;
    }
    case TAG_DT_UINT8: {
        uint8_t uint8_value = *(uint8_t*)data;
        value = std::to_string(uint8_value);
        break;
    }
    case TAG_DT_INT16: {
        int16_t int16_value = *(int16_t*)data;
        value = std::to_string(int16_value);
        break;
    }
    case TAG_DT_UINT16: {
        uint16_t uint16_value = *(uint16_t*)data;
        value = std::to_string(uint16_value);
        break;
    }
    case TAG_DT_INT32: {
        int32_t int32_value = *(int32_t*)data;
        value = std::to_string(int32_value);
        break;
    }
    case TAG_DT_UINT32: {
        uint32_t uint32_value = *(uint32_t*)data;
        value = std::to_string(uint32_value);
        break;
    }
    case TAG_DT_INT64: {
        int64_t int64_value = *(int64_t*)data;
        value = std::to_string(int64_value);
        break;
    }
    case TAG_DT_UINT64: {
        uint64_t uint64_value = *(uint64_t*)data;
        value = std::to_string(uint64_value);
        break;
    }
    case TAG_DT_FLOAT: {
        float float_value = *(float*)data;
        value = std::to_string(float_value);
        break;
    }
    case TAG_DT_DOUBLE: {
        double double_value = *(double*)data;
        value = std::to_string(double_value);
        break;
    }
    case TAG_DT_TEXT: {
        value.assign(data, datalen);
        break;
    }
    case TAG_DT_BLOB: {
        value.assign(data, datalen);
        break;
    }
    default:
        return value; // Unknown data type
    }
    return value; // Success
}