/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: message_generate.hpp .
 *
 * Date: 2025-12-19
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once

#include "vsoa_dto/core/Types.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

////////////////////////////////////////设备注册消息////////////////////////////////////////
/*
 * 消息头
 */
class HeaderDto : public vsoa::DTO 
{
    DTO_INIT(HeaderDto, DTO)
    DTO_FIELD(String, version, "version");
    DTO_FIELD(String, msgId, "msgId");
    DTO_FIELD(Int32, timestamp, "timestamp");
};

/*
 * 设备内部信息，上报注册时携带
 */
class DeviceInnerDto : public vsoa::DTO 
{
    DTO_INIT(DeviceInnerDto, DTO)
    DTO_FIELD(String, sn, "sn");
    DTO_FIELD(String, name, "name");
    DTO_FIELD(String, model, "model");
    DTO_FIELD(String, type, "type");
    DTO_FIELD(String, firmwareVersion, "firmwareVersion");
    DTO_FIELD_INFO(firmwareVersion) {
        info->required = false;
    }
    DTO_FIELD_INFO(type) {
        info->required = false;
    }
    DTO_FIELD_INFO(model) {
        info->required = false;
    }
};

/*
 * 网络接口配置
 */
class NetInterfaceDto : public vsoa::DTO 
{
    DTO_INIT(NetInterfaceDto, DTO)
    DTO_FIELD(String, interfaceName, "interfaceName");
    DTO_FIELD(String, interfaceType, "interfaceType");
    DTO_FIELD(String, ip, "ip");
    DTO_FIELD(String, subnet, "subnet");
    DTO_FIELD(String, gateway, "gateway");
    DTO_FIELD(String, macAddr, "macAddr");
    DTO_FIELD(Int32, mtu, "mtu");
    DTO_FIELD(String, description, "description");
    DTO_FIELD(Boolean, enabled, "enabled");

    DTO_FIELD_INFO(mtu) {
        info->required = false;
    }
    DTO_FIELD_INFO(description) {
        info->required = false;
    }
    DTO_FIELD_INFO(enabled) {
        info->required = false;
    }
};

/*
 * 网络接口配置
 */
class NetworkDto : public vsoa::DTO    
{
    DTO_INIT(NetworkDto, DTO)
    DTO_FIELD(vsoa::Object<NetInterfaceDto>, interfaces, "interfaces");
};

/*
 * 安全配置
 */
class SecurityDto : public vsoa::DTO
{
    DTO_INIT(SecurityDto, DTO)
    DTO_FIELD(String, csr, "csr");
};

/*
 * 设备能力配置
 */
class CapabilitiesDto : public vsoa::DTO
{
    DTO_INIT(CapabilitiesDto, DTO)
    DTO_FIELD(String, maxBandwidth, "maxBandwidth");
    DTO_FIELD(Boolean, supportOTA, "supportOTA");
    DTO_FIELD(Boolean, supportMQTT, "supportMQTT");
    DTO_FIELD(Boolean, supportHTTP, "supportHTTP");
    DTO_FIELD(Boolean, supportHTTPS, "supportHTTPS");
    DTO_FIELD(Boolean, supportCoAP, "supportCoAP");

    DTO_FIELD_INFO(maxBandwidth) {
        info->required = false;
    }
    DTO_FIELD_INFO(supportOTA) {
        info->required = false;
    }
    DTO_FIELD_INFO(supportMQTT) {
        info->required = false;
    }
    DTO_FIELD_INFO(supportHTTP) {
        info->required = false;
    }
    DTO_FIELD_INFO(supportHTTPS) {
        info->required = false;
    }
    DTO_FIELD_INFO(supportCoAP) {
        info->required = false;
    }
};

/*
 * 注册请求消息
 */
class RegistrationRequestMsgDto : public vsoa::DTO 
{
    DTO_INIT(RegistrationRequestMsgDto, DTO)
    DTO_FIELD(vsoa::Object<HeaderDto>, header, "hdr");
    DTO_FIELD(vsoa::Object<DeviceInnerDto>, device, "device");
    DTO_FIELD(vsoa::Object<NetworkDto>, network, "network");
    DTO_FIELD(vsoa::Object<SecurityDto>, security, "security");
    DTO_FIELD(vsoa::Object<CapabilitiesDto>, capabilities, "capabilities");
};

////////////////////////////////////////设备注册响应消息////////////////////////////////////////

/*
 * 云端下发证书信息
 */
class CloudCertDto : public vsoa::DTO {
    DTO_INIT(CloudCertDto, DTO)
    DTO_FIELD(String, deviceCert, "deviceCert");
    DTO_FIELD(String, caChain, "caChain");
    DTO_FIELD(UInt64, validFrom, "validFrom");
    DTO_FIELD(UInt64, validTo, "validTo");
    DTO_FIELD(UInt32, renewBefore, "renewBefore");
};

/*
 * 注册响应消息payload
 */
class RegistrationResponseDto : public vsoa::DTO {
    DTO_INIT(RegistrationResponseDto, DTO)
    DTO_FIELD(String, deviceId, "deviceId");
    DTO_FIELD(String, cloudEndpoint, "cloudEndpoint");
    DTO_FIELD(UInt16, cloudPort, "cloudPort");
    DTO_FIELD(String, username, "username");
    DTO_FIELD(String, password, "password");
    DTO_FIELD(vsoa::Object<CloudCertDto>, certInfo, "certInfo");
    DTO_FIELD_INFO(username) {
        info->required = false;
    }
    DTO_FIELD_INFO(password) {
        info->required = false;
    }
    DTO_FIELD_INFO(cloudPort) {
        info->required = false;
    }
};

/*
 * 注册响应消息
 */
class RegistrationResponseMsgDto : public vsoa::DTO {
    DTO_INIT(RegistrationResponseMsgDto, vsoa::DTO)
    DTO_FIELD(vsoa::Object<HeaderDto>, header, "hdr");
    DTO_FIELD(Int32, code, "code");
    DTO_FIELD(String, message, "message");
    DTO_FIELD(vsoa::Object<RegistrationResponseDto>, payload, "payload");
};

////////////////////////////////////////设备激活消息////////////////////////////////////////
#include VSOA_CODEGEN_END(DTO)