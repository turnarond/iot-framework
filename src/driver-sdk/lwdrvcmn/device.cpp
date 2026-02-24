/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: device.cpp .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "device.h"
#include "driver.h"
#include "comm_helper.h"
#include "drvframework.h"
#include "lwdrvcmn.h"
#include "lwlog/lwlog.h"
#include <memory.h>
#include "user_timer.h"
#include "lwcommunicate/lwconn_device.h"

extern CLWLog g_logger;

CDevice::CDevice()
{ 
    device_info_ = (LWDEVICE*)malloc(sizeof(LWDEVICE));
    if (device_info_ == nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for device info");
        return;
    }
    LWDEVICE_Reset(device_info_);
    device_info_->_pInternalRef = this;
}

CDevice::~CDevice()
{
    LWDEVICE_Free(device_info_);
    free(device_info_);
    device_info_ = nullptr;
}

void CDevice::StartCheckConnectTimer()
{
    // 检测建连接状态
    // TODO: 处理控制信息
    // driver_info_->HandleWriteCmd();
}
void CDevice::Start()
{
    InitConnectToDevice();
    StartCheckConnectTimer();

    if (DRV_FRAMEWORK->pfnInitDevice_) {
        DRV_FRAMEWORK->pfnInitDevice_(device_info_);
    }

    if (lwconn_device_) {
        LWConnError error = lwconn_device_->start();
        if (error != LWConnError::SUCCESS) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start LWConnDevice for device %s", device_name_.c_str());
            return;
        }
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize LWConnDevice");
        return;
    }
}

void CDevice::Stop()
{
    if (lwconn_device_) {
        lwconn_device_->stop();
        lwconn_device_.reset();
    }
}

void CDevice::InitDeviceInfo(CDriver* drv, std::string name, std::string desc, std::string conn_type, std::string conn_param, 
    std::string param1, std::string param2, std::string param3, std::string param4)
{
    device_name_ = name;
    device_desc_ = desc;
    device_conn_type_ = conn_type;
    device_conn_param_ = conn_param;
    device_param1_ = param1;
    device_param2_ = param2;
    device_param3_ = param3;
    device_param4_ = param4;
    driver_info_ = drv;
    InitDeviceInterface();
}

void CDevice::InitDeviceInterface()
{
    // TODO: set driver info.
    device_info_->driver = nullptr; // No driver associated yet
    strncpy(device_info_->name, device_name_.c_str(), LW_NAME_MAXLEN - 1);
    strncpy(device_info_->desc, device_desc_.c_str(), LW_DESC_MAXLEN - 1);
    device_info_->conn_type = 0; // default connection type: other
    strncpy(device_info_->conn_param, device_conn_param_.c_str(), LW_IOADDR_MAXLEN - 1);
    strncpy(device_info_->param1, device_param1_.c_str(), LW_PARAM_MAXLEN - 1);
    strncpy(device_info_->param2, device_param2_.c_str(), LW_PARAM_MAXLEN - 1);
    strncpy(device_info_->param3, device_param3_.c_str(), LW_PARAM_MAXLEN - 1);
    strncpy(device_info_->param4, device_param4_.c_str(), LW_PARAM_MAXLEN - 1);
    device_info_->tag_count = 0; // No tags associated yet
    if (device_info_->pptags) {
        free(device_info_->pptags);
        device_info_->pptags = nullptr;
    }
    // TODO: copy tags.
    if (device_all_tags_.size() > 0) {
        device_info_->pptags = (LWTAG**)malloc(sizeof(LWTAG*) * device_all_tags_.size());
        if (device_info_->pptags == nullptr) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for device tags");
            return;
        }
        for (size_t i = 0; i < device_all_tags_.size(); ++i) {
            device_info_->pptags[i] = device_all_tags_[i];
        }
        device_info_->tag_count = static_cast<int>(device_all_tags_.size());
    } else {
        device_info_->pptags = nullptr;
    }
}

int CDevice::AddTagOfDevice (const LWTAG *tag)
{
    device_all_tags_.push_back(const_cast<LWTAG *>(tag));
    name_to_tags_[tag->name] = const_cast<LWTAG *>(tag);
    auto addr = addr_to_tags_.find(tag->address);
    if (addr == addr_to_tags_.end()) {
        std::vector<LWTAG *> vecTags;
        vecTags.push_back(const_cast<LWTAG *>(tag));
        addr_to_tags_[tag->address] = vecTags;
    } else {
        addr->second.push_back(const_cast<LWTAG *>(tag));
    }

    return 0;
}

void CDevice::OnWriteCommand(LWTAG *pTag, std::string data)
{
    if (DRV_FRAMEWORK->pfnOnControl_) {
        // Call the framework's control function
        DRV_FRAMEWORK->pfnOnControl_(device_info_, pTag, data.c_str(), data.length(), 0);
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "No control function defined in framework for device %s", device_name_.c_str());
    }
    std::vector<LWTAG *> vecTags;
    vecTags.push_back(pTag);
    drv_settagdata_text(pTag, data.c_str(), 0, 0);
    UpdateTagsData(vecTags.data(), 1);
    vecTags.clear();
}
int CDevice::GetTagsByName(const char* name, LWTAG **ppTags, int tag_count)
{
    auto itMapT = name_to_tags_.find(name);
    if (itMapT == name_to_tags_.end())
    {
        return 0;
    }

    LWTAG *pTag = itMapT->second;
    if (pTag == nullptr) {
        return 0;
    }

    ppTags[0] = pTag;
    return 1; // Return 1 tag found
}
std::map<std::string, LWTAG *> CDevice::GetALLTags() const
{
    return name_to_tags_;
}

