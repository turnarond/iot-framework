/*
 * @Author: yanchaodong
 * @Date: 2026-02-15 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-15 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/server_mgr/src/service/ServiceManagerService.cpp
 * @Description: Service Manager Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "ServiceManagerService.hpp"

ServiceManagerService::ServiceManagerService(ServerManager& serverManager)
    : serverManager(serverManager) {
}

vsoa::List<vsoa::Object<ServiceInfoDto>> ServiceManagerService::getServices() {
    auto serviceList = serverManager.listServicesWithStatusAndTime();
    auto result = vsoa::List<vsoa::Object<ServiceInfoDto>>::createShared();
    
    for (const auto& [name, isRunning, startTime] : serviceList) {
        auto serviceInfo = vsoa::Object<ServiceInfoDto>::createShared();
        serviceInfo->name = vsoa::String(name.c_str());
        serviceInfo->running = vsoa::Boolean(isRunning);
        serviceInfo->status = vsoa::String(isRunning ? "running" : "stopped");
        serviceInfo->startTime = vsoa::String(startTime.c_str());
        result->push_back(serviceInfo);
    }
    
    return result;
}

vsoa::Object<StatusDto> ServiceManagerService::startService(const vsoa::String& serviceName) {
    auto status = vsoa::Object<StatusDto>::createShared();
    
    if (serverManager.startService(serviceName->c_str())) {
        status->code = 200;
        status->message = "Service started successfully";
    } else {
        status->code = 400;
        status->message = "Failed to start service";
    }
    
    return status;
}

vsoa::Object<StatusDto> ServiceManagerService::stopService(const vsoa::String& serviceName) {
    auto status = vsoa::Object<StatusDto>::createShared();
    
    if (serverManager.stopService(serviceName->c_str())) {
        status->code = 200;
        status->message = "Service stopped successfully";
    } else {
        status->code = 400;
        status->message = "Failed to stop service";
    }
    
    return status;
}

vsoa::Object<StatusDto> ServiceManagerService::restartService(const vsoa::String& serviceName) {
    auto status = vsoa::Object<StatusDto>::createShared();
    
    if (serverManager.restartService(serviceName->c_str())) {
        status->code = 200;
        status->message = "Service restarted successfully";
    } else {
        status->code = 400;
        status->message = "Failed to restart service";
    }
    
    return status;
}

vsoa::Object<StatusDto> ServiceManagerService::reloadConfig() {
    auto status = vsoa::Object<StatusDto>::createShared();
    
    if (serverManager.reloadConfig()) {
        status->code = 200;
        status->message = "Configuration reloaded successfully";
    } else {
        status->code = 400;
        status->message = "Failed to reload configuration";
    }
    
    return status;
}
