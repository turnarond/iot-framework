/*
 * Copyright (c) 2024 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: lwevent.h 
 *
 * Date: 2023-04-27
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#ifndef _LWEVENT_H_
#define _LWEVENT_H_

#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct event_t *EVENT_HANDLE;

    #define EVENT_SUCCESS   0   /** 事件成功 */
    #define EVENT_TIMEOUT   1   /** 事件等待超时 */
    #define EVENT_ERROR    -1   /** 事件等待失败 */

    /**
     * @brief 创建事件
     * @param manual_reset 是否需要手动恢复事件
     * @param init_state 事件是否初始化就绪
     * @return EVENT_HANDLE NULL 出错
     */
    EVENT_HANDLE EventCreate(bool manual_reset, bool init_state);

    /**
     * @brief
     * @param hevent
     * @return int 0 等到事件，-1出错
     */
    int EventWait(EVENT_HANDLE hevent);

    /**
     * @brief
     * @param hevent
     * @param milliseconds
     * @return int 0 等到事件，-1出错
     */
    int EventTimedWait(EVENT_HANDLE hevent, long milliseconds);

    /**
     * @brief
     * @param hevent
     * @return int 0 成功，-1出错
     */
    int EventSet(EVENT_HANDLE hevent);

    /**
     * @brief
     * @param hevent
     * @return int 0 成功，-1出错
     */
    int EventReset(EVENT_HANDLE hevent);

    /**
     * @brief 销毁事件，不推荐使用，使用 EventDestroySafe 代替。
     * @param hevent
     */
    void EventDestroy(EVENT_HANDLE hevent);

    /**
     * @brief 安全销毁事件并置空指针（仅当确保无其他线程使用时调用）
     * 此函数将 *hevent 设为 NULL，防止调用者后续误用。
     * 仍需保证调用时无任何线程正在访问该事件对象！
     * @param hevent
     */
    void EventDestroySafe (EVENT_HANDLE* hevent);

#ifdef __cplusplus
}
#endif

#endif // _LWEVENT_H_

/*
 * end
 */