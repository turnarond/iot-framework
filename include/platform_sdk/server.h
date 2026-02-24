/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: server.h .
*
* Date: 2024-02-19
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_SERVER_H_
#define _VSOA_SDK_SERVER_H_

#include "lwcomm/lwcomm.h"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "platform_sdk/status.hpp"
#include "platform_sdk/macro.h"
#include "encoding/encoder_provider.h"

#include <cstdint>
#include <functional>
#include <map>

namespace vsoa_sdk
{
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> CreateObjMapper();

namespace server
{
    class ServerHandleImpl;
    class ServerHandle;

    typedef struct ClientRpcInfo {
        std::string url;
        uint32_t cid;
        uint32_t seqno;
        uint8_t method;
    } CliRpcInfo;

    using PfunServerCmd = std::function<void(CliRpcInfo &cli_info, const void *dto, size_t len, void *arg)>;
    using OnConnect = std::function<void(ServerHandle *server, uint32_t cid, bool connect, void *arg)>;

    class LWCOMM_API ServerHandle
    {
    public:
        /* Set server host, need call before create server. */
        void SetServerHost(std::string &host);

        /* Set server port, need call before create server. */
        void SetServerPort(uint16_t port);

        /* Set server authority */
        void SetServerAuthority(VSOA_SERVER_AUTHORITY authority); 

        /* Set encoding provider, need call before create server. */
        void SetEncodingProvider(encoding::EncoderProvider *provider);

        /* Add RPC server listener. */
        void AddRpcListener(std::string url, PfunServerCmd pfn, void *arg);

        /* Set OnConnect callback. */
        void SetOnConnectCb(OnConnect cb, void* arg);

        /* Create server. */
        int CreateServer(void);

        /* Return client numbers of this server */
        uint32_t OnlineClientNums(void);

        /* Create server. */
        int CreateServer(PfunServerCmd pfn, void *arg);

        /* Get server port, need call after create server. */
        uint16_t GetServerPort(void);

        /* Get server name, need call after create server */
        std::string GetServerName(void);

        /* Get host of client peer, need call after create server. */
        int GetPeerCliHost(unsigned int cid, std::string &host);

        /* Destroy server. */
        int DestroyServer(void);

        /* Service response, call in rpc callback. */
        int SrvResponse(CliRpcInfo &rpc_info, const vsoa::Status &status, const vsoa::String &str);

        /* Service response, call in rpc callback. */
        int SrvResponse(CliRpcInfo &rpc_info, const vsoa::Status &status, const void *response, const size_t len);

        /* Service spin, default wait time period is 1000ms. */ 
        void SrvSpin(int64_t ms = 1000);

        /* Service spin once, default wait time period is 1000ms. */ 
        void SrvSpinOnce(int64_t ms = 1000);

        /* Service spin async, default wait time period is 1000ms. */ 
        void SrvSpinAsync(int64_t ms = 1000);

        int Publish(const std::string &url, const std::string &dto);

        /* Publish with vsoa::String type data. */ 
        int Publish(std::string &&url, const std::string &dto);

        /* Publish with void* type data and length is len. */ 
        int Publish(std::string &&url, const void *data, const size_t len);

        /* Add Datagram server listener. */
        void AddDatagramListener(PfunServerCmd pfn, void *arg);

        /* SendDatagram to client. */
        int PublishDatagram(uint32_t cid, std::string url, std::string data);
    
        /* SendDatagram to client. */
        int PublishDatagram(uint32_t cid, std::string &&url, const void *data, const size_t len);

    private:
        ServerHandleImpl* srv_handle_impl;
    
    public:
        ServerHandle(const std::string &sh);
        ServerHandle(const std::string &sh, const std::string &passwd);
        ~ServerHandle();
    };

} // namespace server
} // namespace vsoa_sdk

#endif //_VSOA_SDK_SERVER_H_

/*
 * end
 */