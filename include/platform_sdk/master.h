/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: master.h .
*
* Date: 2024-02-25
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_MASTER_H_
#define _VSOA_SDK_MASTER_H_

#include <cstdint>
#include "lwcomm/lwcomm.h"
#include <stdint.h>

namespace vsoa_sdk
{

namespace master
{
    const std::string server_name = "vsoa_master";
    const std::string gateway_name = "vsoa_gateway";
    const std::string logger_name = "vsoa_logger";
    const std::string auth_center_name = "auth_center";
    const std::string proxy_server_suffix = "_inner";
    const std::string server_passwd = "acoinfo";
    const std::string register_url = "/master/server_register";
    const std::string server_list_of_url = "/master/server_list_of_url";
    const std::string server_list = "/master/server_list";
    const std::string server_detail_info = "/master/server_info";
    const std::string proxy_service_list = "/master/proxy_service_list";
    const std::string logger_post_url = "/logger/post";
    const std::string logger_config_url = "/logger/config";
    const std::string help_url = "/help";
    const std::string apis_url = "/apis";
    const std::string pre_registered_file = "pre_registered.json";

    enum SRV_STATUS {
        SRV_STATUS_ACTIVE = 0,
        SRV_STATUS_INACTIVE,
        SRV_STATUS_NUM
    };

    typedef struct srv_item_s {
        std::string server_name;
        std::string passwd;
        std::string host;
        unsigned short port;
        unsigned short authority;
        SRV_STATUS srv_info;
        std::list<std::string> ip_list;
        std::list<std::string> publisher;
        std::list<std::string> subscriber;
        std::list<std::string> listener;
        std::list<std::string> contact_node;
        std::list<std::string> proxy_servers;
        srv_item_s() {
            server_name = host = "";
        }
    } srv_item_t;

    LWCOMM_API void init(void);
 
    LWCOMM_API const std::string& getHost(void);

    LWCOMM_API uint16_t getPort(void);
    
    LWCOMM_API const std::string& getURI(void);

    LWCOMM_API bool getNodes(void);

    LWCOMM_API bool getTopics(void);

    LWCOMM_API const std::string& getServerMgrHost(void);

    LWCOMM_API const std::string& getServerMgrUri(void);

    LWCOMM_API uint16_t getServerMgrPort(void);

    LWCOMM_API int setVsoaPosHost(std::string host);

} // namespace master

} // namespace vsoa_sdk

#endif //_VSOA_SDK_MASTER_H_

/*
 * end
 */
