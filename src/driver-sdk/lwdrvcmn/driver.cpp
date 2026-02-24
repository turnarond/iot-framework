/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: driver.cpp .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "driver.h"
#include "comm_helper.h"
#include "device.h"
#include "drvframework.h"
#include "lwcomm/lwcomm.h"
#include "lwdrvcmn.h"
#include "lwipcssn/ipc_client.h"
#include "lwlog/lwlog.h"
#include "platform_sdk/init.h"
#include "platform_sdk/macro.h"
#include "platform_sdk/status.hpp"
#include "platform_sdk/timer.h"
#include "drvdto.hpp"
#include "vsoa_dto/core/Types.hpp"
#include <cmath>

extern CLWLog g_logger;
CDriver::CDriver()
    : driver_name_(""), driver_desc_(""), drv_param1(""), drv_param2(""), drv_param3(""), drv_param4(""),
      driver_status_(0), device_num_(0)
{
    driver_info_ = (LWDRIVER*)malloc(sizeof(LWDRIVER));
    if (driver_info_ == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver info");
        return;
    }
    LWDRIVER_Reset(driver_info_);
}

CDriver::~CDriver()
{
    DestroyDevices();
    LWDRIVER_Free(driver_info_);
    free(driver_info_);
    driver_info_ = nullptr;
}

void CDriver::OnDatagramServerConnect(void *arg, ipc_client_auto_t *cliauto, bool connect)
{
    CDriver *driver = static_cast<CDriver*>(arg);

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "OnConnect to server %s: %s",
        driver->node_server_srvname_.c_str(), connect?"connected":"disconnected");
    // TODO: InitTags2NodeServer!!! 
    if (connect)
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Init all tags to node_server");
        driver->UpdateTags2NodeServer();
    }
}

int CDriver::HandleWriteCmd(std::string tag_name, std::string tag_value)
{
    LWTAG *tag = nullptr;
    for (auto device : devices_)
    {
        if (device.second->GetTagsByName(tag_name.c_str(), &tag, 0) > 0)
        {
            device.second->OnWriteCommand(tag, tag_value);
        }
    }
    return 0;
}

void CDriver::UpdateTagsData(LWTAG **tag, int tag_count)
{
    vsoa::List<vsoa::Object<DataValueDto>> data_list = vsoa::List<vsoa::Object<DataValueDto>>::createShared();
    auto now = chrono::system_clock::now();
    long long time = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    for (int i = 0; i < tag_count; i++)
    {
        auto data_dto = vsoa::Object<DataValueDto>::createShared();
        data_dto->name = tag[i]->name;
        data_dto->value = DRV_FRAMEWORK->GetTagDataValueToString(tag[i]->data_type, tag[i]->data_length, tag[i]->data);
        if (tag[i]->time_milli == 0)
        {
            data_dto->time = time;
        }
        else
        {
            data_dto->time = tag[i]->time_milli;
        }
        data_dto->quality = "1";
        data_list->push_back(data_dto);
    }
    if (obj_mapper_ != nullptr)
    {
        vsoa::String json_data = obj_mapper_->writeToString(data_list);
        ipc_payload_t payload 
        {
            .data = (void*)json_data->c_str(),
            .data_len = json_data->length()
        };
        if (client_handle_ != nullptr)
        {
            ipc_client_datagram(client_handle_, &pub_url_, &payload);
        }
    }
    else
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "obj_mapper_ is nullptr in UpdateTagsData");
    }
}
int CDriver::OnStart()
{
    vsoa_sdk::init();
    vsoa_sdk::setDataMode(DataMode::PARAM_MODE);
    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;

    obj_mapper_ = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);

    // 初始化 vsoa 服务
    server_handle_ = new vsoa_sdk::server::ServerHandle(driver_name_);
    if (server_handle_ == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to create server handle");
        return -1; // Failed to create server handle
    }

    server_handle_->AddRpcListener(ctrl_url_, OnWriteCmd, this);
    server_handle_->CreateServer(OnServerCmd, this);

    // 初始化定时器
    TimerInit();

    server_handle_->SrvSpinAsync();
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Driver %s started successfully", driver_name_.c_str());

    // 创建 node_server 的客户端
    node_server_path_ = LWComm::GetDataPath();
    node_server_path_ = node_server_path_ + LW_OS_DIR_SEPARATOR + node_server_srvname_;

    client_auto_ = ipc_client_auto_create(NULL, NULL);
    if (client_auto_ == NULL)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "ipc_client_auto_create failed");
        return -1;
    }
    ipc_client_auto_setup(client_auto_, OnDatagramServerConnect, this);
    if (!ipc_client_auto_start(client_auto_, node_server_path_.c_str(), NULL, 0, 1000, 1000, 1000))
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Start client to server %s failed",
            node_server_path_.c_str());
        return -1;
    }

    client_handle_ = ipc_client_auto_handle(client_auto_);
    ipc_client_set_on_datagram(client_handle_, OnNodeserverCb, this);

    return 0;
}
int CDriver::Start()
{
    int ret = 0;
    if (DRV_FRAMEWORK->pfnInitDriver_)
    {
        if ((ret = DRV_FRAMEWORK->pfnInitDriver_(driver_info_)) != 0)
        {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to initialize driver: %d", ret);
            return ret; // Initialization failed
        }
    }

    if (OnStart() < 0)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to start driver");
        return -1; // Failed to start driver
    }

    // Init all tags to node_server
    UpdateTags2NodeServer();

    for (auto& device_pair : devices_)
    {
        CDevice* device = device_pair.second;
        device->Start();
    }
    return 0; // Success
}