int CDevice::GetTagsByAddr(const char* addr, LWTAG **ppTags, int tag_count)
{
    std::vector<LWTAG *> pVecTagOfAddr;
    auto itMapT = addr_to_tags_.find(addr);
    if (itMapT == addr_to_tags_.end())
    {
        return 0;
    }

    pVecTagOfAddr = itMapT->second;
    int nTagNum = 0;
    for (int i = 0; i < pVecTagOfAddr.size() && i < tag_count; i++)
    {
        LWTAG *pTag = pVecTagOfAddr[i];
        ppTags[i] = pTag;
        nTagNum++;
    }
    return nTagNum;
}

int CDevice::SetDeviceConnected(bool bDevConnected)
{
    // TODO: add your code here
    return 0; 
}
int CDevice::UpdateTagsData(LWTAG **tag, int tag_count)
{
    // TODO: update tags data.
    driver_info_->UpdateTagsData(tag, tag_count);
    return 0;
}
int CDevice::InitConnectToDevice()
{
    // 解析连接类型
    LWConnType conn_type = LWConnDeviceFactory::instance().parseConnType(device_conn_type_);
    if (conn_type == LWConnType::TCP_CLIENT || conn_type == LWConnType::TCP_SERVER ||
        conn_type == LWConnType::UDP || conn_type == LWConnType::SERIAL) {
        // 解析连接参数
        LWConnDeviceParams params = LWConnDeviceFactory::instance().parseConnParams(conn_type, device_conn_param_);
        
        // 创建设备
        lwconn_device_ = LWConnDeviceFactory::instance().createDevice(device_name_, conn_type, params);
        if (!lwconn_device_) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create LWConnDevice for device %s", device_name_.c_str());
            return -1;
        }
        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Created LWConnDevice for device %s with type %s", 
                            device_name_.c_str(), device_conn_type_.c_str());
    } else {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Unsupported connection type: %s", device_conn_type_.c_str());
        return -1;
    }
    return 0; 
}

int CDevice::LWDEVICE_Reset(LWDEVICE *device)
{
    memset(device, 0, sizeof(LWDEVICE));
    device->name = (char*)calloc(LW_NAME_MAXLEN, 1);
    device->desc = (char*)calloc(LW_DESC_MAXLEN, 1);
    device->conn_type = 0; // default connection type: other
    device->conn_param = (char*)calloc(LW_IOADDR_MAXLEN, 1);
    device->param1 = (char*)calloc(LW_PARAM_MAXLEN, 1);
    device->param2 = (char*)calloc(LW_PARAM_MAXLEN, 1);
    device->param3 = (char*)calloc(LW_PARAM_MAXLEN, 1);
    device->param4 = (char*)calloc(LW_PARAM_MAXLEN, 1);
    if (!device->name || !device->desc || !device->conn_param ||
        !device->param1 || !device->param2 || !device->param3 || !device->param4) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for device fields, errno is %d", errno);
        return -1;
    }
    device->driver = nullptr;
    device->pptags = nullptr;
    device->tag_count = 0;
    device->_pInternalRef = nullptr;
    device->conn_timeout = 0; // default connection timeout is 0
    device->enable_connect = true;

    return 0;
}

void CDevice::LWDEVICE_Free(LWDEVICE *device)
{
    if (device == nullptr) {
        return;
    }
    if (device->name) {
        free(device->name);
        device->name = nullptr;
    }
    if (device->desc) {
        free(device->desc);
        device->desc = nullptr;
    }
    if (device->conn_param) {
        free(device->conn_param);
        device->conn_param = nullptr;
    }
    if (device->param1) {
        free(device->param1);
        device->param1 = nullptr;
    }
    if (device->param2) {
        free(device->param2);
        device->param2 = nullptr;
    }
    if (device->param3) {
        free(device->param3);
        device->param3 = nullptr;
    }
    if (device->param4) {
        free(device->param4);
        device->param4 = nullptr;
    }
    
    return;
}

int CDevice::CheckAndConnect(int nConnectTimeOutMS)
{
    if (lwconn_device_) {
        LWConnStatus status = lwconn_device_->getStatus();
        bool connected = (status == LWConnStatus::CONNECTED);
        if (!connected) {
            // 尝试重新连接
            LWConnError error = lwconn_device_->start();
            status = lwconn_device_->getStatus();
            connected = (status == LWConnStatus::CONNECTED);
            SetDeviceConnected(connected);
        }

        if (!connected) {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "CheckAndConnect failed to connect to device:%s,conntype:%s,connparam:%s!",
                device_name_.c_str(), device_conn_type_.c_str(), device_conn_param_.c_str());
            return 0;
        }
    }

    return 0;
}

int CDevice::DisconnectFromDevice()
{
    if (lwconn_device_) {
        lwconn_device_->stop();
        SetDeviceConnected(false);
    }
    return 0;
}

CUserTimer * CDevice::CreateAndStartTimer(DRVTIMER * timerInfo )
{
    CUserTimer *pBlockTimer = new CUserTimer(this);
    memcpy(&pBlockTimer->timer_info_, timerInfo, sizeof(DRVTIMER));
    pBlockTimer->timer_info_.internal_ref = pBlockTimer;

    vec_timers.push_back(pBlockTimer);
    pBlockTimer->StartTimer();
    g_logger.LogMessage(LW_LOGLEVEL_NOTICE, "(CreateAndStartTimer)timer started: device:%s, start driver timer cycle=%d MS", 
        this->device_name_.c_str(), timerInfo->period_ms);
    return pBlockTimer;
}
