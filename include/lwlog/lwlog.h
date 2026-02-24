/*
 * Copyright (c) 2024 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: lwlog.h .
 *
 * Date: 2024-03-23
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#ifndef _LWLOG_H_
#define _LWLOG_H_

#ifdef _WIN32
#ifdef lwlog_EXPORTS
#define LWLog_API __declspec(dllexport)
#else
#define LWLog_API __declspec(dllimport)
#endif
#else
#define LWLog_API __attribute__((visibility("default")))
#endif //_WIN32

#include "lwcomm/lwcomm.h"

#ifndef LW_LOGLEVEL_DEBUG
#define LW_LOGLEVEL_DEBUG 0x01
#define LW_LOGLEVEL_INFO 0x02
#define LW_LOGLEVEL_WARN 0x04
#define LW_LOGLEVEL_ERROR 0x08
#define LW_LOGLEVEL_CRITICAL 0x10
#endif // LW_LOGLEVEL_DEBUG

class CLWLogImp;

class LWLog_API CLWLog
{
public:
    CLWLog(void);
    ~CLWLog();

    /* Set the log output file name, which is also used as the log category identifier, 
     * and does not call this method to General by default */
    bool SetLogFileName(const char *szLogFileName = nullptr);

    /* Record one message */
    void LogMessage(int nLogLevel, const char *szFormat, ...);

    /* Record one error message */
    void LogErrMessage(const char *szFormat, ...); 

    /* Record one message with hex data */
    void LogHexMessage(int nLogLevel, const char *szHexBuf, int nHexBufLen, const char *szFormat, ...);  

private:
    CLWLogImp *m_pLWLogImp;
};

#endif //! defined(_LWLOG_H_)

/*
 * end
 */