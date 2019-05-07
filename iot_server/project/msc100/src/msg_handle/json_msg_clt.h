#ifndef __JSON_MSG_CLT_H
#define __JSON_MSG_CLT_H

#include "../device_manage/clt_param.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CLT_STRING_LEN  32

typedef enum
{
    E_DEV_GET_PARAM = 0x01, /* 获取参数 */
    E_DEV_HEART_BEAT,       /* 心跳 */
} E_CLT_MSG;

typedef struct _CLT_MSG_CB_PARAM
{
    E_CLT_MSG e_msg;
    int  req_id;
    char gopenid[MAX_OPEN_ID_LEN];  /* group open id */
    char str_arg[MAX_OPEN_ID_ARRAY][MAX_CLT_STRING_LEN];
    int  int_arg1[MAX_OPEN_ID_ARRAY];
    int  int_arg2[MAX_OPEN_ID_ARRAY];
} CLT_MSG_CB_PARAM;

typedef int (* json_msg_cb)(void * arg, CLT_MSG_CB_PARAM * cb_param, void * ext_arg);

typedef struct _JSON_MSG_CLT_OBJECT * JMC_HANDLE;

JMC_HANDLE json_msg_clt_create(void);
int  json_msg_clt_msg(JMC_HANDLE handle, char * buffer, int len, char * resp_buf, int resp_buf_len, void * ext_arg);
void json_msg_clt_set_cb(JMC_HANDLE handle, json_msg_cb cb, void * arg);
void json_msg_clt_destroy(JMC_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif
