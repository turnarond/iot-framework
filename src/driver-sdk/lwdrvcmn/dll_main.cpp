/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: dll_main.cpp .
*
* Date: 2025-05-28
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "lwlog/lwlog.h"
#include "lwcomm/lwcomm.h"
#include "lwdrvcmn.h"
#include "drvframework.h"

#include "maintask.h"

#include <string>
#include <dlfcn.h>

extern CLWLog g_logger;

DrvFramework DrvFramework::s_pInstance;
DrvFramework* DrvFramework::GetInstance()
{
    return &s_pInstance;
}

int DrvFramework::InitDrvFramework(std::string drv_name, std::string drv_path)
{
    drv_name_ = drv_name;
    drv_path_ = drv_path;

    int npos = drv_name_.find_last_of(LW_OS_DIR_SEPARATOR_CHAR);
    if (npos != std::string::npos) {
        drv_name_ = drv_name_.substr(npos + 1);
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Driver name: %s, Driver path: %s", 
        drv_name_.c_str(), drv_path_.c_str());
    return 0;
}

void DrvFramework::UninitDrvFramework()
{
    return;
}
int DrvFramework::LoadDrvFuncs()
{
#ifdef WIN32
    std::string dll_path = drv_path_ + LW_OS_DIR_SEPARATOR + drv_name_ + ".dll";
#else
    std::string dll_path = drv_path_ + LW_OS_DIR_SEPARATOR + "lib" + drv_name_ + ".so";
#endif

    std::string current_path = LWComm::GetBinPath();
    LWFileHelper::SetWorkingDirectory(current_path.c_str());
    char working_dir[LW_LONGFILENAME_MAXLEN] = {0};
    LWFileHelper::GetWorkingDirectory(working_dir, sizeof(working_dir));

    std::string driver_dir = current_path + LW_OS_DIR_SEPARATOR + "driver" + LW_OS_DIR_SEPARATOR + drv_name_;

#ifdef _WIN32
	char *szPathEnv = getenv("PATH");
	if (szPathEnv)
	{
		string strSysPath = getenv("PATH");
		strSysPath = strDriverDir + ";" + strCurDir + ";" + strSysPath;
		string strEnv = "PATH=";// c:\\windows";
		strEnv = strEnv + strSysPath;

		int nRet = putenv(strEnv.c_str());
		if (nRet != 0)
		{
			g_logger.LogMessage(PK_LOGLEVEL_CRITICAL, "Set Enviroment Variable path failed, ret:%d, %s", 
                nRet, strEnv.c_str());
		}
	}
#else
	char *szLibPath = getenv("LD_LIBRARY_PATH"); // DO NOT EXIST OF THIS LD_LIBRARY_PATH, return NULL!
	if (szLibPath)
	{
		string strSysPath = szLibPath;
		strSysPath = driver_dir + ":" + current_path + ":" + strSysPath;

		int nRet = setenv("LD_LIBRARY_PATH", strSysPath.c_str(), 1);
		if (nRet != 0)
		{
			g_logger.LogMessage(LW_LOGLEVEL_CRITICAL, "Set Enviroment Variable LD_LIBRARY_PATH failed, ret:%d, %s", 
                nRet, strSysPath.c_str());
		}
	}
#endif
#if 1
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Current working directory: %s, load %s",
        working_dir, dll_path.c_str());
    drv_handle_ = dlopen(dll_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!drv_handle_) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load driver library %s: %s", 
            dll_path.c_str(), dlerror());
        goto FAILED;
    }

    pfnInitDriver_ = (PFN_InitDriver)dlsym(drv_handle_, "InitDriver");
    pfnUnInitDriver_ = (PFN_UnInitDriver)dlsym(drv_handle_, "UnInitDriver");
    pfnInitDevice_ = (PFN_InitDevice)dlsym(drv_handle_, "InitDevice");
    if (pfnInitDevice_ == nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load function InitDevice from %s: %s", 
            dll_path.c_str(), dlerror());
        goto FAILED;
    }
    pfnUnInitDevice_ = (PFN_UnInitDevice)dlsym(drv_handle_, "UnInitDevice");
    pfnOnDeviceConnStateChanged_ = (PFN_OnDeviceConnStateChanged)dlsym(drv_handle_, "OnDeviceConnStateChanged");
    pfnOnTimer_ = (PFN_OnTimer)dlsym(drv_handle_, "OnTimer");
    if (pfnOnTimer_ == nullptr) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load function OnTimer from %s: %s", 
            dll_path.c_str(), dlerror());
        goto FAILED;
    }
    pfnOnControl_ = (PFN_OnControl)dlsym(drv_handle_, "OnControl");
    pfnGetVersion_ = (PFN_GetVersion)dlsym(drv_handle_, "GetVersion");
#else
    extern long OnTimer(LWDEVICE *pDevice, DRVTIMER *timerInfo);
    extern long InitDevice(LWDEVICE *pDevice);
    extern long UnInitDevice(LWDEVICE *pDevice);
    extern long OnControl(LWDEVICE *pDevice, LWTAG *pTag, const char *szBinValue, int nBinValueLen, long lCmdId);
    pfnInitDriver_ = nullptr;
    pfnUnInitDriver_ = nullptr;
    pfnInitDevice_ = InitDevice;
    pfnUnInitDevice_ = UnInitDevice;
    pfnOnTimer_ = OnTimer;
    pfnOnControl_ = OnControl;
#endif
    return 0;

FAILED:
    g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to load driver functions from %s: %s", dll_path.c_str(), dlerror());
    if (drv_handle_) {
        dlclose(drv_handle_);
        drv_handle_ = nullptr;
    }
    return -1;
}

LWDRIVER_EXPORTS int drvMain (char *drv_name, char *drv_path)
{
    printf("drvMain called with drv_name: %s, drv_path: %s\n", drv_name, drv_path);
    g_logger.SetLogFileName(drv_name);
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Driver %s started with path %s", drv_name, drv_path);
    // Initialize the driver here

    int ret = DRV_FRAMEWORK->InitDrvFramework(drv_name, drv_path);
    if (ret != LW_SUCCESS) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Init driver framework failed, ret: %d", ret);
        return ret;
    }

    ret = DRV_FRAMEWORK->LoadDrvFuncs();
    if (ret != LW_SUCCESS) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Load driver functions failed, ret: %d", ret);
        return ret;
    }

    MAINTASK->Start();

    // Wait for the main task to finish
    DRV_FRAMEWORK->RunInteractiveLoop();

    MAINTASK->Stop();

    DRV_FRAMEWORK->UninitDrvFramework();
    return LW_SUCCESS;
}

void DrvFramework::RunInteractiveLoop()
{
    printf(">>");
    while (bdriver_alive_) {
        std::string cmd;
        LWComm::Sleep(1000);
    }
}
