/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: master_cli.h .
*
* Date: 2024-02-28
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_MASTER_CLI_H_
#define _VSOA_SDK_MASTER_CLI_H_

#include "lwevent/lwevent.h"
#include "platform_sdk/master.h"
#include "libvsoa/vsoa_cliauto.h"

#include <map>
#include <mutex>
#include <memory>

class MasterCli
{
public:
    /* Get Instance of MasterCli */
    static MasterCli* GetInstance();

    /* Get the register status */ 
    bool RegisterStatus(void) { return this->registered; }

    /* Register Server Manager */
    int ReRegisterSrvMgr(void);

    /* Register Server Manager with service info */
    int RegisterSrvMgr(vsoa_sdk::master::srv_item_t &srv);

    /* Get service list by URL */
    int GetSrvListByUrl(std::string url, std::list<vsoa_sdk::master::srv_item_t> &srv_list);

    /* Get server list */
    int GetSrvList(std::list<std::string> &list);

    /* Get server list by server name */
    int GetSrvInfoByName(std::list<std::string> &srvlist, std::list<vsoa_sdk::master::srv_item_t> &srv_list);

    /* Get proxy server list*/
    int GetProxySrvList(std::list<std::string> &srv_list);

    /* On master client start */
    int OnStart(void);

    /* On master client stop */
    int OnStop(void);

private:
    /* On master client connect callback */
    static void OnConnectCb(void *arg, vsoa_client_auto_t *cliauto, bool connect, const char *info);

    /* On master client register callback */
    static void OnRegisterCb(void *arg, vsoa_client_t *client, 
                             vsoa_header_t *vsoa_hdr, vsoa_payload_t *payload);

    /* On master client get service list callback */                       
    static void OnGetSrvListCb(void *arg, vsoa_client_t *client, 
                               vsoa_header_t *vsoa_hdr, vsoa_payload_t *payload);

    int RegisterSrvMgrLocal(vsoa_sdk::master::srv_item_t &srv_reg_info);

private:
    bool in_stopping = false; /* Master client is will stop soon. */
    bool connected = false; /* Master client is connected. */
    bool registered = false; /* Master client is registered */
    bool inited = false; /* Master client is inited */
    bool started = false;

    vsoa_client_t *cli_handle;
    vsoa_client_auto_t *cliauto;
    std::map<std::string, vsoa_sdk::master::srv_item_t> local_srv_item; /* current service information */

    std::string req_url;
    std::list<vsoa_sdk::master::srv_item_t> srv_list;
    std::mutex map_mtx;

    EVENT_HANDLE connect_event; /* The event of the connect */
    EVENT_HANDLE sync_url2server; /* The event of the URL sync */

    static std::once_flag s_flag;
    static std::unique_ptr<MasterCli> instance_;

public:
    /* Master client start */  
    int Start(void);

    /* Master client stop */  
    int Stop(void);

    /* Master client destructure */
    virtual ~MasterCli();

private:
    /* Master client constructure */  
    MasterCli(void);
};

#define MASTER_CLI MasterCli::GetInstance()

#pragma once

#endif // _VSOA_SDK_MASTER_CLI_H_

/*
 * end
 */
