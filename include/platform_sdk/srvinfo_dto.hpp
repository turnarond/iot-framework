/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: srvinfo_dto.hpp .
*
* Date: 2024-02-21
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef _VSOA_SDK_SRVINFO_DTO_H_
#define _VSOA_SDK_SRVINFO_DTO_H_

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/*
 * Server Info DTO
 */
class SrvInfoDTO : public vsoa::DTO
{
    DTO_INIT(SrvInfoDTO, DTO)
    DTO_FIELD(String, server_name, "server_name");
    DTO_FIELD(String, host, "host");
    DTO_FIELD(Int32, port, "port");
    DTO_FIELD(Int16, authority, "authority");
    DTO_FIELD(List<String>, ip_list, "srvip list");
    DTO_FIELD(List<String>, publisher, "publisher");
    DTO_FIELD(List<String>, subscriber, "subscriber");
    DTO_FIELD(List<String>, listener, "listener");
    DTO_FIELD(List<String>, proxy_servers, "proxy_servers");
    DTO_FIELD_INFO (authority) {
        info->required = false;
    }
    DTO_FIELD_INFO (publisher) {
        info->required = false;
    }
    DTO_FIELD_INFO (subscriber) {
        info->required = false;
    }
    DTO_FIELD_INFO (listener) {
        info->required = false;
    }
    DTO_FIELD_INFO (proxy_servers) {
        info->required = false;
    }
};

class SrvInfoDtoPaga : public vsoa::DTO
{
    DTO_INIT(SrvInfoDtoPaga, DTO)
    DTO_FIELD(List<Object<SrvInfoDTO> >, srv_list, "SrvInfoDto list");
};

/*
 * URL Info DTO
 */
class UrlInfoDto : public vsoa::DTO
{
    DTO_INIT(UrlInfoDto, DTO)
    DTO_FIELD(String, url_name, "url_name");
};

/*
 * Server name list DTO.
 */
class ServerNameListDto : public vsoa::DTO
{
    DTO_INIT(ServerNameListDto, DTO)
    DTO_FIELD(List<String>, srvname_list, "server name list");
};

/*
 * Server Info DTO
 */
class SrvInfoOfUrl : public vsoa::DTO
{
    DTO_INIT(SrvInfoOfUrl, DTO)
    DTO_FIELD(List<Object<SrvInfoDTO> >, srvlist, "server_list");
};

/*
 * API Info DTO
 */
class ApiInfoDto : public vsoa::DTO
{
    DTO_INIT(ApiInfoDto, DTO)
    DTO_FIELD(String, type, "type");
    DTO_FIELD(String, url, "url");
    DTO_FIELD(String, desc, "description");
};

/*
 * Server Info DTO
 */
class SrvInfoDto : public vsoa::DTO
{
    DTO_INIT(SrvInfoDto, DTO)
    DTO_FIELD(String, server_name, "server_name");
    DTO_FIELD(String, host, "host");
    DTO_FIELD(Int32, port, "port");

    DTO_FIELD(String, desc, "description");
    DTO_FIELD(vsoa::List< vsoa::Object<ApiInfoDto> >, apis, "apis");
};

#include VSOA_CODEGEN_END(DTO)

#endif /* _VSOA_SDK_SRVINFO_DTO_H_ */

/*
 * end
 */