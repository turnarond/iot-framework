#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

// —————— 预注册（管理端） ——————
class PreRegisterRequestDto : public oatpp::DTO {
  DTO_INIT(PreRegisterRequestDto, DTO)
  DTO_FIELD(String, serial_number, "serial_number");   // 必填，唯一
  DTO_FIELD(String, model, "model");                   // 必填
  DTO_FIELD(String, owner, "owner") = nullptr;         // 可选
  DTO_FIELD(String, public_key_pem, "public_key_pem"); // 必填（用于验证 CSR）
};

// —————— 设备注册（边缘设备） ——————
class DeviceRegisterRequestDto : public oatpp::DTO {
  DTO_INIT(DeviceRegisterRequestDto, DTO)
  DTO_FIELD(String, serial_number, "serial_number");   // 必填
  DTO_FIELD(String, csr_pem, "csr_pem");               // 必填（PEM 格式 CSR）
  DTO_FIELD(Int64, timestamp, "timestamp");            // 必填（Unix 秒，防重放 ±5min）
};

#include OATPP_CODEGEN_END(DTO)
