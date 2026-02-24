/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwdrvcmn.h .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "comm_helper.h"

#include <cstdint>

typedef struct _LWTAG
{
    char *name; // tag name
    char *address; // tag address

    int data_type; // data type
    int point_type; // point type (1=device_variable, 2=computed_variable)
    int transfer_type; // transfer type (0=none, 1=linear_scaling, 2=advanced_algorithm)
    int polling_interval; // polling interval in milliseconds
    
    // linear scaling parameters
    double linear_raw_min; // linear raw min
    double linear_raw_max; // linear raw max
    double linear_eng_min; // linear engineering min
    double linear_eng_max; // linear engineering max
    
    // advanced algorithm parameters
    char *advanced_algo_lib; // advanced algorithm library
    char *advanced_param1; // advanced parameter 1
    char *advanced_param2; // advanced parameter 2
    char *advanced_param3; // advanced parameter 3
    char *advanced_param4; // advanced parameter 4
    
    bool enable_control; // enable control
    bool enable_history; // enable history

    // additional fields for compatibility
    int byte_order; // byte order
    char *param; // additional parameters
    int len_bit; // store the variable length in bits

    // the following fields are used for data storage
    char *data; // store the variable data
    int data_length; // data length
    int quantity; // store the variable quantity
    uint64_t time_milli; // store the variable time in milliseconds.

    // for developer, reserved data
    int res_data1;
    int res_data2;
    int res_data3;
    int res_data4;
    void *res_pdata1;
    void *res_pdata2;
} LWTAG;

struct _LWDRIVER;
typedef struct _LWDEVICE
{
    char *name;
    char *desc;
    int conn_type; // connection type, reference to t_dict_conn_types
    char *conn_param; // connection parameters
    char *param1; // parameter 1
    char *param2; // parameter 2
    char *param3; // parameter 3
    char *param4; // parameter 4

    _LWDRIVER *driver;
    LWTAG **pptags;  // current device tags.
    int tag_count;
    void *_pInternalRef; // internal reference for device.

    int conn_timeout; // connection timeout in milliseconds.
    int recv_timeout; // receive timeout in milliseconds.
    bool enable_connect; // if the device is enabled for connection.

    // the below fields are set for driver to use, cannot be modified.
    // the follow fields are reserved for internal use.
    void *pUserData[DRV_USERDATA_MAXNUM];
    int nUserData[DRV_USERDATA_MAXNUM];
} LWDEVICE;

typedef struct _LWDRIVER {
    char *name;
    int type; // driver type
    char *version; // driver version
    char *description; // driver description
    
    // parameter definitions
    char *param1_name; // parameter 1 name
    char *param1_desc; // parameter 1 description
    char *param2_name; // parameter 2 name
    char *param2_desc; // parameter 2 description
    char *param3_name; // parameter 3 name
    char *param3_desc; // parameter 3 description
    char *param4_name; // parameter 4 name
    char *param4_desc; // parameter 4 description

    LWDEVICE **ppdevices;
    int device_count;
    void* _pInternalRef;

    // the below fields are set for driver to use, cannot be modified.
    // the follow fields are reserved for internal use.
    void *pUserData[DRV_USERDATA_MAXNUM];
    int nUserData[DRV_USERDATA_MAXNUM];
} LWDRIVER;

typedef struct _LWTIMER {
    int period_ms; // period in milliseconds
    int phase_ms; // phase in milliseconds
    void *internal_ref; // internal reference for timer

    // the follow fields are reserved for internal use.
    void *ptr_user_data[DRV_USERDATA_MAXNUM];
    int int_user_data[DRV_USERDATA_MAXNUM];
} DRVTIMER;

// 主动与设备进行连接。通常不需要显示调用，读写数据时驱动框架会自动和设备建立连接。发生错误时可以手工断开和重新连接
LWDRIVER_EXPORTS int drv_connect(LWDEVICE *device, int timeout_ms);
// 主动和设备断开连接。通常不需要显示调用，可以在发生错误时主动断开与设备的连接，以便重新握手时调用
LWDRIVER_EXPORTS int drv_disconnect(LWDEVICE *device);
// 向设备发送nBufLen长度的内容，返回发送的字节个数
LWDRIVER_EXPORTS int drv_send(LWDEVICE *device, const char *data, unsigned int len, int timeout_ms);
// 从设备接收最多nBufLen长度的内容，返回接收的实际字节个数
LWDRIVER_EXPORTS int drv_recv(LWDEVICE *device, char *buffer, unsigned int len, int timeout_ms);

LWDRIVER_EXPORTS int drv_sendto(LWDEVICE *device, char *address, const char *data, unsigned int len, int timeout_ms);

LWDRIVER_EXPORTS int drv_recvfrom(LWDEVICE *device, char *address, char *buffer, unsigned int len, int timeout_ms);

LWDRIVER_EXPORTS void drv_clear_recv_buffer(LWDEVICE *device);

// 设置某个TAG的值(需为文本字符串格式)、时间戳、质量。仅修改pTag的VTQ, 还未写入内存数据库
LWDRIVER_EXPORTS int drv_settagdata_text(LWTAG *tag, const char* value, unsigned int tagmsec, int tag_quantity);
// 设置某个TAG的值(二进制原始值)、时间戳、质量。仅修改pTag的VTQ, 还未写入内存数据库
LWDRIVER_EXPORTS int drv_settagdata_binary(LWTAG *tag, void *value, int value_len, unsigned int tagmsec, int tag_quantity);
LWDRIVER_EXPORTS int drv_update_tagsdata(LWDEVICE *device, LWTAG **tag, int tag_count);
LWDRIVER_EXPORTS int drv_update_tagdate_by_address(LWDEVICE *device, const char *address, void *value, uint64_t value_len, uint64_t tagmsec, int tag_quantity);

LWDRIVER_EXPORTS void drv_logmessage(int level, const char *fmt, ...);

LWDRIVER_EXPORTS void *drv_create_timer(LWDEVICE *device, DRVTIMER *timerinfo);
LWDRIVER_EXPORTS void drv_destroy_timer(LWDEVICE *device, void *timer_handle);

LWDRIVER_EXPORTS int drv_gettags_by_address(LWDEVICE *device, const char *address, LWTAG **tag, int tag_count);
LWDRIVER_EXPORTS int drv_gettag_data(LWDEVICE *device, LWTAG *tag, char *buffer, unsigned int len, int *ret_value_len, 
                                     unsigned int *tagmsec, int *tag_quantity);
LWDRIVER_EXPORTS int drv_tagval_str2binary(LWTAG *tag, const char *tag_string_val, void *buffer, unsigned int buffer_len, 
    int *ret_value_buffer_len_bytes, int *ret_value_buffer_len_bits);

LWDRIVER_EXPORTS int drv_set_connect_timeout(LWDEVICE *device, int timeout_ms);
LWDRIVER_EXPORTS int drv_set_connect_success(LWDEVICE *device, bool success);
