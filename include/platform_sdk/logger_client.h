/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: logger_client.h .
*
* Date: 2024-05-16
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "libvsoa/vsoa_client.h"

#ifndef _VSOA_SDK_LOGGER_CLIENT_H_
#define _VSOA_SDK_LOGGER_CLIENT_H_

class VsoaLoggerClientImpl;
class VsoaLoggerClient {
    friend VsoaLoggerClientImpl;

public:
    /* Log client initial */
    int InitLogClient(const char* model, const char* log_server = nullptr);

    /* Print log message */
    void LogMessage(int nLogLevel, const char *szFormat, ...);

public:
    /* Vsoa logger client constructer */
    VsoaLoggerClient();

    /* Vsoa logger client destructer */
    ~VsoaLoggerClient();

private:
    VsoaLoggerClientImpl* impl = nullptr;
};

#endif // _VSOA_SDK_LOGGER_CLIENT_H_

/*
 * end
 */
