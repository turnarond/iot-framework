/*
* Copyright (c) 2025 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: broker_def.h .
*
* Date: 2025-11-13
*
* Author: Zhang.lang <zhanglang@acoinfo.com>
*
*/

#pragma once

#include <iostream>
#include <string>
#include <stdint.h>


namespace vsoa_sdk {
namespace broker {

// global variable definition
static const char* const GSCpBrokerSetLastwordsUrl = "/v1/client/lastwords"; // the broker lastwords url prefix
static const char* const GSCpBrokerSetPrefixUrl = "/v1/client/setting"; // the broker rpc url prefix
static const std::string GSCstrBrokerSrvName = "vosa_broker_server"; // the vsoa broker server default name
static const uint16_t GSCui16BrokerPort = 18830; // the vsoa broker server default port

// broker server configuration information
struct BrokerCfgInfo {
    uint16_t ui16Authority = 0; // broker server authority
    uint16_t ui16Port = 0; // broker server port
    std::string strIPv4 = ""; // broker server ipv4 info, if empty means ANY_IPADDR
    std::string strName = ""; // broker server name, if empty, it will use default name: vosa_broker_server
    std::string strPassword = ""; // broker server connect password, if empty means not use password
};

} // namespace broker
} // namespace vsoa_sdk

