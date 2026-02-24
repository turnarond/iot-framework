/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: macro.h .
*
* Date: 2024-02-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_MACRO_H_
#define _VSOA_SDK_MACRO_H_

#include <string.h>

#ifdef __cplusplus

#if __cplusplus < 201402L
#include <memory>
namespace std {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

extern "C"
{
#endif
#ifdef __GNUC__
#if __GNUC__ >= 3
#define VSOACONSOLE_PRINTF_ATTRIBUTE(a, b) __attribute__((__format__ (__printf__, a, b)))
#endif
#endif

#ifndef VSOA_UNLIKELY
#define VSOA_UNLIKELY(x)     __builtin_expect(!!(x),0)
#endif
#ifndef VSOA_LIKELY
#define VSOA_LIKELY(x)       __builtin_expect(!!(x),1)
#endif

#define DEFAULT_KEEPALIVE_TIMEOUT 3000
#define DEFAULT_CONNECT_TIMEOUT 1000
#define DEFAULT_RECONN_DELAY 2000

#define DEFAULT_RPC_TIMEOUT     3000

#define LOCAL_IPADDR            "127.0.0.1"
#define ANY_IPADDR              "0.0.0.0"

#define MAX_SERVER_NAME_LENGTH  256
#define MAX_SUBURL_NUM          32
#define MAX_SYNCCALL_NUM        4096
#define LOCAL_SYNC_SEQNO(seqno) ((seqno) % (MAX_SYNCCALL_NUM))

#define MAX_NAME_LEN        32
#define MAX_HOST_LEN        32
#define MAX_PASSWD_LEN      16
#define MAX_CLIENT_NUM      16
#define MAX_URL_LEN         32

#define MAX_TAGNAME_LEN     32
#define DEFAULT_VALUE_LEN   64
#define MAX_CONNURL_LEN     64

#ifndef VSOA_LOGLEVEL_DEBUG
#define VSOA_LOGLEVEL_DEBUG 0x01
#endif // LW_LOGLEVEL_DEBUG
#ifndef VSOA_LOGLEVEL_INFO
#define VSOA_LOGLEVEL_INFO  0x02
#endif // VSOA_LOGLEVEL_INFO
#ifndef VSOA_LOGLEVEL_WARN
#define VSOA_LOGLEVEL_WARN  0x04
#endif // VSOA_LOGLEVEL_WARN
#ifndef VSOA_LOGLEVEL_ERROR
#define VSOA_LOGLEVEL_ERROR 0x08
#endif // VSOA_LOGLEVEL_ERROR
#ifndef VSOA_LOGLEVEL_FATAL
#define VSOA_LOGLEVEL_FATAL 0x10
#endif // VSOA_LOGLEVEL_FATAL

#ifndef VSOA_LOGLEVEL_DEBUG_STR
#define VSOA_LOGLEVEL_DEBUG_STR "DEBUG"
#endif // VSOA_LOGLEVEL_DEBUG_STR
#ifndef VSOA_LOGLEVEL_INFO_STR
#define VSOA_LOGLEVEL_INFO_STR  "INFO"
#endif // VSOA_LOGLEVEL_INFO_STR
#ifndef VSOA_LOGLEVEL_WARN_STR
#define VSOA_LOGLEVEL_WARN_STR  "WARN"
#endif // VSOA_LOGLEVEL_WARN_STR
#ifndef VSOA_LOGLEVEL_ERROR_STR
#define VSOA_LOGLEVEL_ERROR_STR "ERROR"
#endif // VSOA_LOGLEVEL_ERROR_STR
#ifndef VSOA_LOGLEVEL_FATAL_STR
#define VSOA_LOGLEVEL_FATAL_STR "FATAL"
#endif // VSOA_LOGLEVEL_FATAL_STR

inline int VSOA_LOGLEVEL_STR_TO_LEVEL(const char* level_str)
{
    if (strcmp(level_str, VSOA_LOGLEVEL_DEBUG_STR) == 0) {
        return VSOA_LOGLEVEL_DEBUG;
    }
    if (strcmp(level_str, VSOA_LOGLEVEL_INFO_STR) == 0) {
        return VSOA_LOGLEVEL_INFO;
    }
    if (strcmp(level_str, VSOA_LOGLEVEL_WARN_STR) == 0) {
        return VSOA_LOGLEVEL_WARN;
    }
    if (strcmp(level_str, VSOA_LOGLEVEL_ERROR_STR) == 0) {
        return VSOA_LOGLEVEL_ERROR;
    }
    if (strcmp(level_str, VSOA_LOGLEVEL_FATAL_STR) == 0) {
        return VSOA_LOGLEVEL_FATAL;
    }
    else {
        return VSOA_LOGLEVEL_DEBUG;
    }
}

inline const char* VSOA_LOGLEVEL_LEVEL_TO_STR(int level)
{
    switch (level) {
    case VSOA_LOGLEVEL_DEBUG:
        return VSOA_LOGLEVEL_DEBUG_STR;
    case VSOA_LOGLEVEL_INFO:
        return VSOA_LOGLEVEL_INFO_STR;
    case VSOA_LOGLEVEL_WARN:
        return VSOA_LOGLEVEL_WARN_STR;
    case VSOA_LOGLEVEL_ERROR:
        return VSOA_LOGLEVEL_ERROR_STR;
    case VSOA_LOGLEVEL_FATAL:
        return VSOA_LOGLEVEL_FATAL_STR;
    default:
        return VSOA_LOGLEVEL_DEBUG_STR;
    }
}

/*
 * If need authentication.
 */
#define NEED_AUTH 0

/*
 * Server authority type
 */
typedef enum {
    SYS_SERVER_AUTHORITY = 0,
    USER_SERVER_AUTHORITY = 3
} VSOA_SERVER_AUTHORITY;

/* 
 * Use vsoa payload data mode.
 */
 enum class DataMode {
    PARAM_MODE = 0,   // use param, param_len as payload data;
    DATA_MODE     // use data, data_len as payload data;
};

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_VSOA_SDK_MACRO_H_

/*
 * end
 */
