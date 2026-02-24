/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: user_timer.cpp .
*
* Date: 2025-06-19
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "user_timer.h"
#include "lwdrvcmn.h"
#include "platform_sdk/timer.h"
#include "drvframework.h"

#include <cstring>

CUserTimer::CUserTimer(CDevice *device)
    : device_(device), timer_id_(NULL)
{
    memset(&timer_info_, 0, sizeof(timer_info_));
}

CUserTimer::~CUserTimer()
{

}

void CUserTimer::HandleTimeout (void *arg, void* timer)
{
    if (arg == nullptr || timer == nullptr) {
        return;
    }
    CUserTimer* user_timer = (CUserTimer*)arg;
    if (DRV_FRAMEWORK->pfnOnTimer_) {
        DRV_FRAMEWORK->pfnOnTimer_(user_timer->device_->GetDeviceInterface(),
            &user_timer->timer_info_);
    }
}

void CUserTimer::StartTimer()
{
    if (device_ == nullptr) {
        // Handle error: device or timer info is null
        return;
    }

    timer_id_ = TimerTaskAdd(1, timer_info_.period_ms, HandleTimeout, this);
}

void CUserTimer::StopTimer()
{
    if (device_ == nullptr) {
        // Handle error: device or timer info is null
        return;
    }

    // Stop the timer (implementation depends on your framework)
    // For example, you might use a platform-specific API to stop the timer
}

