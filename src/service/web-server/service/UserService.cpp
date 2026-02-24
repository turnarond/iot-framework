/*
 * @Author: yanchaodong
 * @Date: 2026-02-06 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-06 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/UserService.cpp
 * @Description: User Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "UserService.hpp"
#include "common/Logger.hpp"
#include "dto/UserDto.hpp"
#include "lwlog/lwlog.h"
#include "vsoa_dto/core/Types.hpp"
#include <regex>
#include <chrono>

#include "dto/PageDto.hpp"

/**
 * 获取所有用户
 */
vsoa::Object<UserPageDto> UserService::getUsers(vsoa::UInt32 page, vsoa::UInt32 size) 
{
    auto dbResult = userDb->getUserCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];
    
    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }   
    
    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    
    dbResult = userDb->getUsers(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    auto items = dbResult->fetch<vsoa::Vector<vsoa::Object<UserDto>>>();

    auto dto = UserPageDto::createShared();
    dto->items = items;
    dto->counts = items->size();
    dto->pages = pages;
    dto->page = page;
    dto->size = size;
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Get Users page=%u size=%u, returned=%u", page, size, dto->counts);
    return dto;
}

/**
 * 根据ID获取用户
 */
vsoa::Object<UserDto> UserService::getUserById(vsoa::UInt32 id) 
{
    auto result = userDb->getUserById(id);
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    OATPP_ASSERT_HTTP(result->hasMoreToFetch(), Status::CODE_404, "User not found");
    auto users = result->fetch<vsoa::Vector<vsoa::Object<UserDto>>>();
    return users->front();
}

/**
 * 根据用户名获取用户
 */
vsoa::Object<UserDto> UserService::getUserByUsername(const vsoa::String& username) 
{
    auto result = userDb->getUserByUsername(username);
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    OATPP_ASSERT_HTTP(result->hasMoreToFetch(), Status::CODE_404, "User not found");
    auto users = result->fetch<vsoa::Vector<vsoa::Object<UserDto>>>();
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Get User by username=%s", username->c_str());
    return users->front();
}

/**
 * 创建用户
 */
vsoa::Object<UserDto> UserService::createUser(const vsoa::Object<UserRequestDto>& userRequest) 
{
    // 验证密码
    OATPP_ASSERT_HTTP(validatePassword(userRequest->password), Status::CODE_400, "Password must be at least 8 characters, containing uppercase, lowercase letters and numbers");
    
    // 生成密码哈希
    std::string passwordHash = hashPassword(userRequest->password);
    
    // 获取当前时间戳
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    vsoa::Object<UserDto> userDto = UserDto::createShared();
    userDto->username = userRequest->username;
    userDto->password = passwordHash;
    userDto->role_id = userRequest->role_id;
    userDto->role_name = userRequest->role_name;
    userDto->role_cname = userRequest->role_cname;
    userDto->created_at = timestamp;
    userDto->enable = userRequest->enable;
    // 创建用户
    auto result = userDb->createUser(userDto);
    
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    
    // 获取创建的用户信息
    auto newId = vsoa::sqlite::Utils::getLastInsertRowId(result->getConnection());
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Create User %s success, id=%d", 
        userRequest->username->c_str(), newId);
    return getUserById(newId);
}

/**
 * 更新用户
 */
vsoa::Object<UserDto> UserService::updateUser(const vsoa::Object<UserRequestDto>& userRequest) 
{
    // 验证密码
    OATPP_ASSERT_HTTP(validatePassword(userRequest->password), Status::CODE_400, "Password must be at least 8 characters, containing uppercase, lowercase letters and numbers");
    
    // 生成密码哈希
    std::string passwordHash = hashPassword(userRequest->password);
    
    // 获取当前时间戳
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    vsoa::Object<UserDto> userDto = UserDto::createShared();
    userDto->username = userRequest->username;
    userDto->password = passwordHash;
    userDto->role_id = userRequest->role_id;
    userDto->role_name = userRequest->role_name;
    userDto->role_cname = userRequest->role_cname;
    userDto->updated_at = timestamp;
    userDto->enable = userRequest->enable;

    // 更新用户
    auto result = userDb->updateUser(userDto);
    
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    
    // 获取刚刚插入的行的ID 
    auto userId = vsoa::sqlite::Utils::getLastInsertRowId(result->getConnection());
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Update User id=%d success", userId);
    
    return getUserById(userId);
}

/**
 * 删除用户
 */
vsoa::Object<StatusDto> UserService::deleteUser(vsoa::UInt32 id) 
{
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (!getUserById(id)) {
        status->message = "User not found";
        status->code = 404;
        return status;
    }

    auto result = userDb->deleteUser(id);
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    
    status->message = "OK";
    status->code = 200;
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Delete User success, id=%d", id);
    return status;
}

/**
 * 获取所有角色
 */
vsoa::Vector<vsoa::Object<RoleDto>> UserService::getRoles() 
{
    auto result = userDb->getRoles();
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    return result->fetch<vsoa::Vector<vsoa::Object<RoleDto>>>();
}

/**
 * 根据ID获取角色
 */
vsoa::Object<RoleDto> UserService::getRoleById(vsoa::UInt32 id) 
{
    auto result = userDb->getRoleById(id);
    OATPP_ASSERT_HTTP(result->isSuccess(), Status::CODE_500, result->getErrorMessage());
    OATPP_ASSERT_HTTP(result->hasMoreToFetch(), Status::CODE_404, "Role not found");
    auto roles = result->fetch<vsoa::Vector<vsoa::Object<RoleDto>>>();
    return roles[0];
}

/**
 * 验证密码是否符合要求
 */
bool UserService::validatePassword(const std::string& password) 
{
    // 密码长度至少8位
    if (password.length() < 8) {
        return false;
    }
    
    // 检查是否包含大写字母
    bool hasUpper = false;
    // 检查是否包含小写字母
    bool hasLower = false;
    // 检查是否包含数字
    bool hasDigit = false;
    
    for (char c : password) {
        if (std::isupper(c)) {
            hasUpper = true;
        } else if (std::islower(c)) {
            hasLower = true;
        } else if (std::isdigit(c)) {
            hasDigit = true;
        }
    }
    
    return hasUpper && hasLower && hasDigit;
}

/**
 * 对密码进行哈希处理
 */
std::string UserService::hashPassword(const std::string& password) {
    // 注意：实际系统中应使用安全的哈希算法，如bcrypt
    // 这里为了示例，直接返回密码
    return password;
}

/**
 * 验证密码是否正确
 */
bool UserService::verifyPassword(const std::string& password, const std::string& hashedPassword) {
    // 注意：实际系统中应使用安全的哈希算法进行验证
    // 这里为了示例，直接比较密码
    return password == hashedPassword;
}
