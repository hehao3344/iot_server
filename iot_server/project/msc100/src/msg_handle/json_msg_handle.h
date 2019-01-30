#ifndef __JSON_MSG_HANDLE_H
#define __JSON_MSG_HANDLE_H

#include "../device_manage/dev_param.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_STRING_LEN  32

typedef enum
{
    E_DEV_REGISTER = 0x01,  /* 注册 */
    E_DEV_INFORM_STATUS,    /* 主动上报设备消息 */
} E_DEV_MSG;

typedef struct _MSG_CB_PARAM
{
    E_DEV_MSG e_msg;
    int  req_id;

    char cc_uuid[MAX_ID_LEN];
    char str_arg1[MAX_STRING_LEN];
    char str_arg2[MAX_STRING_LEN];
    char str_arg3[MAX_STRING_LEN];
    char str_arg4[MAX_STRING_LEN];

    int  int_arg1;
    int  int_arg2;
    int  int_arg3;
    int  int_arg4;
    int  int_arg5;
    int  int_arg6;
    int  int_arg7;
    int  int_arg8;
} MSG_CB_PARAM;

typedef int (* json_msg_cb)(void * arg, MSG_CB_PARAM * cb_param, void * ext_arg);

typedef struct _JSON_MSG_HDL_OBJECT * JMH_HANDLE;

JMH_HANDLE json_msg_handle_create(void);
int  json_msg_handle_msg(JMH_HANDLE handle, char * buffer, int len, char * resp_buf, int resp_buf_len, void * ext_arg);
void json_msg_handle_set_cb(JMH_HANDLE handle, json_msg_cb cb, void * arg);
void json_msg_handle_destroy(JMH_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif

