/*
 * @Author: yanchaodong
 * @Date: 2026-02-06 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-06 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/UserController.hpp
 * @Description: 用户控制器，用于处理用户相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef USER_CONTROLLER_HPP
#define USER_CONTROLLER_HPP

#include "dto/StatusDto.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/core/parser/ParsingError.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/UserDto.hpp"
#include "service/UserService.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 用户控制器
 * 处理用户相关的API请求
 */
class UserController : public vsoa::web::server::api::ApiController {
private:
    UserService userService; ///< 用户服务实例，用于处理业务逻辑

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    UserController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {}

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<UserController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<UserController>(objectMapper);
    }

    // 用户相关端点
    /**
     * 获取所有用户
     * @return 用户列表
     */
    ENDPOINT_INFO(getUsers) {
        info->summary = "获取所有用户";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码，默认为1";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量，默认为10";
        info->addResponse<vsoa::Object<UserPageDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::Vector<vsoa::Object<UserDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/users", getUsers, 
        QUERY(vsoa::UInt32, page), 
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/users called with page=%u, size=%u", 
            page.getValue(1), size.getValue(10));
        auto response = userService.getUsers(page, size); 
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/users returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取用户
     * @param id 用户ID
     * @return 用户信息
     */
    ENDPOINT_INFO(getUserById) {
        info->summary = "根据ID获取用户";
        info->queryParams.add<vsoa::UInt32>("id").description = "用户ID";
        info->addResponse<vsoa::Object<UserDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/users", getUserById, 
        QUERY(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/users called with id=%d", 
            id.getValue(-1));
        auto response = userService.getUserById(id);
        if (!response) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[UserController] User not found with id=%d", 
                id.getValue(-1));
            return createResponse(Status::CODE_404, "用户不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/users returned successfully for id=%d", 
            id.getValue(-1));
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 创建用户
     * @param userRequest 用户创建数据
     * @return 创建的用户信息
     */
    ENDPOINT_INFO(createUser) {
        info->summary = "创建用户";
        info->addConsumes<vsoa::Object<UserRequestDto>>("application/json");
        info->addResponse<vsoa::Object<UserDto>>(Status::CODE_201, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/users", createUser, 
        BODY_DTO(vsoa::Object<UserRequestDto>, userRequest)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] POST /api/users called to create user");
        auto response = userService.createUser(userRequest);
        if (!response) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[UserController] Failed to create user");
            return createResponse(Status::CODE_400, "创建用户失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] User created successfully");
        return createDtoResponse(Status::CODE_201, response);
    }

    /**
     * 更新用户
     * @param userRequest 用户更新数据
     * @return 更新后的用户信息
     */
    ENDPOINT_INFO(updateUser) {
        info->summary = "更新用户";
        info->addConsumes<vsoa::Object<UserRequestDto>>("application/json");
        info->addResponse<vsoa::Object<UserDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/users", updateUser, 
        BODY_DTO(vsoa::Object<UserRequestDto>, userRequest)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] PUT /api/users called to update user");
        auto response = userService.updateUser(userRequest);
        if (!response) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[UserController] User not found for update with name=%s", 
                userRequest->username->c_str());
            return createResponse(Status::CODE_404, "用户不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] User updated successfully with name=%s", 
            userRequest->username->c_str());
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 删除用户
     * @param id 用户ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteUser) {
        info->summary = "删除用户";
        info->queryParams.add<vsoa::UInt32>("id").description = "用户ID";
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_404, "application/json");
    }
    ENDPOINT("DELETE", "/api/users", deleteUser,
        QUERY(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] DELETE /api/users called with id=%d", 
            id.getValue(-1));
        auto status = userService.deleteUser(id.getValue(-1));
        if (status->code == 404) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[UserController] User not found with id=%d", 
                id.getValue(-1));
            return createDtoResponse(Status::CODE_404, status);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] User deleted successfully with id=%d", 
            id.getValue(-1));
        return createDtoResponse(Status::CODE_204, status);
    }

    // 角色相关端点
    /**
     * 获取所有角色
     * @return 角色列表
     */
    ENDPOINT_INFO(getRoles) {
        info->summary = "获取所有角色";
        info->addResponse<vsoa::Vector<vsoa::Object<RoleDto>>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/roles", getRoles) {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/roles called");
        auto response = userService.getRoles();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/roles returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取角色
     * @param id 角色ID
     * @return 角色信息
     */
    ENDPOINT_INFO(getRoleById) {
        info->summary = "根据ID获取角色";
        info->queryParams.add<vsoa::UInt32>("id").description = "角色ID";
        info->addResponse<vsoa::Object<RoleDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/roles", getRoleById, 
        QUERY(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/roles called with id=%d", 
            id.getValue(-1));
        auto response = userService.getRoleById(id);
        if (!response) {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[UserController] Role not found with id=%d", 
                id.getValue(-1));
            return createResponse(Status::CODE_404, "角色不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[UserController] GET /api/roles returned successfully for id=%d", 
            id.getValue(-1));
        return createDtoResponse(Status::CODE_200, response);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif /* USER_CONTROLLER_HPP */
