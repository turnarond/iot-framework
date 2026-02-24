/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: drvframework.h .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "lwdrvcmn.h"

#include <string>

typedef long (*PFN_InitDriver)(LWDRIVER *pDriver);
typedef long (*PFN_InitDevice)(LWDEVICE *pDevice);
typedef long (*PFN_UnInitDevice)(LWDEVICE *pDevice);
typedef void (*PFN_OnDeviceConnStateChanged)(LWDEVICE *pDevice, int nConnectState);

typedef long (*PFN_OnTimer)(LWDEVICE *pDevice, DRVTIMER *timeInfo);
typedef long (*PFN_OnControl)(LWDEVICE *pDevice, LWTAG *pTag, const char *szStrValue, int nBinValueLen, long lCmdId);

typedef long (*PFN_UnInitDriver)(LWDRIVER *pDriver);
typedef long (*PFN_GetVersion)();

class DrvFramework
{
    DrvFramework()
    {
        pfnInitDriver_ = nullptr;
        pfnUnInitDriver_ = nullptr;
        pfnInitDevice_ = nullptr;
        pfnUnInitDevice_ = nullptr;
        pfnOnDeviceConnStateChanged_ = nullptr;
        pfnOnTimer_ = nullptr;
        pfnOnControl_ = nullptr;
        pfnGetVersion_ = nullptr;
    }
    ~DrvFramework() = default;

private:
    std::string drv_name_;
    std::string drv_extname_;
    std::string drv_path_;

public:
    PFN_InitDriver       pfnInitDriver_;
    PFN_UnInitDriver     pfnUnInitDriver_;
    PFN_InitDevice       pfnInitDevice_;
    PFN_UnInitDevice     pfnUnInitDevice_;
    PFN_OnDeviceConnStateChanged     pfnOnDeviceConnStateChanged_;
    PFN_OnTimer          pfnOnTimer_;
    PFN_OnControl        pfnOnControl_;
    PFN_GetVersion       pfnGetVersion_;

private:
    void *drv_handle_ = nullptr;
    bool bdriver_alive_ = true;

public:
    int InitDrvFramework(std::string drv_name, std::string drv_path);
    int LoadDrvFuncs();
    void RunInteractiveLoop();
    void UninitDrvFramework();
    std::string GetDrvName() const { return drv_name_; }
    std::string GetDrvExtName() const { return drv_extname_; }
    std::string GetDrvPath() const { return drv_path_; }

    int GetTagDataTypeAndLen(std::string& datatype, int datalen, int &datatype_id, int &datalen_out);

    const std::string GetTagDataValueToString(int datatype_id, int datalen, const char *data);
public:
    static DrvFramework s_pInstance;
public:
    static DrvFramework* GetInstance();
};

#define DRV_FRAMEWORK DrvFramework::GetInstance()