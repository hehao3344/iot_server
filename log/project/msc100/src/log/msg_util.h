#ifndef _MSG_UTIL_H_
#define _MSG_UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/* 消息队列编号 */
typedef enum
{
    IOT_MSG_SQUEUE_LOCAL,         /* 内部协议模块之间的消息队列 */
    IOT_MSG_SQUEUE_REMOTE,        /* tcp通信和远程协议模块之间的消息队列 */
    IOT_MSG_SQUEUE_DEBUG,         /* log与各个模块之间的通信消息队列 */
}IOT_MSG_SQUEUE_NO;

/* 模块ID，有增删时，需同步更新 IOT_MODULE_ID_NUM 值 */
/* bit 位编码，0x80,0x100等 */
typedef enum
{
    IOT_MODULE_NONE             = 0x00,
    IOT_MODULE_IOT_SERVER       = 0x01,
    IOT_MODULE_DEBUG            = 0x02,
    IOT_MODULE_LOG              = 0x04  /*log module self*/
} IOT_MODULE_ID;

/* 模块ID个数，需与 IOT_MODULE_ID 同步 */
#define IOT_MODULE_ID_NUM       4
#define IOT_MSG_BODY_LEN_MAX    4000

typedef struct
{
    long dst_mod;
    long src_mod;
    long handler_mod;
    unsigned int body_len;
    unsigned int req_flag;          /* 0: requrire,1: ack */
    unsigned int req_id;             /* require id for msg communication */
    unsigned char body[IOT_MSG_BODY_LEN_MAX];
} IOT_MSG;

/*
  get or create a msg queue
  [in]queue_no: queue number to get
  [out]return:     failed == -1; success != -1
*/
int msg_queue_get(IOT_MSG_SQUEUE_NO queue_no);

/*
  put msg to a msg queue

  [in]queue_id:         iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:     the module id to send
  [in]msg->src_mod:     msg creator's module id
  [in]msg->handler_mod: the module need to deal the msg(can set to none)
  [in]msg->body_len:the msg body's length
  [in]msg->body:    the msg body's context
  [out]return:          failed == -1; success == 0
*/
int msg_send_nowait(int queue_id, IOT_MSG * msg);

/*
  get msg from a msg squeue, if none, waiting

  [in]queue_id:          iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:      the module id to send.(aller's module id)
  [out]msg->src_mod:     msg creator's module id
  [out]msg->handler_mod: the module need to deal the msg(can set to none)
  [out]msg->body_len:    the msg body's length
  [out]msg->body:        the msg body's context
  [out]return:           failed < 0; success == msg len
*/
int msg_recv_wait(int queue_id, IOT_MSG * msg_buf);

/*
  get msg from a msg squeue, if none, return

  [in]queue_id:          iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:      the module id to send.(aller's module id)
  [out]msg->src_mod:     msg sender's module id
  [out]msg->handler_mod: the module need to deal the msg(can set to none)
  [out]msg->body_len:    the msg body's length
  [out]msg->body:        the msg body's context
  [out]return:           failed < 0; success == msg len or 0
*/
int msg_recv_nowait(int queue_id, IOT_MSG * msg_buf);


/*
  get msg from a msg squeue, if none, return

  [in]queue_id:          iot_msg_queue_get return value. (!=-1)
  [in]msg->dst_mod:      the module id to send.(aller's module id)
  [out]msg->src_mod:     msg sender's module id
  [out]msg->handler_mod: the module need to deal the msg(can set to none)
  [out]msg->body_len:    the msg body's length
  [out]msg->body:        the msg body's context
  [out]return:           failed < 0; success == msg len or 0
*/
int msg_recv_log_wait(int queue_id, IOT_MSG *msg);

/* Func Desc: clear msg queue of queue_id, where dst module equal to dst_mod
 * Func Para: queue_id: queue id for clear
 * Func Para: dst_mod: specified the dst module
 * Func return: void
 */
void msg_clear_queue(int queue_id, long dst_mod);

#endif

