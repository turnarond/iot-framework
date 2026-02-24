/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: maintask.h .
*
* Date: 2025-05-30
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "drvframework.h"
#include "lwdrvcmn.h"
#include "driver.h"
#include "lwlog/lwlog.h"
#include "lwcomm/lwcomm.h"
#include <map>

#include <thread>

class CMainTask
{
public:
    CMainTask();
    ~CMainTask();

    int Start();
    void Stop();

    int CalcDriverTagDataSize(unsigned int *pnTagCount, unsigned int *pnTagDataSize, unsigned int *pnTagMaxDataSize);
    int ProcessWriteCmds(std::string &strCmds);
private:

    // 线程初始化
    int OnStart();

    // 线程中停止
    int OnStop();

    static void svc_thread_main(void *arg);

    int LoadConfig();

private:
    // 处理控制命令，json 格式
    int ProcessWriteOneCmd(std::string ctrl_cmd);
public:
    static CMainTask main_task_;
    static CMainTask *GetInstance();

private:
    bool bstop_ = false; // 线程停止标志
    std::thread main_thread_;
    CDriver drive_info_;
    int     tag_num_; // 该驱动的tag点个数
    int     device_num_; // 该驱动的设备数量
};

#define MAINTASK CMainTask::GetInstance()
