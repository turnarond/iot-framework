/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwmsgq.h .
*
* Date: 2024-06-07
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef __MSGQUE_H__
#define __MSGQUE_H__

#include <pthread.h>
#include "list.h"
#include "lwevent/lwevent.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define LW_QUE_MAXSIZE 100000000

#ifdef __cplusplus
extern "C"{
#endif

/* 
 * User callback
 */
typedef int (*pfQueUserHandleCb)(void* msg, int msglen, void* userctx); 

/*
 * User context.
 */
typedef struct tagMsgUserCtx
{    
    void* pvUserCtx; /* User context. */
    pfQueUserHandleCb pfUserHandler; /* User handler.*/
}MsgUsrCtx;

/*
 * Message queue.Designed with object pool.
 * used_list: used list.
 * idle_list: idle list.
 * 
 */
typedef struct tagMsgQueue
{
    int capacity; /* capacity of this msgq */
    LW_DLIST_NODE_S idle_list; /* idle list */
    int idle_nums; /* counts of idle list */
    LW_DLIST_NODE_S used_list; /* used list */
    int used_nums; /* counts of used list */
    pthread_mutex_t msg_mutex; /* mutex of list */
    EVENT_HANDLE msg_push_notify; /* msg pushed event */
    MsgUsrCtx user_ctx; /*user context*/
}LW_MSGQUE_S, *PLW_MSGQUE_S;

/**
 * @brief Create queue.
 * @param queue_size alloc size of queue.
 * @param user_handle_cb handler callback of this msg queue
 * @param ctx user context of this callback
 * @return PLW_MSGQUE_S 
 */
PLW_MSGQUE_S MsgQueCreate(int queue_size, pfQueUserHandleCb user_handle_cb, void *ctx);

/**
 * @brief Release queue
 * @param ppstQueue 
 */
void MsgQueRelease(PLW_MSGQUE_S *msgq);

/**
 * @brief Push msg to this queue.
 * @param msgq Msgq handle 
 * @param pvMsg User msg to push.
 * @param iMsgLen Length of this user msg.
 * @return int 
 */
int MsgQuePush(PLW_MSGQUE_S msgq, void *msg, int msg_len);

/**
 * @brief Push msg to the msgq.If msgq is full, then pop the front msg of this queue.
 * @param msgq 
 * @param msg User msg to push.
 * @param msg_len Length of this user msg.
 * @param p_popped_msg .The msgq popped from the queue.
 * @return int .0 for success.
 */
int MsgQuePushWithEvict(PLW_MSGQUE_S msgq, void* msg, int msg_len, void **p_popped_msg);

/**
 * @brief Push msg to the head of queue.
 * @param msgq 
 * @param msg 
 * @param msg_len 
 * @return int 
 */
int MsgQuePushHead(PLW_MSGQUE_S msgq, void *msg, int msg_len);

/**
 * @brief Pop msg from the queue.
 * @param msgq 
 * @param msg The popped msg.
 * @param len Length of this user msg.
 * @return int .0 for success.
 */
int MsgQuePop(PLW_MSGQUE_S msgq, void **msg, int *len);

/**
 * @brief Try pop msg from the queue.
 * @param msgq 
 * @param msg The popped msg.
 * @param len Length of this user msg.
 * @return int .0 for success.
 */
int MsgQueTryPop(PLW_MSGQUE_S msgq, void **msg, int *len);

/**
 * @brief Stop this msgq
 * @param msgq 
 * @return int 
 */
int MsgQueStop(PLW_MSGQUE_S msgq);

/**
 * @brief Waitingfor event.
 * @param msgq 
 * @param piErrorCode 
 * @return int 
 */
int MsgQueWorkerWaitfor(PLW_MSGQUE_S msgq, int *errcode);

/**
 * @brief Get the used counts of queue.
 * @param msgq 
 * @return int 
 */
int MsgQueGetUsedNums(PLW_MSGQUE_S msgq);

#ifdef __cplusplus
}
#endif

#endif

/*
 * end
 */
