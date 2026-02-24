/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: resource_manager.cpp .
*
* Date: 2026-02-11
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "resource_manager.h"
#include "device.h"
#include "lwlog/lwlog.h"
#include <cstring>
#include <cmath>

extern CLWLog g_logger;

// TagResourceManager 实现
TagResourceManager::~TagResourceManager() {
    ClearTags();
}

LWTAG* TagResourceManager::CreateTag(const std::string& name, const std::string& address, 
                                   int data_type, int len_bit) {
    try {
        // 创建点位
        std::unique_ptr<LWTAG> tag_info = std::make_unique<LWTAG>();
        
        // 初始化点位信息
        tag_info->name = strdup(name.c_str());
        tag_info->address = strdup(address.c_str());
        tag_info->polling_interval = 1000; // 默认轮询间隔
        tag_info->data_type = data_type;
        tag_info->len_bit = len_bit;
        tag_info->data_length = static_cast<int>(ceil(static_cast<double>(len_bit) / 8.0));
        
        // 使用智能指针管理data内存
        std::unique_ptr<char[]> data_ptr(new char[tag_info->data_length + 1]);
        std::memset(data_ptr.get(), '\0', tag_info->data_length + 1);
        tag_info->data = data_ptr.release();
        
        tag_info->byte_order = 0; // 默认小端序
        tag_info->param = strdup("");
        tag_info->quantity = 0; // Initialize quantity to 0
        tag_info->time_milli = 0; // Initialize time_milli to 0
        
        // 设置其他默认值
        tag_info->point_type = 1; // device_variable
        tag_info->transfer_type = 0; // none
        tag_info->linear_raw_min = 0.0;
        tag_info->linear_raw_max = 0.0;
        tag_info->linear_eng_min = 0.0;
        tag_info->linear_eng_max = 0.0;
        tag_info->advanced_algo_lib = nullptr;
        tag_info->advanced_param1 = nullptr;
        tag_info->advanced_param2 = nullptr;
        tag_info->advanced_param3 = nullptr;
        tag_info->advanced_param4 = nullptr;
        tag_info->enable_control = false;
        tag_info->enable_history = false;
        
        // 初始化保留数据
        tag_info->res_data1 = 0;
        tag_info->res_data2 = 0;
        tag_info->res_data3 = 0;
        tag_info->res_data4 = 0;
        tag_info->res_pdata1 = nullptr;
        tag_info->res_pdata2 = nullptr;
        
        // 添加到列表
        LWTAG* tag_ptr = tag_info.get();
        tags_.push_back(std::move(tag_info));
        
        return tag_ptr;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception when creating tag: %s", e.what());
        return nullptr;
    }
}

std::vector<LWTAG*> TagResourceManager::GetTags() {
    std::vector<LWTAG*> tags;
    for (auto& tag : tags_) {
        tags.push_back(tag.get());
    }
    return tags;
}

void TagResourceManager::ClearTags() {
    for (auto& tag : tags_) {
        if (tag->name) free(tag->name);
        if (tag->address) free(tag->address);
        if (tag->data) delete[] tag->data;
        if (tag->param) free(tag->param);
        if (tag->advanced_algo_lib) free(tag->advanced_algo_lib);
        if (tag->advanced_param1) free(tag->advanced_param1);
        if (tag->advanced_param2) free(tag->advanced_param2);
        if (tag->advanced_param3) free(tag->advanced_param3);
        if (tag->advanced_param4) free(tag->advanced_param4);
    }
    tags_.clear();
}

// DeviceResourceManager 实现
DeviceResourceManager::~DeviceResourceManager() {
    ClearDevices();
}

CDevice* DeviceResourceManager::CreateDevice(CDriver* drv, const std::string& name, const std::string& desc, 
                                           const std::string& conn_type, const std::string& conn_param,
                                           const std::string& param1, const std::string& param2,
                                           const std::string& param3, const std::string& param4) {
    try {
        // 创建设备
        std::unique_ptr<CDevice> device_info = std::make_unique<CDevice>();
        
        // 初始化设备信息
        device_info->InitDeviceInfo(drv, name, desc, conn_type, conn_param,
                                  param1, param2, param3, param4);
        
        // 创建对应的点位管理器
        std::unique_ptr<TagResourceManager> tag_manager = std::make_unique<TagResourceManager>();
        
        // 添加到映射
        CDevice* device_ptr = device_info.get();
        devices_[name] = std::move(device_info);
        tag_managers_[name] = std::move(tag_manager);
        
        return device_ptr;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception when creating device: %s", e.what());
        return nullptr;
    }
}

