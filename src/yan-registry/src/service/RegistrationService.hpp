/*
 * RegistrationService.hpp
 */

#pragma once

#include "db/DeviceRegistrationDb.hpp"
#include "dto/RegistrationDto.hpp"
#include "dto/StatusDto.hpp"

#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include <oatpp/core/Types.hpp>

class RegistrationService {

private:
  typedef oatpp::web::protocol::http::Status Status;

private:
  OATPP_COMPONENT(std::shared_ptr<DeviceRegistrationDb>,
                  m_database); // Inject database component

public:
  oatpp::Object<StatusDto> preRegister(const oatpp::Object<PreRegisterRequestDto> request);
  oatpp::Object<StatusDto> deleteDevice(const oatpp::Int32 &id);

  // 设备注册，a边缘端
  oatpp::Object<StatusDto> deviceRegister(const oatpp::Object<DeviceRegisterRequestDto> request);

  oatpp::String generateSMIME(const oatpp::Object<DeviceRegisterRequestDto> request);
  // oatpp::Object<UserDto> createUser(const oatpp::Object<UserDto>& dto);
  // oatpp::Object<UserDto> updateUser(const oatpp::Object<UserDto>& dto);
  // oatpp::Object<UserDto> getUserById(const oatpp::Int32& id, const
  // oatpp::provider::ResourceHandle<oatpp::orm::Connection>& connection =
  // nullptr); oatpp::Object<PageDto<oatpp::Object<UserDto>>> getAllUsers(const
  // oatpp::UInt32& offset, const oatpp::UInt32& limit);
  // oatpp::Object<StatusDto> deleteUserById(const oatpp::Int32& id);

public:
    RegistrationService() = default;
    ~RegistrationService() = default;
};