void CDriver::OnNodeserverCb(void *arg, ipc_client_t *client, ipc_url_t *url, ipc_payload_t *payload)
{
    CDriver* driver = static_cast<CDriver*>(arg);
    int ret = -1;
    if (driver == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Driver pointer is null in OnNodeserverCb");
        return;
    }
    if (strncmp(url->url, driver->ctrl_url_.c_str(), driver->ctrl_url_.length()))
    {
        vsoa::Object<ControlValueDto> ctrl_value;
        try
        {
            ctrl_value = driver->obj_mapper_->readFromString<vsoa::Object<ControlValueDto>>(
                vsoa::String((char*)payload->data, payload->data_len)
            );
        }
        catch (vsoa::parser::ParsingError &e)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "OnNodeserverCb: parse %.*s to DataValueDto failed: %s",
                payload->data_len, payload->data, e.what());
            return;
        }
        driver->HandleWriteCmd(ctrl_value->name, ctrl_value->value);
    }
    else
    {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "OnNodeserverCb: unknown url %.*s",
            url->url_len, url->url);
    }
}
void CDriver::OnWriteCmd (vsoa_sdk::server::CliRpcInfo& cli, const void* dto, size_t len, void* arg)
{
    CDriver* driver = static_cast<CDriver*>(arg);
    int ret = -1;
    if (driver == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Driver pointer is null in OnWriteCmd");
        return;
    }

    // Deserialize the dto to a command structure
    // return all urls.
    if (cli.url == driver->ctrl_url_)
    {
        // 收到点位控制
        auto tag_write = driver->obj_mapper_->readFromString<vsoa::Object<ControlValueDto>>(
            vsoa::String((char*)dto, len)
        );
        ret = driver->HandleWriteCmd(tag_write->name->c_str(), tag_write->value->c_str());
        if (ret == 0)
        {
            driver->server_handle_->SrvResponse(cli, vsoa::Status::CODE_0, "OK");
        }
        else
        {
            driver->server_handle_->SrvResponse(cli, vsoa::Status::CODE_1, "Failed to handle write command");
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to handle write command for tag %s", 
                tag_write->name->c_str());
        }
    }

    return;
}
void CDriver::OnServerCmd(vsoa_sdk::server::CliRpcInfo& cli, const void* dto, size_t len, void* arg)
{
    CDriver* driver = static_cast<CDriver*>(arg);
    if (driver == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Driver pointer is null in OnServerCmd");
        return;
    }

    // Deserialize the dto to a command structure
    
}
int CDriver::InitDriverInfo(const std::string& driver_name, const std::string& driver_desc,
    const std::string& drv_param1, const std::string& drv_param2,
    const std::string& drv_param3, const std::string& drv_param4)
{
    driver_name_ = driver_name;
    driver_desc_ = driver_desc;
    this->drv_param1 = drv_param1;
    this->drv_param2 = drv_param2;
    this->drv_param3 = drv_param3;
    this->drv_param4 = drv_param4;

    if (driver_info_ == nullptr)
    {
        driver_info_ = (LWDRIVER*)malloc(sizeof(LWDRIVER));
        if (driver_info_ == nullptr)
        {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver info");
            return -1;
        }
        memset(driver_info_, 0, sizeof(LWDRIVER));
    }

    publish_url_ = "/" + driver_name_ + "/" + fix_data_cmd_; // URL for publishing data
    ctrl_url_ = "/" + driver_name_ + "/" + fix_ctrl_cmd_;
    return 0; // Success
}

