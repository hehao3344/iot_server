#ifndef __JSON_MSG_HANDLE_H
#define __JSON_MSG_HANDLE_H

#include "../device_manage/dev_param.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_STRING_LEN  32
#define MAX_APP_ID_LEN  32

typedef enum
{
    E_DEV_REGISTER = 0x01,  /* 注册 */
    E_DEV_HEART_BEAT,       /* 设备心跳 */
    E_DEV_INFORM_STATUS,    /* 主动上报设备消息 */
    E_DEV_FW_REQUEST,       /* ota */
} E_DEV_MSG;

typedef struct _MSG_CB_PARAM
{
    E_DEV_MSG e_msg;
    int  req_id;
    char cc_uuid[MAX_ID_LEN];
    char str_arg[MAX_ID_ARRAY][MAX_STRING_LEN];
    char app_id[MAX_APP_ID_LEN];
    int  int_arg1[MAX_ID_ARRAY];
    int  int_arg2[MAX_ID_ARRAY];
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

