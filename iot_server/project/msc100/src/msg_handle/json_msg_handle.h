#ifndef __INNER_MSG_HANDLE_H
#define __INNER_MSG_HANDLE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    E_DEV_REGISTER = 0x01,  /* 注册 */
    E_DEV_INFORM_STATUS,    /* 主动上报设备消息 */
} E_DEV_MSG;

typedef struct _MSG_CB_PARAM
{
    E_DEV_MSG e_msg,
    char cc_uuid[16];
    int  req_id;

    char str_arg1[16];
    char str_arg2[16];
    char str_arg3[16];
    char str_arg4[16];

    int  int_arg1;
    int  int_arg2;
    int  int_arg3;
    int  int_arg4;
    int  int_arg5;
    int  int_arg6;
    int  int_arg7;
    int  int_arg8;
} MSG_CB_PARAM;

typedef int (* json_msg_cb)(void * arg, MSG_CB_PARAM * cb_param);

typedef struct _JSON_MSG_HDL_OBJECT * JMH_HANDLE;

JMH_HANDLE inner_msg_handle_create(void);
void inner_msg_handle_set_cb(JMH_HANDLE handle, json_msg_cb cb, void * arg);
int  inner_msg_handle_extevt_msg_send(JMH_HANDLE handle, char * buffer, int len);
void inner_msg_handle_destroy(JMH_HANDLE handle);
int  inner_msg_handle_qt_msg_send(JMH_HANDLE handle, char * buffer, int len);

#ifdef __cplusplus
}
#endif

#endif

