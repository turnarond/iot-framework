/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: maintask.cpp .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "maintask.h"

#include "device.h"
#include "drvframework.h"
#include "comm_helper.h"
#include "config_loader.h"
#include "resource_manager.h"
#include "lwlog/lwlog.h"

extern CLWLog g_logger;

#include "lwcomm/lwcomm.h"
#include <cstdlib>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

// 使用Poco库进行XML解析
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/XML/XMLException.h>

// 使用Poco库进行SQLite操作
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>

using namespace Poco::Data;

// 全局日志对象
extern CLWLog g_logger;

CMainTask CMainTask::main_task_;
CMainTask* CMainTask::GetInstance()
{
    return &main_task_;
}
CMainTask::CMainTask()
    : main_thread_(), drive_info_(), tag_num_(0), device_num_(0)
{
    // Constructor implementation
}
CMainTask::~CMainTask()
{
    // Destructor implementation
}

int CMainTask::Start()
{
    // Start implementation
    main_thread_ = std::thread(&CMainTask::svc_thread_main, this);
    if (main_thread_.joinable()) {
        main_thread_.detach();
    } else {
        return -1; // Failed to start thread
    }
    return 0;
}

void CMainTask::Stop()
{
    // Stop implementation
    bstop_ = true;
}

int CMainTask::CalcDriverTagDataSize(unsigned int *pnTagCount, unsigned int *pnTagDataSize, unsigned int *pnTagMaxDataSize)
{
    return 0;
}

int CMainTask::OnStart()
{
    int ret = LoadConfig();
    if (ret != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load configuration: %d", ret);
        return ret;
    }
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Configuration loaded successfully, device count: %d", 
        device_num_);

    // Start the driver
    ret = drive_info_.Start();
    if (ret != 0) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start driver: %d", ret);
        return ret;
    }

    return 0;
}

int CMainTask::OnStop()
{
    return 0;
}

void CMainTask::svc_thread_main(void *arg)
{
    CMainTask *pThis = static_cast<CMainTask *>(arg);
    if (pThis) {
        pThis->OnStart();
        // Main task logic goes here
        while (!pThis->bstop_) {
            // Perform periodic tasks, e.g., polling devices, processing data, etc.
            std::this_thread::sleep_for(std::chrono::seconds(3));
            for (auto &device : pThis->drive_info_.GetDevices()) {
                // Poll each device
            }

            // find and process write commands
            std::string write_cmds;
            pThis->ProcessWriteCmds(write_cmds);
        }
        // Cleanup and stop the driver
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Stopping main task...");
        pThis->OnStop();
    } 
}

// 连接类型映射：从数据库数字类型映射到字符串类型
std::string connTypeToString(int conn_type) {
    switch (conn_type) {
        case 1: return "tcpclient";
        case 2: return "tcpserver";
        case 3: return "udp";
        case 4: return "serial";
        default: return "other";
    }
}

// 数据类型映射：从数据库数字类型映射到字符串类型
std::string dataTypeToString(int data_type) {
    switch (data_type) {
        case 1: return "bool";
        case 2: return "int";
        case 3: return "float";
        case 4: return "string";
        default: return "int";
    }
}

