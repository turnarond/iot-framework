/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: client.h .
*
* Date: 2024-02-27
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_CLIENT_H_
#define _VSOA_SDK_CLIENT_H_

#include <string>

#include "platform_sdk/macro.h"
#include "platform_sdk/status.hpp"
#include "lwcomm/lwcomm.h"
#include "encoding/encoder_provider.h"

#include <functional>
#include <string_view>

namespace vsoa_sdk
{
namespace client
{
    class ClientHandleImpl;

    struct ClientContext;

    typedef struct ClientContext *CLIENT_CONTEXT;

    /* Function pointer type of client. */
    /* Callback of subscribe */
    typedef void (*PfunSubscribeCb)(std::string url, const void *dto, size_t len, void *arg);

    /* Callback of rpc response */
    typedef void (*PfunResponseCb)(const vsoa::Status &status, std::string url, const void *dto, size_t len, void *arg);

    /* Callback of connect change */
    typedef void (*PfunOnConnectCb)(bool connect, std::string servername, CLIENT_CONTEXT ctx, void *arg);
 
    /* Callback of vsoa Datagram */
    typedef void (*PfunOnDatagramCb)(std::string url, const void* dto, size_t len, void* arg);
    
    /* functional type of client. */
    /* Callback of subscribe */
    using SubscribeCb = std::function<void(const std::string& url, const void *dto, size_t len, void *arg)>;

    /* Callback of rpc response */
    using ResponseCb = std::function<void(const vsoa::Status &status, const std::string& url, const void *dto, size_t len, void *arg)>;

    /* Callback of connect change */
    using ConnectCb = std::function<void(bool connect, const std::string& servername, CLIENT_CONTEXT ctx, void *arg)>;

    /* Callback of vsoa Datagram */
    using DatagramCb = std::function<void(const std::string& url, const void* dto, size_t len, void* arg)>;

    /* 
     * RPC call method.
     */
    typedef enum RPC_CALL_METHOD_E {
        CLIENT_RPC_METHOD_GET = 0,
        CLIENT_RPC_METHOD_SET,
        CLIENT_RPC_METHOD_NUMS     /* The numbers of RPC METHOD. */
    } RPC_CALL_METHOD;

    /* 
     * Server connect information
     */
    typedef struct ServerConnInfo {
        std::string server_name;
        std::string passwd;
        VSOA_SERVER_AUTHORITY auth;
        std::vector<std::string> sub_urls;
        PfunSubscribeCb sub_cb;
        void *sub_arg;
        PfunOnConnectCb conn_cb;
        void *conn_arg;
        unsigned int keepalive;
        unsigned int conn_timeout;
        unsigned int reconn_delay;
        ServerConnInfo();
    } SrvConnInfo;

    typedef struct RpcResponseInfo {
        void *data;
        size_t len;
        vsoa::Status status;
    } RpcRespInfo;

    class LWCOMM_API ClientHandle
    {
    public:
        /* Subscribe all services in the same url */
        /* Callback with C type function point */
        int Subscribe(std::string url, PfunSubscribeCb cb, void *arg, encoding::EncoderProvider *provider = NULL);

        /* Callback with C++ type function object */
        int Subscribe(std::string url, SubscribeCb cb, 
                      void *arg, encoding::EncoderProvider *provider = NULL);

        /* Subscribe the url of ctx's server */
        /* Callback with C type function point */
        int Subscribe(CLIENT_CONTEXT ctx, std::string url, PfunSubscribeCb cb, void *arg,
                      encoding::EncoderProvider *provider = NULL);

        /* Callback with C++ type function object */
        int Subscribe(CLIENT_CONTEXT ctx, std::string url, SubscribeCb cb, 
                      void *arg, encoding::EncoderProvider *provider = NULL);

        /* Unsubscribe */
        int Unsubscribe(std::string url);

        /* When calling RPC Call more frequently, the following APIs are used, 
         * so you don't need to create client contexts frequently */
        CLIENT_CONTEXT CreateClient(SrvConnInfo &srv);

        /* Set serialize encoding provider */
        void SetEncodingProvider(CLIENT_CONTEXT cli, encoding::EncoderProvider *provider);

#ifdef USE_LINK_TRACER
        /* Set tracer enable. */
        void SetEnableTracer(void);

        /* Set tracer disable. */
        void SetDisableTracer(void);
#endif

