/*
 * @Author: yanchaodong
 * @Date: 2026-02-15 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-15 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/server_mgr/src/service/ServiceManagerService.hpp
 * @Description: Service Manager Service for business logic
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef SERVICE_MANAGER_SERVICE_HPP
#define SERVICE_MANAGER_SERVICE_HPP

#include "../servermgr.h"
#include "../dto/StatusDto.hpp"
#include "../dto/ServiceInfoDto.hpp"
#include "vsoa_dto/core/Types.hpp"

class ServiceManagerService {
private:
    ServerManager& serverManager;
    
public:
    /**
     * 构造函数
     * @param serverManager 服务器管理器实例
     */
    ServiceManagerService(ServerManager& serverManager);
    
    /**
     * 获取所有服务列表
     * @return 服务信息列表
     */
    vsoa::List<vsoa::Object<ServiceInfoDto>> getServices();
    
    /**
     * 启动服务
     * @param serviceName 服务名称
     * @return 操作结果
     */
    vsoa::Object<StatusDto> startService(const vsoa::String& serviceName);
    
    /**
     * 停止服务
     * @param serviceName 服务名称
     * @return 操作结果
     */
    vsoa::Object<StatusDto> stopService(const vsoa::String& serviceName);
    
    /**
     * 重启服务
     * @param serviceName 服务名称
     * @return 操作结果
     */
    vsoa::Object<StatusDto> restartService(const vsoa::String& serviceName);
    
    /**
     * 重新加载配置
     * @return 操作结果
     */
    vsoa::Object<StatusDto> reloadConfig();
};

#endif /* SERVICE_MANAGER_SERVICE_HPP */
