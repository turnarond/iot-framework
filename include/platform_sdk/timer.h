/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: timer.h .
*
* Date: 2024-07-15
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/
#pragma once


#include <stdbool.h>
#include <stdint.h>

#define LWTIMER void*

typedef void(*PfnTimerHandlerCb)(void *arg, void *timer);

#define MAX_TIMER_FD  64

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize timer service 
 * The initialization function must first be called */
bool TimerInit(void);

/* Timer task start.
 * The time unit is millisecond, if `interval` is 0, it means trigger once */
LWTIMER TimerTaskAdd(int64_t count, int64_t interval, PfnTimerHandlerCb cb, void* user_ctx);

/* Timer delete */
int TimerTaskDelete(LWTIMER task);

#ifdef __cplusplus
}
#endif




// Package the cpp timer class
#define TIMERAPI_EXPORT __attribute__((visibility("default")))

// Structure declaration
struct vsoa_timer;

// Timer business callback function, pContext is the caller context
typedef void (*OnTimerCallback)(void *pContext);


namespace vsoa {

class TIMERAPI_EXPORT Timer
{
public:
    // the attributes of timer
    struct TimerAttributes
    {
        bool bUseNanoSec = false; // use nanoseconds to timing if it equal true
        int64_t i64IntervalMs = 0;// the timer interval, default unit is milliseconds, if 'bUseNanoSec = true' it unit is nanoseconds
        void *pContext = nullptr; // the caller context pointer
        OnTimerCallback pfn = nullptr; // the timer task function
    };

public:
    /*********************************************************************
     * description: construct function
     * *******************************************************************/
    Timer();

    /*********************************************************************
     * description: destruct function
     * *******************************************************************/
    ~Timer();

    /*********************************************************************
     * description: start timer task
     * input params:
     *      stAttr: the attributes information of timer
     * return value:
     *      bool: call result, true -- success, false -- failed.
     * *******************************************************************/
    bool startTimer(const vsoa::Timer::TimerAttributes& stAttr);

    /*********************************************************************
     * description: stop timer task
     * *******************************************************************/
    void stopTimer();

private:
    vsoa_timer *m_pVsoaTimer = nullptr; // vsoa_timer instance pointer

};


} // namespace vsoa

/*
 * end.
 */


