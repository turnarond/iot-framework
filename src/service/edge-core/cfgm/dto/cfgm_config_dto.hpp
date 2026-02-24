/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: cfgm_config_dto.h .
 *
 * Date: 2025-12-17
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

ENUM  
(
    InterFaceType, 
    v_uint8,
    VALUE(ETHERNET, 1),
    VALUE(WIFI, 2),
    VALUE(VIRTUAL, 3),
    VALUE(LOOPBACK, 4),
    VALUE(BLUETOOTH, 5),
    VALUE(USB, 6),
    VALUE(UNKNOWN, 7)
)

/*
 * 边缘注册信息，上报注册时携带
 */
class RegistrationConfigDTO : public vsoa::DTO {
    DTO_INIT(RegistrationConfigDTO, DTO)

    DTO_FIELD(String, deviceId);
    DTO_FIELD(String, deviceName);
    DTO_FIELD(String, deviceType);
    DTO_FIELD(String, manufaturer);
    DTO_FIELD(String, projectId);
    DTO_FIELD(String, location);
    DTO_FIELD(Int32, registratioBrokerId);
    DTO_FIELD(String, firmwareVersion);
};

/*
 * 边缘注册broker信息
 */
class RegistrationBrokerDto : public vsoa::DTO {
    DTO_INIT(RegistrationBrokerDto, DTO)

    DTO_FIELD(String, brokerName);
    DTO_FIELD(String, endpoint);
    DTO_FIELD(Int32, port);
    DTO_FIELD(String, username);
    DTO_FIELD(String, password);
    DTO_FIELD(Boolean, sslEnable);
    DTO_FIELD(Int32, keepalive);
    DTO_FIELD(Int32, createdTime);
};

/* 
 * 云端下发的服务凭证
 */
class ServiceCredentialDTO : public vsoa::DTO {
    DTO_INIT(ServiceCredentialDTO, DTO)

    DTO_FIELD(String, serviceEndpoint);
    DTO_FIELD(Int32, servicePort);
    DTO_FIELD(String, serviceUsername);
    DTO_FIELD(String, serviceToken);
    DTO_FIELD(Int32, issuedTime);
    DTO_FIELD(Int32, expiresTime);
    DTO_FIELD(Int32, createdTime);
};

/*
 * 网络接口配置信息
 */
class NetworkInterfaceConfigDTO : public vsoa::DTO {
    DTO_INIT(NetworkInterfaceConfigDTO, DTO)

    DTO_FIELD(String, interfaceName);
    DTO_FIELD(Enum<InterFaceType>, interfaceType);
    DTO_FIELD(String, macAddr);
    DTO_FIELD(Int32, mtu);
    DTO_FIELD(String, description);
    DTO_FIELD(Boolean, enabled);
    DTO_FIELD(Int32, created_time);
};

/*
 * IP 配置
 */
class IpConfigDTO : public vsoa::DTO {
    DTO_INIT(IpConfigDTO, DTO)

    DTO_FIELD(Int32, interfaceId);
    DTO_FIELD(String, interfaceName);
    DTO_FIELD(Int32, ipVersion);
    DTO_FIELD(String, addrType);
    DTO_FIELD(String, ipAddress);
    DTO_FIELD(String, subnetMask);
    DTO_FIELD(Int32, prefixLength);
    DTO_FIELD(String, gateway);
    DTO_FIELD(String, dnsServer);
    DTO_FIELD(String, searchDomain);
    DTO_FIELD(Int32, metric);
    DTO_FIELD(Boolean, enable);
    DTO_FIELD(Int32, createdTime);
    DTO_FIELD(Int32, updatedTime);
};

class LoggingConfigDTO : public vsoa::DTO
{
    DTO_INIT(LoggingConfigDTO, DTO)

    DTO_FIELD(String, level);
    DTO_FIELD(UInt32, maxFileSize);
    DTO_FIELD(UInt32, maxFiles);
    DTO_FIELD(Boolean, uploadEnabled);
    DTO_FIELD(String, uploadEndpoint);
    DTO_FIELD(UInt32, uploadInterval);
};

class ResourceConfigDTO : public vsoa::DTO
{
    DTO_INIT(ResourceConfigDTO, DTO)

    DTO_FIELD(Float32, cpuThreshold);
    DTO_FIELD(Float32, diskThreshold);
    DTO_FIELD(Boolean, enabled);
    DTO_FIELD(UInt32, interval);
    DTO_FIELD(Float32, memoryThreshold);
    DTO_FIELD(Boolean, reportChangesOnly);
};

class SystemConfigDTO : public vsoa::DTO
{
    DTO_INIT(SystemConfigDTO, DTO)

    DTO_FIELD(Object<RegistrationConfigDTO>, registration);
    DTO_FIELD(Object<RegistrationBrokerDto>, cloud);
    DTO_FIELD(Object<ServiceCredentialDTO>, serviceCredential);
    DTO_FIELD(Vector<Object<NetworkInterfaceConfigDTO>>, networkIf);
    DTO_FIELD(Vector<Object<IpConfigDTO>>, ipConfig);
    DTO_FIELD(String, databaseName);
    DTO_FIELD(Object<LoggingConfigDTO>, logging);
    DTO_FIELD(String, redisAddress);
};

#include VSOA_CODEGEN_END(DTO)