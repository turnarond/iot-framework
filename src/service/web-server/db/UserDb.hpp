#pragma once

#include "dto/UserDto.hpp"
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * 用户数据库操作类
 */
class UserDb : public vsoa::orm::DbClient {
public:
    UserDb(const std::shared_ptr<vsoa::orm::Executor>& executor) 
    : vsoa::orm::DbClient(executor) 
    {}

    /**
     * 获取所有用户
     */
    QUERY(getUsers, 
        "SELECT u.id, u.username, u.role_id, u.enable, u.created_at, u.updated_at, r.name as role_name, r.cname as role_cname "
        "FROM t_users u "
        "LEFT JOIN t_roles r ON u.role_id = r.id "
        "ORDER BY u.id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))
    
    /**
     * 获取用户总数
     */
    QUERY(getUserCount, 
        "SELECT COUNT(*) as count FROM t_users;"
        )
    
    /**
     * 根据ID获取用户
     */
    QUERY(getUserById, 
        "SELECT u.id, u.username, u.role_id, u.enable, u.created_at, u.updated_at, r.name as role_name, r.cname as role_cname "
        "FROM t_users u "
        "LEFT JOIN t_roles r ON u.role_id = r.id "
        "WHERE u.id = :id;",
        PARAM(vsoa::UInt32, id))
    
    /**
     * 根据用户名获取用户
     */
    QUERY(getUserByUsername, 
        "SELECT u.id, u.username, u.password_hash, u.role_id, u.enable, u.created_at, u.updated_at "
        "FROM t_users u "
        "WHERE u.username = :username;",
        PARAM(vsoa::String, username))
    
    /**
     * 创建用户
     */
    QUERY(createUser, 
        "INSERT INTO t_users (username, password_hash, role_id, enable, created_at, updated_at) "
        "VALUES (:userRequest.username, "
        ":userRequest.password, "
        ":userRequest.role_id, "
        ":userRequest.enable, "
        ":userRequest.created_at);",
        PARAM(vsoa::Object<UserDto>, userRequest))
    
    /**
     * 更新用户
     */
    QUERY(updateUser, 
        "UPDATE t_users "
        "SET username = :userDto.username, "
        "password_hash = :userDto.password, "
        "role_id = :userDto.role_id, "
        "enable = :userDto.enable, "
        "updated_at = :userDto.updated_at "
        "WHERE username = :userDto.username;",
        PARAM(vsoa::Object<UserDto>, userDto))
    
    /**
     * 删除用户
     */
    QUERY(deleteUser, 
        "DELETE FROM t_users "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))
    
    /**
     * 获取所有角色
     */
    QUERY(getRoles, 
        "SELECT id, name, cname, description "
        "FROM t_roles "
        "ORDER BY id")
    
    /**
     * 根据ID获取角色
     */
    QUERY(getRoleById, 
        "SELECT id, name, cname, description "
        "FROM t_roles "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))
};

/**
 * 系统配置数据库操作类
 */
class SystemConfigDb : public vsoa::orm::DbClient {
public:
    SystemConfigDb(const std::shared_ptr<vsoa::orm::Executor>& executor) 
    : vsoa::orm::DbClient(executor) 
    {}

    /**
     * 获取所有系统配置
     */
    QUERY(getSystemConfig, 
        "SELECT id, config_key, config_value, description, updated_at "
        "FROM t_system_config "
        "ORDER BY id")
    
    /**
     * 根据键获取系统配置
     */
    QUERY(getSystemConfigByKey, 
        "SELECT id, config_key, config_value, description, updated_at "
        "FROM t_system_config "
        "WHERE config_key = :config_key;",
        PARAM(vsoa::String, config_key))
    
    /**
     * 更新系统配置
     */
    QUERY(updateSystemConfig, 
        "UPDATE t_system_config "
        "SET config_value = :config_value, updated_at = :updated_at "
        "WHERE config_key = :config_key;",
        PARAM(vsoa::String, config_key),
        PARAM(vsoa::String, config_value),
        PARAM(vsoa::UInt64, updated_at))
    
    /**
     * 插入系统配置
     */
    QUERY(insertSystemConfig, 
        "INSERT INTO t_system_config (config_key, config_value, description, updated_at) "
        "VALUES (:config_key, :config_value, :description, :updated_at);",
        PARAM(vsoa::String, config_key),
        PARAM(vsoa::String, config_value),
        PARAM(vsoa::String, description),
        PARAM(vsoa::UInt64, updated_at))
};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen
