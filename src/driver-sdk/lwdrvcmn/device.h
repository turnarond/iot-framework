/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: device.h .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "drvframework.h"
#include "lwdrvcmn.h"
#include "lwcommunicate/lwconn_device.h"

#include <queue>
#include <vector>
#include <string>
#include <map>
#include <memory>

class CUserTimer;
class CDriver;
class CDevice
{
private:
    int             polling_interval_;
    int             recv_timeout_;
    int             conn_timeout_;
    std::shared_ptr<LWConnDevice> lwconn_device_;

public:
    CDevice();
    ~CDevice();
    CDevice &operator=(CDevice &theDevice) = default;

public:
    void InitDeviceInfo(CDriver* drv, std::string name, std::string desc, std::string conn_type, std::string conn_param, 
        std::string param1, std::string param2, std::string param3, std::string param4);
    void Start();
    void StartCheckConnectTimer();
    void DestroyTimers();
    void Stop();

    int SendDevice(const char *szBuffer, int lBufLen, int lTimeOutMS );
    int RecvDevice(char *szBuff, int lBufLen, int lTimeOutMS); 
    int SendToDevice(char *szIpPort, const char *szBuffer, int lBufLen, int lTimeOutMS);
    int RecvFromDevice(char *szIpPort, int nIpPortBufLen, char *szBuff, int lBufLen, int lTimeOutMS);

    int InitConnectToDevice();

    void OnWriteCommand(LWTAG *pTag, std::string data);
    void SetConnectType(int type) { connect_type_ = type; }
    void SetPollingInterval(int interval) { polling_interval_ = interval; }
    std::string GetDeviceName() const { return device_name_; }

    // user timer interface
    CUserTimer * CreateAndStartTimer(DRVTIMER * timerInfo );
	DRVTIMER * GetTimers( int * pnTimerNum );
	int StopAndDestroyTimer(CUserTimer * pTimer );

    int DisconnectFromDevice();
    int ReConnectDevice(int nTimeOutMS);
    int CheckAndConnect(int nConnectTimeOutMS);

    void InitDeviceInterface();
    int AddTagOfDevice(const LWTAG *tags);

    int GetTagsByAddr(const char* addr, LWTAG **tag, int tag_count);
    int GetTagsByName(const char* name, LWTAG **ppTags, int tag_count);
    int UpdateTagsData(LWTAG **tag, int tag_count);
    std::map<std::string, LWTAG *> GetALLTags() const;

    LWDEVICE* GetDeviceInterface() { return device_info_; }
public:
    int SetDeviceConnected(bool bDevConnected);

private:
    CDriver *driver_info_;
    std::map<std::string, std::string> deviceip_to_server;

    std::vector<LWTAG *> device_all_tags_;
    std::map<std::string, LWTAG *> name_to_tags_;
    std::map<std::string, std::vector<LWTAG *>> addr_to_tags_;

    std::vector<CUserTimer*> vec_timers;
private:
    LWDEVICE *device_info_;
    std::string device_name_;
    std::string device_desc_;
    std::string device_conn_type_;
    std::string device_conn_param_;
    std::string device_param1_;
    std::string device_param2_;
    std::string device_param3_;
    std::string device_param4_;
    bool config_changed_ = false;

    std::string connect_type_;

    int LWDEVICE_Reset(LWDEVICE *device);
    void LWDEVICE_Free(LWDEVICE *device);
};
