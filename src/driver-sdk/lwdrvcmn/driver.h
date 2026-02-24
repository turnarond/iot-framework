/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: driver.h .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/
#pragma once

#include "device.h"
#include <string>

#include "lwdrvcmn.h"
#include "lwipcssn/ipc_cliauto.h"

#include "platform_sdk/server.h"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"

class CDriver 
{
public:
    CDriver();
    ~CDriver();
    void Copy2OutDriverInfo();
    static int UpdateDriverStatus(int nStatus, char *szErrTip, char *pszTime);
    static int UpdateDriverOnlineConfigTime(int nStatus, char *szErrTip, char *pszTime);
    static int UpdateDriverStartTime();
    int UpdateDriverDeviceNum();
    int GetDeviceNum();
    std::map<std::string, CDevice*> GetDevices() const {
        return devices_;
    }
    static void ResetTagsValue(std::string driver_name);

    int InitDriverInfo(const std::string& driver_name, const std::string& driver_desc,
                        const std::string& drv_param1, const std::string& drv_param2,
                        const std::string& drv_param3, const std::string& drv_param4);
    int AddDevice(CDevice* device_info);
    void InitDriverInterface();
    int HandleWriteCmd(std::string tag_name, std::string tag_value);
    void UpdateTagsData(LWTAG **tag, int tag_count);
    void UpdateTags2NodeServer();
    // 下发控制命令
    void PostControlCmd(CDevice *device, LWTAG *tag, std::string tag_value);
    int Start();
    void Stop();

    int OnStart();
    void OnStop();

    void DestroyDevices(); 

private:
    std::map<std::string, CDevice*> devices_; // map of device name to device object
    int InitListenerControl();

private:
    static void OnDatagramServerConnect(void *arg, ipc_client_auto_t *cliauto, bool connect);
    static void OnWriteCmd(vsoa_sdk::server::CliRpcInfo& cli, const void* dto, size_t len, void* arg);
    static void OnServerCmd(vsoa_sdk::server::CliRpcInfo& cli, const void* dto, size_t len, void* arg);
    static void OnNodeserverCb(void *arg, ipc_client_t *client, ipc_url_t *url, ipc_payload_t *payload);

private:
    int LWDRIVER_Reset(LWDRIVER *pDriver);
    void LWDRIVER_Free(LWDRIVER *pDriver);
private:
    std::string driver_name_;
    std::string driver_desc_;
    std::string drv_param1, drv_param2, drv_param3, drv_param4; // driver parameters
    int driver_status_; // 0: stopped, 1: running, 2: error
    int device_num_; // number of devices managed by this driver
    std::string last_error_msg_; // last error message
    std::string last_error_time_; // last error time

    LWDRIVER *driver_info_ = nullptr; // pointer to the driver info structure
    vsoa_sdk::server::ServerHandle *server_handle_ = nullptr; // 用于接收前端下发的控制命令
    std::string fix_data_cmd_ = "data"; // fixed read command for the driver
    std::string fix_ctrl_cmd_ = "control"; // fixed control command for the driver
    std::string publish_url_; // URL for publishing data
    std::string ctrl_url_;

    // node server 的客户端
    std::string node_server_path_;
    const std::string node_server_srvname_ = "node_server";
    ipc_url_t pub_url_ {
        .url = "/tags/update",
        .url_len = strlen(pub_url_.url)
    };
    ipc_url_t init_url_ {
        .url = "/tags/init",
        .url_len = strlen(init_url_.url)
    };
    ipc_client_auto_t *client_auto_ = nullptr;
    ipc_client_t *client_handle_ = nullptr; // client handle for vsoa server
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> obj_mapper_ = nullptr;
};
