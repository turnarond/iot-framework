/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: auth_cli.h .
*
* Date: 2024-06-28
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_AUTH_CLI_H_
#define _VSOA_SDK_AUTH_CLI_H_

#include "lwevent/lwevent.h"
#include "platform_sdk/master.h"
#include "libvsoa/vsoa_cliauto.h"

#include <map>
#include <mutex>

class AuthSyncCli
{

private:
    bool connected = false; /* Authority client is connected. */

    vsoa_client_t *cli_handle;
    vsoa_client_auto_t *cliauto;

    std::string req_url;

    EVENT_HANDLE connect_event; /* The event of the connect */

protected:

public:
    int GetAuthTokenSync(std::string host, std::string user_info, std::string &token);
    static void OnConnectCb(void *arg, vsoa_client_auto_t *cliauto, bool connect, const char *info);

public:
    /* Authority client constructure */  
    AuthSyncCli(void);

    /* Authority client destructure */  
    virtual ~AuthSyncCli();
};

#pragma once

#endif // _VSOA_SDK_AUTH_CLI_H_

/*
 * end
 */