        /* Set connect cb. */
        /* Callback with C type function point */
        void SetConnectCb(CLIENT_CONTEXT cli, PfunOnConnectCb, void *arg);

        /* Callback with C++ type function object */
        void SetConnectCb(CLIENT_CONTEXT cli, ConnectCb cb, void *arg);

        /* Set Datagram cb. */
        /* Callback with C type function point */
        void SetDatagramCb(CLIENT_CONTEXT cli, PfunOnDatagramCb, void *arg);

        /* Callback with C++ type function object */
        void SetDatagramCb(CLIENT_CONTEXT cli, DatagramCb, void *arg);

        /* Check whether the server is connected */
        static bool IsConnected(vsoa_sdk::client::CLIENT_CONTEXT);

        /* Call the RPC interface, parameter 1st is the result returned by CreateClient, 
         * and the data is returned in the cb callback function */
        /* Callback with C type function point */
        int Call(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, 
                 PfunResponseCb cb, void *arg, unsigned int timeout_ms = 1000, 
                 RPC_CALL_METHOD method = CLIENT_RPC_METHOD_SET);

        /* Callback with C++ type function object */
        int Call(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, ResponseCb cb,
                 void *arg, unsigned int timeout_ms = 1000, RPC_CALL_METHOD method = CLIENT_RPC_METHOD_SET);

        /* Call the RPC interface with SET method */
        /* Callback with C type function point */
        int RpcSet(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, 
                 PfunResponseCb cb, void *arg, unsigned int timeout_ms = 1000);

        /* Callback with C++ type function object */
        int RpcSet(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, 
                   ResponseCb cb, void *arg, unsigned int timeout_ms = 1000);

        /* Call the RPC interface with GET method */
        /* Callback with C type function point */
        int RpcGet(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, 
                 PfunResponseCb cb, void *arg, unsigned int timeout_ms = 1000);

        /* Callback with C++ type function object */
        int RpcGet(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, 
                   ResponseCb cb, void *arg, unsigned int timeout_ms = 1000);

        /* Sync call the RPC interface, parameter 1st is the result returned by CreateClient,
         * and the response is paragrament resp*/
        int CallSync(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, RpcRespInfo& resp,
                     unsigned int timeout_ms = 1000, RPC_CALL_METHOD method = CLIENT_RPC_METHOD_SET);

        /* Sync call the RPC interface with SET method */
        int RpcSyncSet(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, RpcRespInfo& resp,
                     unsigned int timeout_ms = 1000);

        /* Sync call the RPC interface with GET method */
        int RpcSyncGet(CLIENT_CONTEXT cli, std::string &url, const void *dto, size_t len, RpcRespInfo& resp,
                     unsigned int timeout_ms = 1000);

        /* Unref the response from GetResponse interface*/
        void UnrefResponse(CLIENT_CONTEXT cli, RpcRespInfo& resp);

        /* Send Datagram to client. */
        int PublishDatagram(CLIENT_CONTEXT, std::string &&url, const void *data, const size_t len);
        /* Send Datagram to client. */
        int PublishDatagram(CLIENT_CONTEXT, const std::string &url, const void *data, const size_t len);

        /* Release the Client context created by CreateClient */
        void DestroyClient(CLIENT_CONTEXT);

        /* Release all client context created by CreateClient */
        void DestroyClient(void);

    private:
        /* Call the RPC interface, which can be used when the call is infrequent 
         * and requires the content of SrvConnInfo to be constructed.
         * The data is returned in the cb callback function. */
        int Call(SrvConnInfo &host, std::string &url, const void *dto, size_t len, 
                 PfunResponseCb cb, void *arg, unsigned int timeout_ms = 1000,
                 RPC_CALL_METHOD method = CLIENT_RPC_METHOD_SET);

        /* Call the RPC interface; If you can't use this API if you don't call it frequently, 
         * you need to construct the content of SrvConnInfo.
         * And the data is received in GetResponse */
        int CallSync(SrvConnInfo &host, std::string &url, const void *dto, size_t len, 
                     RpcRespInfo& resp, unsigned int timeout_ms = 1000,
                     RPC_CALL_METHOD method = CLIENT_RPC_METHOD_SET);

    private:
        ClientHandleImpl *cli_handle_impl;

    public:
        /* Consturcture */
        ClientHandle(void);

        /* Desturcture */
        ~ClientHandle();
    };
}
}

#endif // _VSOA_SDK_CLIENT_H_

/*
 * end
 */
