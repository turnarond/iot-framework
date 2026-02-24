/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: comm_helper.h .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _IOT_PLATFORM_COMM_HELPER_H
#define _IOT_PLATFORM_COMM_HELPER_H 

#ifndef LWDRIVER_EXPORTS
#ifdef _WIN32
#define LWDRIVER_EXPORTS extern "C" __declspec(dllexport)
#else
#define LWDRIVER_EXPORTS extern "C" __attribute__ ((visibility ("default")))
#endif
#endif // LWDRIVER_EXPORTS


// TAG点的数据类型.在控制命令时会作为参数传入
#define TAG_DT_UNKNOWN          0		// 未知类型
#define TAG_DT_BOOL             1		// 1 bit value
#define TAG_DT_INT8             2		// 8 bit signed integer value
#define TAG_DT_UINT8            3		// 8 bit signed integer value
#define TAG_DT_INT16            4		// 16 bit Signed Integer value
#define TAG_DT_UINT16           5		// 16 bit Unsigned Integer value
#define TAG_DT_INT32            6		// 16 bit Signed Integer value
#define TAG_DT_UINT32           7		// 16 bit Unsigned Integer value
#define TAG_DT_INT64            8		// 16 bit Signed Integer value
#define TAG_DT_UINT64           9		// 16 bit Unsigned Integer value
#define TAG_DT_FLOAT            10		// 32 bit IEEE float
#define TAG_DT_DOUBLE           11		// 64 bit double
#define TAG_DT_TEXT             12		// 4 byte TIME (H:M:S:T)
#define TAG_DT_BLOB             13		// blob, maximum 65535
#define TAG_DT_MAX              13		// Total number of Datatypes

#define DT_NAME_BOOL            "bool"		// 1 bit value
#define DT_NAME_CHAR            "int8"		// 8 bit signed integer value
#define DT_NAME_UCHAR           "uint8"		// 8 bit unsigned integer value
#define DT_NAME_INT16           "int16"		// 16 bit Signed Integer value
#define DT_NAME_UINT16          "uint16"	// 16 bit Unsigned Integer value
#define DT_NAME_INT32           "int32"		// 32 bit signed integer value
#define DT_NAME_UINT32          "uint32"	// 32 bit integer value
#define DT_NAME_INT64           "int64"		// 64 bit signed integer value
#define DT_NAME_UINT64          "uint64"	// 64 bit unsigned integer value
#define DT_NAME_FLOAT           "float"		// 32 bit IEEE float
#define DT_NAME_DOUBLE          "double"	// 64 bit double
#define DT_NAME_TEXT            "text"		// ASCII string, maximum: 127
#define DT_NAME_BLOB            "blob"		// blob, maximum 65535

#define LW_LOGLEVEL_DEBUG 0x01
#define LW_LOGLEVEL_INFO 0x02
#define LW_LOGLEVEL_WARN 0x04
#define LW_LOGLEVEL_ERROR 0x08
#define LW_LOGLEVEL_CRITICAL 0x10
#define LW_LOGLEVEL_NOTICE 0x20
#define LW_LOGLEVEL_COMM 0x100

#define LW_NAME_MAXLEN          128
#define LW_IOADDR_MAXLEN        128
#define LW_DESC_MAXLEN          256
#define LW_PARAM_MAXLEN         512
#define LW_USERDATA_MAXNUM      10

#define DRV_USERDATA_MAXNUM 10

#define NULLASSTRING(x) x==NULL?"":x

#endif // _IOT_PLATFORM_COMM_HELPER_H