CDevice* DeviceResourceManager::GetDevice(const std::string& name) {
    auto it = devices_.find(name);
    if (it != devices_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::map<std::string, CDevice*> DeviceResourceManager::GetDevices() {
    std::map<std::string, CDevice*> device_map;
    for (auto& device : devices_) {
        device_map[device.first] = device.second.get();
    }
    return device_map;
}

LWTAG* DeviceResourceManager::AddTagToDevice(const std::string& device_name, const std::string& tag_name,
                                           const std::string& address, int data_type, int len_bit) {
    auto it = tag_managers_.find(device_name);
    if (it != tag_managers_.end()) {
        LWTAG* tag = it->second->CreateTag(tag_name, address, data_type, len_bit);
        if (tag) {
            // 添加点位到设备
            CDevice* device = GetDevice(device_name);
            if (device) {
                device->AddTagOfDevice(tag);
            }
        }
        return tag;
    }
    return nullptr;
}

void DeviceResourceManager::ClearDevices() {
    // 清空点位管理器
    tag_managers_.clear();
    
    // 清空设备
    devices_.clear();
}

// DriverResourceManager 实现
DriverResourceManager::DriverResourceManager() {
    device_manager_ = std::make_unique<DeviceResourceManager>();
}

DriverResourceManager::~DriverResourceManager() {
    ClearResources();
}

int DriverResourceManager::InitDriver(const std::string& name, const std::string& desc,
                                    const std::string& param1, const std::string& param2,
                                    const std::string& param3, const std::string& param4) {
    try {
        // 创建驱动实例
        driver_ = std::make_unique<LWDRIVER>();
        
        // 初始化驱动信息
        driver_->name = strdup(name.c_str());
        driver_->type = 5; // 默认类型：其他
        driver_->version = strdup("1.0.0");
        driver_->description = strdup(desc.c_str());
        driver_->param1_name = strdup(param1.c_str());
        driver_->param1_desc = strdup("");
        driver_->param2_name = strdup(param2.c_str());
        driver_->param2_desc = strdup("");
        driver_->param3_name = strdup(param3.c_str());
        driver_->param3_desc = strdup("");
        driver_->param4_name = strdup(param4.c_str());
        driver_->param4_desc = strdup("");
        
        driver_->ppdevices = nullptr;
        driver_->device_count = 0;
        driver_->_pInternalRef = nullptr;
        
        // 初始化保留数据
        memset(driver_->pUserData, 0, sizeof(driver_->pUserData));
        memset(driver_->nUserData, 0, sizeof(driver_->nUserData));
        
        driver_name_ = name;
        
        return 0;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception when initializing driver: %s", e.what());
        return -1;
    }
}

DeviceResourceManager* DriverResourceManager::GetDeviceManager() {
    return device_manager_.get();
}

LWDRIVER* DriverResourceManager::GetDriver() {
    return driver_.get();
}

std::string DriverResourceManager::GetDriverName() const {
    return driver_name_;
}

void DriverResourceManager::ClearResources() {
    // 清空设备资源
    device_manager_->ClearDevices();
    
    // 清空驱动资源
    if (driver_) {
        if (driver_->name) free(driver_->name);
        if (driver_->version) free(driver_->version);
        if (driver_->description) free(driver_->description);
        if (driver_->param1_name) free(driver_->param1_name);
        if (driver_->param1_desc) free(driver_->param1_desc);
        if (driver_->param2_name) free(driver_->param2_name);
        if (driver_->param2_desc) free(driver_->param2_desc);
        if (driver_->param3_name) free(driver_->param3_name);
        if (driver_->param3_desc) free(driver_->param3_desc);
        if (driver_->param4_name) free(driver_->param4_name);
        if (driver_->param4_desc) free(driver_->param4_desc);
        if (driver_->ppdevices) delete[] driver_->ppdevices;
    }
}

// GlobalResourceManager 实现
GlobalResourceManager::GlobalResourceManager() {
    driver_manager_ = std::make_unique<DriverResourceManager>();
}

GlobalResourceManager::~GlobalResourceManager() {
    Cleanup();
}

DriverResourceManager* GlobalResourceManager::GetDriverManager() {
    return driver_manager_.get();
}

int GlobalResourceManager::Init() {
    // 初始化全局资源管理器
    return 0;
}

void GlobalResourceManager::Cleanup() {
    // 清空驱动资源
    driver_manager_->ClearResources();
}

// 静态实例
GlobalResourceManager& GlobalResourceManager::GetInstance() {
    static GlobalResourceManager instance;
    return instance;
}
