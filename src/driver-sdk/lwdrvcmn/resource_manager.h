/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: resource_manager.h .
*
* Date: 2026-02-11
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "lwdrvcmn.h"
#include <memory>
#include <vector>
#include <map>
#include <string>

// 前向声明
class CDevice;

class CDriver;

// 点位资源管理器
class TagResourceManager {
private:
    std::vector<std::unique_ptr<LWTAG>> tags_;

public:
    ~TagResourceManager();

    // 创建点位
    LWTAG* CreateTag(const std::string& name, const std::string& address, int data_type, int len_bit);
    
    // 获取点位列表
    std::vector<LWTAG*> GetTags();
    
    // 清空点位
    void ClearTags();
};

// 设备资源管理器
class DeviceResourceManager {
private:
    std::map<std::string, std::unique_ptr<CDevice>> devices_;
    std::map<std::string, std::unique_ptr<TagResourceManager>> tag_managers_;

public:
    ~DeviceResourceManager();

    // 创建设备
    CDevice* CreateDevice(CDriver* drv, const std::string& name, const std::string& desc, 
                         const std::string& conn_type, const std::string& conn_param,
                         const std::string& param1, const std::string& param2,
                         const std::string& param3, const std::string& param4);
    
    // 获取设备
    CDevice* GetDevice(const std::string& name);
    
    // 获取所有设备
    std::map<std::string, CDevice*> GetDevices();
    
    // 添加点位到设备
    LWTAG* AddTagToDevice(const std::string& device_name, const std::string& tag_name,
                         const std::string& address, int data_type, int len_bit);
    
    // 清空设备
    void ClearDevices();
};

// 驱动资源管理器
class DriverResourceManager {
private:
    std::unique_ptr<DeviceResourceManager> device_manager_;
    std::unique_ptr<LWDRIVER> driver_;
    std::string driver_name_;

public:
    DriverResourceManager();
    ~DriverResourceManager();

    // 初始化驱动
    int InitDriver(const std::string& name, const std::string& desc,
                  const std::string& param1, const std::string& param2,
                  const std::string& param3, const std::string& param4);
    
    // 获取设备管理器
    DeviceResourceManager* GetDeviceManager();
    
    // 获取驱动信息
    LWDRIVER* GetDriver();
    
    // 获取驱动名称
    std::string GetDriverName() const;
    
    // 清空资源
    void ClearResources();
};

// 全局资源管理器
class GlobalResourceManager {
private:
    std::unique_ptr<DriverResourceManager> driver_manager_;

public:
    GlobalResourceManager();
    ~GlobalResourceManager();

    // 获取驱动资源管理器
    DriverResourceManager* GetDriverManager();
    
    // 初始化
    int Init();
    
    // 清理资源
    void Cleanup();
    
    // 静态实例
    static GlobalResourceManager& GetInstance();
};

// 宏定义
#define RESOURCE_MANAGER GlobalResourceManager::GetInstance()