int CDriver::AddDevice(CDevice* device_info)
{
    std::string device_name = device_info->GetDeviceName();
    if (devices_.find(device_name) != devices_.end())
    {
        return 0; // Device already exists
    }
    devices_[device_name] = device_info;
    device_num_ = devices_.size();

    return 0; // Success
}

void CDriver::InitDriverInterface ()
{
    if (driver_info_ == nullptr)
    {
        return;
    }

    driver_info_->ppdevices = (LWDEVICE**)malloc(sizeof(LWDEVICE*) * device_num_);
    if (driver_info_->ppdevices == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver devices");
        return;
    }
    int i = 0;
    for (auto& device_pair : devices_)
    {
        device_pair.second->InitDeviceInterface();
        driver_info_->ppdevices[i++] = device_pair.second->GetDeviceInterface();
    }
    driver_info_->device_count = device_num_;

}

void CDriver::UpdateTags2NodeServer()
{
    vsoa::Object<DriverTagsDto> driver_tags = vsoa::Object<DriverTagsDto>::createShared();
    driver_tags->driver_name = vsoa::String(driver_name_);
    driver_tags->devtags = vsoa::Vector<vsoa::Object<DeviceTagsDto> >::createShared();
    for (const auto& device_pair : devices_)
    {
        CDevice* device = device_pair.second;
        std::map<std::string, LWTAG *> tags = device->GetALLTags();
        auto device_tags = DeviceTagsDto::createShared();
        device_tags->device_name = vsoa::String(device->GetDeviceName());
        device_tags->taglist = vsoa::Vector<vsoa::String>::createShared();
        for (const auto& tag : tags)
        {
            device_tags->taglist->push_back(tag.first);
            g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "UpdateTags2NodeServer: device %s, tag %s",
                device->GetDeviceName().c_str(), tag.first.c_str());
        }
        driver_tags->devtags->push_back(device_tags);
        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "UpdateTags2NodeServer: driver %s, device %s, tag count %zu",
            driver_name_.c_str(), device->GetDeviceName().c_str(), device_tags->taglist->size());
    }

    auto node_init_str = obj_mapper_->writeToString(driver_tags);
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "UpdateTags2NodeServer: node init str %s", 
        node_init_str->c_str());
    ipc_payload_t init_payload 
    {
        .data = (void*)node_init_str->c_str(),
        .data_len = node_init_str->length()
    };
    if (client_handle_ != nullptr)
    {
        ipc_client_datagram(client_handle_, &init_url_, &init_payload);
    }
    else
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Client handle is null, cannot send init data to node server");
    }
    g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "UpdateTags2NodeServer: driver %s, init data sent to node server",
        driver_name_.c_str());
}

// TODO: Implement the logic to handle write commands
void CDriver::DestroyDevices()
{
    for (auto& device_pair : devices_)
    {
        device_pair.second->Stop();
    }
    devices_.clear();
    return;
}

