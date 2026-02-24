/*
* Copyright (c) 2025 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: broker.h .
*
* Date: 2025-11-13
*
* Author: Zhang.lang <zhanglang@acoinfo.com>
*
*/

#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>

#include "lwcomm/lwcomm.h"
#include "encoding/encoder_provider.h"
#include "platform_sdk/broker_def.h"

namespace vsoa_sdk {
namespace broker {

// pre-declaration
class BrokerHandleImpl;
class BrokerHandle;

// the class of broker server handle
class LWCOMM_API BrokerHandle
{
public:
    /*********************************************************************
     * description: construct function
     * *******************************************************************/
    BrokerHandle();

    /*********************************************************************
     * description: construct function
     * input params:
     *      stCfgInfo: broker server configuration information
     * *******************************************************************/
    BrokerHandle(const BrokerCfgInfo &stCfgInfo);

    /*********************************************************************
     * description: destruct function
     * *******************************************************************/
    ~BrokerHandle();

    /*********************************************************************
     * description: set broker server configuration information
     * input params:
     *      stCfgInfo: broker server configuration information
     * *******************************************************************/
    void setConfigInfo(const BrokerCfgInfo &stCfgInfo);

    /*********************************************************************
     * description: set data encoding provider, need call before start server
     * input params:
     *      pEncProvider: data encoding provider pointer
     * *******************************************************************/
    void setEncodingProvider(encoding::EncoderProvider *pEncProvider);

    /*********************************************************************
     * description: start broker server
     * return value:
     *      bool: call result, true -- success, false -- failed.
     * *******************************************************************/
    bool start();

    /*********************************************************************
     * description: stop broker server
     * *******************************************************************/
    void stop();

private:
    std::shared_ptr<BrokerHandleImpl> m_sp4BrokerHdImpl = nullptr; // the concrete instance for broker implementation
};

} // namespace broker
} // namespace vsoa_sdk