int CMainTask::LoadConfig()
{
    try {
        // 构建配置文件路径
        std::string bin_path = LWComm::GetBinPath();
        std::string drv_name = DRV_FRAMEWORK->GetDrvName();
        std::string xmlconfig_path = bin_path + LW_OS_DIR_SEPARATOR_CHAR + "drivers" + 
                                    LW_OS_DIR_SEPARATOR_CHAR + drv_name + 
                                    LW_OS_DIR_SEPARATOR_CHAR + drv_name + ".xml";
        
        // 构建数据库路径
        std::string db_path = bin_path + LW_OS_DIR_SEPARATOR_CHAR + ".." + 
                            LW_OS_DIR_SEPARATOR_CHAR + "data" + 
                            LW_OS_DIR_SEPARATOR_CHAR + "bas-business.db";
        
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Loading configuration from XML: %s", xmlconfig_path.c_str());
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Database path: %s", db_path.c_str());
        
        // 使用ConfigLoader加载配置
        ConfigLoader config_loader(xmlconfig_path, db_path);
        
        if (!config_loader.LoadConfig()) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load configuration");
            return -1;
        }
        
        // 获取驱动列表
        auto drivers = config_loader.GetDrivers();
        if (drivers.empty()) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "No drivers found in configuration");
            return -1;
        }
        
        // 找到当前驱动
        LWDRIVER* current_driver = nullptr;
        for (auto driver : drivers) {
            if (driver->name && std::string(driver->name) == drv_name) {
                current_driver = driver;
                break;
            }
        }
        
        if (!current_driver) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Driver %s not found in configuration", drv_name.c_str());
            return -1;
        }
        
        // 初始化驱动信息
        std::string desc = current_driver->description ? current_driver->description : "";
        std::string param1 = current_driver->param1_name ? current_driver->param1_name : "";
        std::string param2 = current_driver->param2_name ? current_driver->param2_name : "";
        std::string param3 = current_driver->param3_name ? current_driver->param3_name : "";
        std::string param4 = current_driver->param4_name ? current_driver->param4_name : "";
        
        // 使用全局资源管理器初始化驱动
        int ret = RESOURCE_MANAGER.GetDriverManager()->InitDriver(drv_name, desc, param1, param2, param3, param4);
        if (ret != 0) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize driver info: %d", ret);
            return ret;
        }
        
        // 初始化驱动信息到CDriver
        ret = drive_info_.InitDriverInfo(drv_name, desc, param1, param2, param3, param4);
        if (ret != 0) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize driver info: %d", ret);
            return ret;
        }
        
        // 日志驱动信息
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Driver Name: %s, Desc: %s, Param1: %s, Param2: %s, Param3: %s, Param4: %s",
                            drv_name.c_str(), desc.c_str(), param1.c_str(), param2.c_str(), param3.c_str(), param4.c_str());
        
        // 处理设备
        if (current_driver->ppdevices && current_driver->device_count > 0) {
            for (int i = 0; i < current_driver->device_count; i++) {
                auto device = current_driver->ppdevices[i];
                if (device) {
                    // 转换连接类型为字符串
                    std::string device_conn_type = connTypeToString(device->conn_type);
                    
                    // 使用资源管理器创建设备
                    CDevice* device_info = RESOURCE_MANAGER.GetDriverManager()->GetDeviceManager()->CreateDevice(
                        &drive_info_,
                        device->name ? device->name : "",
                        device->desc ? device->desc : "",
                        device_conn_type,
                        device->conn_param ? device->conn_param : "",
                        device->param1 ? device->param1 : "",
                        device->param2 ? device->param2 : "",
                        device->param3 ? device->param3 : "",
                        device->param4 ? device->param4 : ""
                    );
                    
                    if (device_info) {
                        // 处理点位
                        if (device->pptags && device->tag_count > 0) {
                            for (int j = 0; j < device->tag_count; j++) {
                                auto tag = device->pptags[j];
                                if (tag) {
                                    // 使用资源管理器添加点位到设备
                                    RESOURCE_MANAGER.GetDriverManager()->GetDeviceManager()->AddTagToDevice(
                                        device->name ? device->name : "",
                                        tag->name ? tag->name : "",
                                        tag->address ? tag->address : "",
                                        tag->data_type,
                                        tag->len_bit
                                    );
                                    ++tag_num_;
                                }
                            }
                        }
                        
                        // 添加设备到驱动
                        drive_info_.AddDevice(device_info);
                        ++device_num_;
                    }
                }
            }
        }
        
        // 初始化驱动接口
        drive_info_.InitDriverInterface();
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Successfully loaded configuration. Devices: %d, Tags: %d", device_num_, tag_num_);
        
        return 0;
    } catch (const std::exception& e) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Exception during config loading: %s", e.what());
        return -1;
    } catch (...) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Unknown exception during config loading");
        return -1;
    }
}

int CMainTask::ProcessWriteCmds(std::string &strCmds)
{
    // Process write commands logic here
    // For now, just return 0 to indicate success
    return 0;
}

int CMainTask::ProcessWriteOneCmd(std::string ctrl_cmd)
{
    // Process a single write command
    // For now, just return 0 to indicate success
    return 0;
}
