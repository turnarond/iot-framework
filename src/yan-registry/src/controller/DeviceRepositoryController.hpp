/*
 *  Created on: 2018.07.02
 *  Author: yanchaodong
 */

#pragma once

#include "dto/RegistrationDto.hpp"
#include "dto/StatusDto.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "service/RegistrationService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class RegistrationController : public oatpp::web::server::api::ApiController 
{

private:
  std::shared_ptr<RegistrationService> m_service;

public:
    RegistrationController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                            objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    static std::shared_ptr<RegistrationController> createShared(OATPP_COMPONENT(
        std::shared_ptr<ObjectMapper>,
        objectMapper) // Inject objectMapper component here as default parameter
    ) {
        return std::make_shared<RegistrationController>(objectMapper);
    }

    // —————— 预注册 API（管理端） ——————
    ENDPOINT_INFO(preRegister) 
    {
        info->summary = "Pre-register a device (admin/factory)";
        info->addConsumes<oatpp::Object<PreRegisterRequestDto>>("application/json");
        info->addResponse<oatpp::Object<StatusDto>>(Status::CODE_201, "application/json");
        info->addResponse<oatpp::Object<StatusDto>>(Status::CODE_400, "application/json");
        info->addResponse<oatpp::Object<StatusDto>>(Status::CODE_401,
                                            "application/json"); // 未授权
    }
    ENDPOINT("POST", "api/v1/devices/add", preRegister,
            HEADER(String, authHeader, "Authorization"), // ← 提取 Authorization
            BODY_DTO(oatpp::Object<PreRegisterRequestDto>, request)) 
    {

        // 🔑 验证 API Key 或 Bearer Token（简化：检查前缀）
        if (!authHeader || authHeader->find("Bearer ") != 0) {
        auto error = StatusDto::createShared();
        error->message = "Missing or invalid Authorization header";
        return createDtoResponse(Status::CODE_401, error);
        }

        auto result = m_service->preRegister(request);
        if (result->code) {
            return createDtoResponse(Status::CODE_400, result);
        }
        return createDtoResponse(Status::CODE_201, result);
    }

    // -----删除i设备----
    ENDPOINT_INFO(deleteDevice) 
    {
        info->summary = "Delete Device by deviceId";

        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");

        info->pathParams["deviceId"].description = "Device Identifier";
    }
    ENDPOINT("DELETE", "api/v1/devices/{deviceId}", deleteDevice, 
            HEADER(String, authHeader, "Authorization"), // ← 提取 Authorization
            PATH(Int32, deviceId)) {
        // 🔑 验证 API Key 或 Bearer Token（简化：检查前缀）
        if (!authHeader || authHeader->find("Bearer ") != 0) {
            auto error = StatusDto::createShared();
            error->message = "Missing or invalid Authorization header";
            return createDtoResponse(Status::CODE_401, error);
        }

        auto result = m_service->deleteDevice(deviceId);
        if (result->code) {
            return createDtoResponse(Status::CODE_400, result);
        }

        return createDtoResponse(Status::CODE_200, result);
    }

  // —————— 设备注册 API（边缘设备） ——————
  ENDPOINT_INFO(deviceRegister) {
    info->summary = "Device self-registration (zero-trust, MUST use HTTPS)";
    info->addConsumes<oatpp::Object<DeviceRegisterRequestDto>>("application/json");
    info->addResponse<String>(Status::CODE_200,
                              "application/pkcs7-mime"); // SMIME 二进制
    info->addResponse<Object<StatusDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_404,
                                         "application/json"); // SN 未预注册
  }
  ENDPOINT("POST", "api/v1/register", deviceRegister,
           BODY_DTO(oatpp::Object<DeviceRegisterRequestDto>, request)) {

    // ⚠️ 注意：此处不验证证书！因为设备尚无凭证
    // 安全性由 HTTPS 传输层 + 预注册公钥验证保证

    auto result = m_service->deviceRegister(request);
    if (result->code) {
      return createDtoResponse(Status::CODE_400, result);
    }

    // 返回 SMIME 凭证（二进制）
    auto smime = m_service->generateSMIME(request); // ← 实际生成逻辑
    auto response = createResponse(Status::CODE_200, smime);
    response->putHeader("Content-Type", "application/pkcs7-mime");
    response->putHeader("Content-Disposition",
                    "attachment; filename=\"credential.p7m\"");

    return response;
  }
};

#include OATPP_CODEGEN_END(ApiController)