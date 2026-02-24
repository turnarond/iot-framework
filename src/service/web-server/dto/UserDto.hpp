#ifndef USER_DTO_HPP
#define USER_DTO_HPP

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * @brief 用户数据传输对象
 */
class UserDto : public vsoa::DTO {
public:
    DTO_INIT(UserDto, DTO)
    
    DTO_FIELD(UInt32, id, "id");                    ///< 用户ID
    DTO_FIELD(String, username, "username");        ///< 用户名
    DTO_FIELD(String, password, "password");        ///< 密码（仅用于创建/更新，不返回给前端）
    DTO_FIELD(UInt32, role_id, "role_id");          ///< 角色ID
    DTO_FIELD(String, role_name, "role_name");      ///< 角色名称（用于返回给前端）
    DTO_FIELD(String, role_cname, "role_cname");    ///< 角色中文名称（用于返回给前端）
    DTO_FIELD(Boolean, enable, "enable");            ///< 是否启用
    DTO_FIELD(UInt64, created_at, "created_at");    ///< 创建时间
    DTO_FIELD(UInt64, updated_at, "updated_at");    ///< 更新时间

    DTO_FIELD_INFO(updated_at) {
        info->description = "更新时间";
        info->required = false;
    }
};

/**
 * @brief 用户分页数据传输对象
 */
class UserPageDto : public PageDto<vsoa::Object<UserDto>> {
public:
    DTO_INIT(UserPageDto, PageDto<vsoa::Object<UserDto>>)
};

/**
 * @brief 用户创建/更新请求对象
 */
class UserRequestDto : public vsoa::DTO 
{
    DTO_INIT(UserRequestDto, DTO)
    
    DTO_FIELD(String, username, "username");        ///< 用户名
    DTO_FIELD(String, password, "password");        ///< 密码
    DTO_FIELD(UInt32, role_id, "role_id");          ///< 角色ID
    DTO_FIELD(String, role_name, "role_name");      ///< 角色名称
    DTO_FIELD(String, role_cname, "role_cname");    ///< 角色中文名称
    DTO_FIELD(Boolean, enable, "enable");            ///< 是否启用
};

/**
 * @brief 角色数据传输对象
 */
class RoleDto : public vsoa::DTO 
{
    DTO_INIT(RoleDto, DTO)
    
    DTO_FIELD(UInt32, id, "id");                    ///< 角色ID
    DTO_FIELD(String, name, "name");                ///< 角色名称
    DTO_FIELD(String, cname, "cname");              ///< 角色中文名称
    DTO_FIELD(String, description, "description");  ///< 角色描述
};

/**
 * @brief 系统配置数据传输对象
 */
class SystemConfigDto : public vsoa::DTO 
{
    DTO_INIT(SystemConfigDto, DTO)
    
    DTO_FIELD(UInt32, id, "id");                    ///< 配置ID
    DTO_FIELD(String, config_key, "config_key");    ///< 配置键
    DTO_FIELD(String, config_value, "config_value");///< 配置值
    DTO_FIELD(String, description, "description");  ///< 配置描述
    DTO_FIELD(UInt64, updated_at, "updated_at");    ///< 更新时间
};

/**
 * @brief 系统配置更新请求对象
 */
class SystemConfigRequestDto : public vsoa::DTO 
{
    DTO_INIT(SystemConfigRequestDto, DTO)
    
    DTO_FIELD(String, config_key, "config_key");    ///< 配置键
    DTO_FIELD(String, config_value, "config_value");///< 配置值
};

/**
 * @brief NTP配置数据传输对象
 */
class NtpConfigDto : public vsoa::DTO 
{
    DTO_INIT(NtpConfigDto, DTO)
    
    DTO_FIELD(String, server, "server");            ///< NTP服务器地址
    DTO_FIELD(Boolean, enabled, "enabled");         ///< 是否启用自动时间同步
    DTO_FIELD(String, current_time, "current_time");///< 当前系统时间
    DTO_FIELD(String, sync_status, "sync_status");  ///< 同步状态
};

/**
 * @brief 网络配置数据传输对象
 */
class NetworkConfigDto : public vsoa::DTO 
{
    DTO_INIT(NetworkConfigDto, DTO)
    
    DTO_FIELD(String, mode, "mode");                ///< 网络模式（static/dhcp）
    DTO_FIELD(String, ip_address, "ip_address");    ///< IP地址
    DTO_FIELD(String, subnet_mask, "subnet_mask");  ///< 子网掩码
    DTO_FIELD(String, gateway, "gateway");          ///< 网关
    DTO_FIELD(String, dns1, "dns1");                ///< 首选DNS
    DTO_FIELD(String, dns2, "dns2");                ///< 备用DNS
};

#include VSOA_CODEGEN_END(DTO)

#endif // USER_DTO_HPP