int CDriver::LWDRIVER_Reset(LWDRIVER *pDriver)
{
    if (pDriver == nullptr)
    {
        return -1;
    }
    pDriver->name = (char*)calloc(LW_NAME_MAXLEN, 1);
    if (pDriver->name == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver name, errno is %d", 
            errno);
        return -1;
    }
    pDriver->type = 5; // default type: other
    pDriver->version = (char*)calloc(LW_NAME_MAXLEN, 1);
    if (pDriver->version == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver version, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->description = (char*)calloc(LW_DESC_MAXLEN, 1);
    if (pDriver->description == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver description, errno is %d", 
            errno);
        goto FAILED;
    }
    
    // parameter definitions
    pDriver->param1_name = (char*)calloc(LW_NAME_MAXLEN, 1);
    if (pDriver->param1_name == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param1_name, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param1_desc = (char*)calloc(LW_DESC_MAXLEN, 1);
    if (pDriver->param1_desc == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param1_desc, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param2_name = (char*)calloc(LW_NAME_MAXLEN, 1);
    if (pDriver->param2_name == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param2_name, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param2_desc = (char*)calloc(LW_DESC_MAXLEN, 1);
    if (pDriver->param2_desc == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param2_desc, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param3_name = (char*)calloc(LW_NAME_MAXLEN, 1);
    if (pDriver->param3_name == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param3_name, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param3_desc = (char*)calloc(LW_DESC_MAXLEN, 1);
    if (pDriver->param3_desc == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param3_desc, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param4_name = (char*)calloc(LW_NAME_MAXLEN, 1);
    if (pDriver->param4_name == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param4_name, errno is %d", 
            errno);
        goto FAILED;
    }
    pDriver->param4_desc = (char*)calloc(LW_DESC_MAXLEN, 1);
    if (pDriver->param4_desc == nullptr)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to allocate memory for driver param4_desc, errno is %d", 
            errno);
        goto FAILED;
    }
    
    pDriver->ppdevices = NULL;
    pDriver->device_count = 0;
    pDriver->_pInternalRef = NULL;

    memset(pDriver->pUserData, 0, sizeof(pDriver->pUserData));
    memset(pDriver->nUserData, 0, sizeof(pDriver->nUserData));
    
    return 0;

FAILED:
    LWDRIVER_Free(pDriver);
    return -1;
}

void CDriver::LWDRIVER_Free(LWDRIVER *pDriver)
{
    if (pDriver == nullptr)
    {
        return;
    }
    if (pDriver->name)
    {
        free(pDriver->name);
        pDriver->name = nullptr;
    }
    if (pDriver->version)
    {
        free(pDriver->version);
        pDriver->version = nullptr;
    }
    if (pDriver->description)
    {
        free(pDriver->description);
        pDriver->description = nullptr;
    }
    if (pDriver->param1_name)
    {
        free(pDriver->param1_name);
        pDriver->param1_name = nullptr;
    }
    if (pDriver->param1_desc)
    {
        free(pDriver->param1_desc);
        pDriver->param1_desc = nullptr;
    }
    if (pDriver->param2_name)
    {
        free(pDriver->param2_name);
        pDriver->param2_name = nullptr;
    }
    if (pDriver->param2_desc)
    {
        free(pDriver->param2_desc);
        pDriver->param2_desc = nullptr;
    }
    if (pDriver->param3_name)
    {
        free(pDriver->param3_name);
        pDriver->param3_name = nullptr;
    }
    if (pDriver->param3_desc)
    {
        free(pDriver->param3_desc);
        pDriver->param3_desc = nullptr;
    }
    if (pDriver->param4_name)
    {
        free(pDriver->param4_name);
        pDriver->param4_name = nullptr;
    }
    if (pDriver->param4_desc)
    {
        free(pDriver->param4_desc);
        pDriver->param4_desc = nullptr;
    }
    if (pDriver->_pInternalRef)
    {
        free(pDriver->_pInternalRef);
        pDriver->_pInternalRef = nullptr;
    }
}
