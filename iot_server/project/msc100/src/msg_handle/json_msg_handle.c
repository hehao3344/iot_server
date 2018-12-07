#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <json/cjson.h>

#include "json_msg_handle.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 类型定义                                         *
 *----------------------------------------------*/
typedef struct _JSON_MSG_HDL_OBJECT
{
    json_msg_cb cb;
    void * arg;
} JSON_MSG_HDL_OBJECT;

/*----------------------------------------------*
 * 内部函数声明                                       *
 *----------------------------------------------*/
static int inner_msg_get_is_request(char * buffer, int len);
static int msg_is_inner_response(char * buffer);
static void inner_msg_pull_center(long user_info);

JMH_HANDLE json_msg_handle_create(void)
{
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)calloc(1, sizeof(JSON_MSG_HDL_OBJECT));
    if (NULL == handle)
    {
        control_err("malloc failed \n");
        return NULL;
    }

    return handle;
create_failed:
    json_msg_handle_destroy(handle);
    return NULL;
}

int json_msg_handle_msg(JMH_HANDLE handle, char * buffer, int len)
{
    if ((NULL == handle) || (NULL == buffer))
    {
        debug_err("invalid param \n");
        return -1;
    }


}

void json_msg_handle_set_cb(JMH_HANDLE handle, json_msg_cb cb, void * arg)
{
    if (NULL == handle)
    {
        return;
    }
    handle->cb  = cb;
    handle->arg = arg;
}

void json_msg_handle_destroy(JMH_HANDLE handle)
{
    if (NULL != handle)
    {
        if (NULL != handle->rthread_inner_msg)
        {
            rthread_delete(handle->rthread_inner_msg,    handle->ttask_inner_msg);
            rthread_wait_exit(handle->rthread_inner_msg, handle->ttask_inner_msg);
            rthread_free(handle->rthread_inner_msg,      handle->ttask_inner_msg);
            handle->ttask_inner_msg = NULL;
        }
        if (NULL != handle->im_qt_handle)
        {
            inner_msg_destroy(handle->im_qt_handle);
        }

        free(handle);
    }
}

/*******************************************************************************
* static function.
*******************************************************************************/
static void inner_msg_pull_center(long user_info)
{
#define MAX_RECV_BUF_LEN   1024
    int ret = -1;
    // char response[512];
    char * recv_buf = malloc(MAX_RECV_BUF_LEN);
    if (NULL == recv_buf)
    {
        control_err("not enough memory \n");
        return;
    }
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)user_info;
    if (NULL == handle)
    {
        control_err("invalid param \n");
        return;
    }

    while (1)
    {
        if (1 == rthread_is_destroyed(handle->rthread_inner_msg, handle->ttask_inner_msg))
        {
            control_info("inform_center destroyed \n");
            break;
        }

        /* 注意 从 im_qt_handle里收到的数据有可能来自external_event */
        ret = inner_msg_recv(handle->im_qt_handle, recv_buf, MAX_RECV_BUF_LEN);
        if (-1 == ret)
        {
            control_err("inner_msg_recv failed \n");
            continue;
        }
        if (1 == msg_is_inner_response(recv_buf))
        {
            control_info("recv inner response msg \n");
            continue;
        }

        if (IOT_MODULE_QT == ret)
        {
            if (1 == inner_msg_get_is_request(recv_buf, strlen(recv_buf)))
            {
                if (NULL != handle->msg_cb_fn)
                {
                    handle->msg_cb_fn(handle->arg, E_QT, 0, NULL, 0);
                }
            }
        }
    }

    free(recv_buf);
}

#if 0
{
"method":"up_msg",
"cc_uuid":"10001122334455",
"req_id":123456789,
"attr":
{
    "cmd":"register",
    "version":"V2.01.01"
}
}
#endif
static int parse_device_msg(char * buffer, MSG_CB_PARAM * cb_param)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;

    root = cJSON_Parse(buffer);
    if (NULL == root)
    {
        debug_print("cJSON_Parse error!\n");
        return -1;
    }

    sub_obj = cJSON_GetObjectItem(root, "method");
    if (NULL != sub_obj)
    {
        if (0 != strcmp(sub_obj->valuestring, "up_msg"))
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    sub_obj = cJSON_GetObjectItem(root, "cc_uuid");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param->cc_uuid))
        {
            strncpy(cb_param->cc_uuid, sub_obj->valuestring, sizeof(cb_param->cc_uuid));
        }
    }
    sub_obj = cJSON_GetObjectItem(root, "req_id");
    if (NULL != sub_obj)
    {
        cb_param->req_id = sub_obj->valueint;
    }

    cJSON * attr_obj = cJSON_GetObjectItem(sub_obj, "attr");
    if (NULL == attr_obj)
    {
        debug_error("can't find attr \n");
        return -1;
    }

    cJSON * cmd_obj = cJSON_GetObjectItem(attr_obj, "cmd");
    if (NULL != cmd_obj)
    {
        if (0 == strcmp(cmd_obj->valuestring, "register"))
        {
            cb_param->e_msg = E_DEV_REGISTER;
        }
    }

    cJSON * ver_obj = cJSON_GetObjectItem(attr_obj, "version");
    if (NULL != ver_obj)
    {
        if (strlen(ver_obj->valuestring) < sizeof(cb_param->str_arg1))
        {
            strncpy(cb_param->str_arg1, sub_obj->valuestring, sizeof(cb_param->str_arg1));
        }
    }


        cb_param->req_id = sub_obj->valueint;

    return ret;
}

#if 0
{
"method": "request",
"node_id": "real_param",
"params":
{
    "value": "dummy"
}
}
#endif
/* -1 错误 1 该消息为请求实时数据 */
static int inner_msg_get_is_request(char * buffer, int len)
{
    cJSON *root = NULL;
    UNUSED_VALUE(len);
    root = iot_json_parse(buffer);
    if (NULL == root)
    {
        control_err("cJSON_Parse error!\n");
        return -1;
    }

    char node_id[32] = {0};
    if (0 != iot_json_get_string(root, "node_id", node_id, sizeof(node_id)))
    {
        control_err("get node_id failed \n");
        return -1;
    }

    if (0 != strcmp(node_id, "real_param"))
    {
        control_err("invalid node_id %s \n", node_id);
        return -1;
    }

    return 1;
}

