/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwdrivercommon.cpp .
*
* Date: 2025-06-19
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "comm_helper.h"
#include "lwdrvcmn.h"
#include "lwcomm/lwcomm.h"
#include "lwlog/lwlog.h"
#include "device.h"
#include "maintask.h"

#include <sstream>

CLWLog g_logger;

#define BITS_PER_BYTE 8

#define MAX_LOGTEMP_SIZE 4096

// TODO: Implement the TagValFromString2Bin function
// This function should convert a string value to binary data based on the tag's data type.
// The implementation is not provided here, but it should handle different data types accordingly.
int TagValFromString2Bin(LWTAG *tag, int data_type, const char *value, void *bin_outdata, int outdata_length, int *datatype_len_bits)
{
    // This function should convert a string value to binary data based on the tag's data type.
    // The implementation is not provided here, but it should handle different data types accordingly.
    // For now, we will return 0 to indicate success.
    int str_data_len = strlen(value);
    if (data_type == TAG_DT_TEXT) {
        LWStringHelper::SafeStrNCpy((char*)bin_outdata, value, outdata_length);
        *datatype_len_bits = str_data_len * BITS_PER_BYTE;
        return 0; // Success
    }

    if (data_type == TAG_DT_BLOB) {
        int blob_data_len = 0;
        // FromBase64StrToBlob(value, bin_outdata, outdata_length, &blob_data_len);
        *datatype_len_bits = blob_data_len * BITS_PER_BYTE;
        return 0;
    }

    char strValue[LW_NAME_MAXLEN + 1] = { 0 };
    LWStringHelper::SafeStrNCpy(strValue, value, sizeof(strValue));
    memset(bin_outdata, 0x0, outdata_length);

    int databits = 0;
    std::stringstream ss;
    switch (data_type) {
        case TAG_DT_BOOL: {
            unsigned char bool_value = ::atoi(strValue) == 1;
            memcpy(bin_outdata, &bool_value, sizeof(bool_value));
            databits = 1;
            break;
        }
        case TAG_DT_INT8: { 
            int8_t int8_value = ::atoi(strValue);
            memcpy(bin_outdata, &int8_value, sizeof(int8_value));
            databits = 8;
            break;
        }
        case TAG_DT_UINT8: {
            unsigned char uint8_value = ::atoi(strValue);
            memcpy(bin_outdata, &uint8_value, sizeof(uint8_value));
            databits = 8;
            break;
        }
        case TAG_DT_INT16: {
            int16_t int16_value = ::atoi(strValue);
            memcpy(bin_outdata, &int16_value, sizeof(int16_value));
            databits = sizeof(short) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_UINT16: {
            unsigned short uint16_value = ::atoi(strValue);
            memcpy(bin_outdata, &uint16_value, sizeof(uint16_value));
            databits = sizeof(short) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_INT32: {
            int32_t int32_value = ::atoi(strValue);
            memcpy(bin_outdata, &int32_value, sizeof(int32_value));
            databits = sizeof(int32_t) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_UINT32: {
            unsigned int uint32_value = ::atoi(strValue);
            memcpy(bin_outdata, &uint32_value, sizeof(uint32_value));
            databits = sizeof(uint32_t) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_INT64: {
            int64_t int64_value = ::atoll(strValue);
            memcpy(bin_outdata, &int64_value, sizeof(int64_value));
            databits = sizeof(int64_t) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_UINT64: {
            uint64_t uint64_value = ::atoll(strValue);
            memcpy(bin_outdata, &uint64_value, sizeof(uint64_value));
            databits = sizeof(uint64_t) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_FLOAT: {
            float float_value = ::atof(strValue);
            memcpy(bin_outdata, &float_value, sizeof(float_value));
            databits = sizeof(float) * BITS_PER_BYTE;
            break;
        }
        case TAG_DT_DOUBLE: {
            double double_value = ::atof(strValue);
            memcpy(bin_outdata, &double_value, sizeof(double_value));
            databits = sizeof(double) * BITS_PER_BYTE;
            break;
        }
        default: {
            drv_logmessage(LW_LOGLEVEL_ERROR, "Unknown data type, TAG(%s), type(%d)", tag->name, tag->data_type);
            return -1;
        }
    }
    *datatype_len_bits = databits;
    return 0; // Success
}

LWDRIVER_EXPORTS int drv_connect(LWDEVICE *device, int timeout_ms)
{
    if (device == nullptr || device->_pInternalRef != nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_connect: device is null");
        return -1;
    }

    CDevice *pDevice = (CDevice *)device->_pInternalRef;

    return pDevice->CheckAndConnect(timeout_ms);
}
LWDRIVER_EXPORTS int drv_disconnect(LWDEVICE *device)
{
    if (device == nullptr || device->_pInternalRef == nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_disconnect: device is null");
        return -1;
    }

    CDevice *pDevice = (CDevice *)device->_pInternalRef;

    return pDevice->DisconnectFromDevice();
}

LWDRIVER_EXPORTS void drv_logmessage(int level, const char *fmt, ...)
{
    char szLogString[MAX_LOGTEMP_SIZE];
    va_list ap;
    va_start(ap,fmt);
    vsnprintf(szLogString,MAX_LOGTEMP_SIZE, fmt,ap);
    va_end(ap);
    g_logger.LogMessage(level, szLogString);
}

LWDRIVER_EXPORTS void *drv_create_timer(LWDEVICE *device, DRVTIMER *timerinfo)
{
    if (device == nullptr || device->_pInternalRef == nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_create_timer: device is null");
        return nullptr;
    }

    CDevice *pDevice = (CDevice *)device->_pInternalRef;

    return pDevice->CreateAndStartTimer(timerinfo);

}

LWDRIVER_EXPORTS int drv_update_tagdate_by_address(LWDEVICE *device, 
    const char *address,
    void *value,
    uint64_t value_len,
    uint64_t tagmsec,
    int tag_quantity)
{
    LWTAG *pptags[128] = {0};
    int tag_num = drv_gettags_by_address(device, address, pptags, 128);
    if (tag_num <= 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_update_tagdate_by_address: no tags found for address %s", address);
        return 0; // No tags found
    }

    for (int i = 0; i < tag_num; i++) {
        LWTAG *tag = pptags[i];
        if (tag == nullptr) {
            continue;
        }
        drv_settagdata_binary(tag, value, value_len, tagmsec, tag_quantity);
    }

    return drv_update_tagsdata(device, pptags, tag_num);
}

LWDRIVER_EXPORTS int drv_update_tagsdata(LWDEVICE *device, LWTAG **tags, int tag_count)
{
    if (device == nullptr || device->_pInternalRef == nullptr) {
        return -1;
    }

    CDevice *pDevice = (CDevice *)device->_pInternalRef;
    return pDevice->UpdateTagsData(tags, tag_count);
}

LWDRIVER_EXPORTS int drv_gettags_by_address(LWDEVICE *device, const char *address, LWTAG **tag, int tag_count)
{
    if (device == nullptr || device->_pInternalRef == nullptr) {
        return 0;
    }

    CDevice *pDevice = (CDevice *)device->_pInternalRef;

    return pDevice->GetTagsByAddr(address, tag, tag_count);
}

LWDRIVER_EXPORTS int drv_settagdata_text(LWTAG *tag, const char* value, unsigned int tagmsec, int tag_quantity)
{
    tag->time_milli = tagmsec;
    tag->quantity = tag_quantity;
    int nRet = 0;
    if (tag == nullptr || value == nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_settagdata_text: invalid parameters");
        return -1; // Invalid parameters
    }
    if (tag->data_type == TAG_DT_BLOB)  {
        int value_len = strlen(value);
        if (value_len > tag->data_length)
            value_len = tag->data_length;
        if (value_len < 0)
            nRet = -1001;
        else
            memcpy(tag->data, value, value_len);
    }
    else
    {
        int nDataTypeLenBits = tag->len_bit;
        nRet = TagValFromString2Bin(tag, tag->data_type, value, tag->data, tag->data_length + 1, &nDataTypeLenBits);
    }
    return nRet;
}

int drv_settagdata_binary(LWTAG *tag, void *value, int value_len, unsigned int tagmsec, int tag_quantity)
{
    if (tag == nullptr || value == nullptr || value_len <= 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_settagdata_binary: invalid parameters");
        return -1; // Invalid parameters
    }

    tag->time_milli = tagmsec;
    tag->quantity = tag_quantity;

    if (value_len > tag->data_length) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "drv_settagdata_binary: value length exceeds tag data length");
        return -1001; // Value length exceeds tag data length
    }

    memcpy(tag->data, value, value_len);
    tag->data[value_len] = '\0'; // Ensure null-termination for string types
    return 0; // Success
}