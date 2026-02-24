/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: user_timer.h .
*
* Date: 2025-06-19
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "lwdrvcmn.h"
#include "device.h"

class CUserTimer
{
    friend class CDevice;
public:
    CUserTimer(CDevice *device);
    virtual ~CUserTimer();
    void StartTimer();
	void StopTimer();
    static void HandleTimeout (void *arg, void* timer);
private:
    DRVTIMER timer_info_; // timer information
    CDevice *device_; // device associated with the timer
    void* timer_id_; // unique identifier for the timer
};
