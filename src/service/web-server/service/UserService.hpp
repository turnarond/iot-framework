#pragma once

#include "db/UserDb.hpp"
#include "dto/UserDto.hpp"
#include "dto/StatusDto.hpp"
#include "dto/PageDto.hpp"
#include "vsoa_dto/core/Types.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 * 用户服务类
 */
class UserService {
private:
    typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<UserDb>, userDb);

public:
    UserService() = default;

    /**
     * 获取所有用户（分页）
     */
    vsoa::Object<UserPageDto> getUsers(vsoa::UInt32 page, vsoa::UInt32 size);

    /**
     * 根据ID获取用户
     */
    vsoa::Object<UserDto> getUserById(vsoa::UInt32 id);

    /**
     * 根据用户名获取用户
     */
    vsoa::Object<UserDto> getUserByUsername(const vsoa::String& username);

    /**
     * 创建用户
     */
    vsoa::Object<UserDto> createUser(const vsoa::Object<UserRequestDto>& userRequest);

    /**
     * 更新用户
     */
    vsoa::Object<UserDto> updateUser(const vsoa::Object<UserRequestDto>& userRequest);

    /**
     * 删除用户
     */
    vsoa::Object<StatusDto> deleteUser(vsoa::UInt32 id);

    /**
     * 获取所有角色
     */
    vsoa::Vector<vsoa::Object<RoleDto>> getRoles();

    /**
     * 根据ID获取角色
     */
    vsoa::Object<RoleDto> getRoleById(vsoa::UInt32 id);

    /**
     * 验证密码是否符合要求
     */
    bool validatePassword(const std::string& password);

    /**
     * 对密码进行哈希处理
     */
    std::string hashPassword(const std::string& password);

    /**
     * 验证密码是否正确
     */
    bool verifyPassword(const std::string& password, const std::string& hashedPassword);
};
