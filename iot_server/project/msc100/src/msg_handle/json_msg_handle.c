#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <json/cjson.h>
#include <core/core.h>

#include "json_msg_handle.h"

typedef int (*exec_func)(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);

typedef struct _JSON_MSG_HDL_OBJECT
{
    json_msg_cb cb;
    void * arg;
} JSON_MSG_HDL_OBJECT;

typedef struct _JSON_HANDLE_PARAM
{
    char * method;
    char * cmd;
    exec_func exec_fn;
    int  send_resp;
} JSON_HANDLE_PARAM;

static int dev_up_msg_register(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int dev_up_msg_heart_beat(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int dev_up_msg_update_status(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int dev_up_msg_fw_request(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);

static JSON_HANDLE_PARAM json_handle_tbl[] =
{
    {"up_msg",      "register",         dev_up_msg_register,        1},  /* 注册 */
    {"up_msg",      "heart_beat",       dev_up_msg_heart_beat,      1},  /* 心跳 */
    {"report_msg",  "updata_status",    dev_up_msg_update_status,   0},  /* 上报消息 */
    {"up_msg",      "fw_request",       dev_up_msg_fw_request,      1},  /* OTA */
};

JMH_HANDLE json_msg_handle_create(void)
{
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)calloc(1, sizeof(JSON_MSG_HDL_OBJECT));
    if (NULL == handle)
    {
        debug_error("malloc failed \n");
        return NULL;
    }

    return handle;
}

/* 调用者可以使用 strlen(resp_buf)的长度判断是否需要发送 */
int json_msg_handle_msg(JMH_HANDLE handle, char * buffer, int len, char * resp_buf, int resp_buf_len, void * ext_arg)
{
    if ((NULL == handle) || (NULL == buffer))
    {
        debug_error("invalid param \n");
        return -1;
    }

    cJSON *root     = NULL;
    cJSON *method_obj  = NULL;

    cJSON *attr_obj  = NULL;
    cJSON *cmd_obj  = NULL;

    root = cJSON_Parse(buffer);
    if (NULL == root)
    {
        debug_error("cJSON_Parse error [%s] !\n", buffer);
        return -1;
    }

    //{"method":"up_msg","cc_uuid":"1084f3eb83a7aa","req_id":100,"attr":{"cmd":"register","version":"V0.01"}}
    attr_obj   = cJSON_GetObjectItem(root, "attr");
    method_obj = cJSON_GetObjectItem(root, "method");
    if ((NULL != attr_obj) && (NULL != method_obj))
    {
        cmd_obj = cJSON_GetObjectItem(attr_obj, "cmd");
        if (NULL != cmd_obj)
        {
            int i;
            for (i=0; i<ARRAY_SIZE(json_handle_tbl); i++)
            {
                if ((0 == strcmp(json_handle_tbl[i].method, method_obj->valuestring)) &&
                    (0 == strcmp(json_handle_tbl[i].cmd, cmd_obj->valuestring)))
                {
                    json_handle_tbl[i].exec_fn(handle, buffer, resp_buf, resp_buf_len, ext_arg);
                    break;
                }
            }
        }
    }
    else
    {
        debug_info("json invalid [%s] \n", buffer);
    }

    cJSON_Delete(root);

    return 0;
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
        free(handle);
    }
}

/*******************************************************************************
* static function.
*******************************************************************************/
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
static int dev_up_msg_register(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    MSG_CB_PARAM  cb_param;
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }

    memset(&cb_param, 0, sizeof(MSG_CB_PARAM));
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
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.cc_uuid))
        {
            strncpy(cb_param.cc_uuid, sub_obj->valuestring, sizeof(cb_param.cc_uuid));
        }
    }
    sub_obj = cJSON_GetObjectItem(root, "req_id");
    if (NULL != sub_obj)
    {
        cb_param.req_id = sub_obj->valueint;
    }

    cJSON * attr_obj = cJSON_GetObjectItem(root, "attr");
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
            cb_param.e_msg = E_DEV_REGISTER;
        }
    }
    cJSON * ver_obj = cJSON_GetObjectItem(attr_obj, "version");
    if (NULL != ver_obj)
    {
        if (strlen(ver_obj->valuestring) < sizeof(cb_param.str_arg))
        {
            strncpy(cb_param.str_arg[0], ver_obj->valuestring, sizeof(cb_param.str_arg[0]));
        }
    }

    cb_param.req_id = sub_obj->valueint;
    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;
}

#if 0
{
"method":"up_msg",
"cc_uuid":"10001122334455",
"req_id":123456789,
"attr":
{
    "cmd":"heart_beat",
}
}
#endif
static int dev_up_msg_heart_beat(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    MSG_CB_PARAM  cb_param;
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }

    memset(&cb_param, 0, sizeof(MSG_CB_PARAM));
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
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.cc_uuid))
        {
            strncpy(cb_param.cc_uuid, sub_obj->valuestring, sizeof(cb_param.cc_uuid));
        }
    }
    sub_obj = cJSON_GetObjectItem(root, "req_id");
    if (NULL != sub_obj)
    {
        cb_param.req_id = sub_obj->valueint;
    }

    cJSON * attr_obj = cJSON_GetObjectItem(root, "attr");
    if (NULL == attr_obj)
    {
        debug_error("can't find attr \n");
        return -1;
    }

    cJSON * cmd_obj = cJSON_GetObjectItem(attr_obj, "cmd");
    if (NULL != cmd_obj)
    {
        if (0 == strcmp(cmd_obj->valuestring, "heart_beat"))
        {
            cb_param.e_msg = E_DEV_HEART_BEAT;
        }
    }

    cb_param.req_id = sub_obj->valueint;
    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;
}

#if 0
{
"method":"report_msg",
"cc_uuid":"10001122334455",
"dev1":
{
"dev_uuid":"02001122334455",
"online":"yes",
"switch":"off"
},
"dev2":{
"dev_uuid":"02001122334456",
"online":"no",
"switch":"off"
},
"dev3":
{
"dev_uuid":"02001122334457",
"online":"yes",
"switch":"on"
},
"dev4":
{
"dev_uuid":"02001122334458",
"online":"yes",
"switch":"on"
}
}
#endif
static int dev_up_msg_update_status(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    MSG_CB_PARAM  cb_param;
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }

    memset(&cb_param, 0, sizeof(MSG_CB_PARAM));
    root = cJSON_Parse(buffer);
    if (NULL == root)
    {
        debug_print("cJSON_Parse error!\n");
        return -1;
    }

    sub_obj = cJSON_GetObjectItem(root, "method");
    if (NULL != sub_obj)
    {
        if (0 != strcmp(sub_obj->valuestring, "report_msg"))
        {
            return -1;
        }
        cb_param.e_msg = E_DEV_INFORM_STATUS;
    }
    else
    {
        return -1;
    }

    sub_obj = cJSON_GetObjectItem(root, "cc_uuid");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.cc_uuid))
        {
            strncpy(cb_param.cc_uuid, sub_obj->valuestring, sizeof(cb_param.cc_uuid));
        }
    }

    int i;
    for (i=0; i<4; i++)
    {
        char dev_buf[8];
        memset(dev_buf, 0, sizeof(dev_buf));
        snprintf(dev_buf, sizeof(dev_buf), "dev%d", i+1);

        cJSON * dev_obj = cJSON_GetObjectItem(root, dev_buf);
        if (NULL == dev_obj)
        {
            debug_error("can't find dev_obj \n");
            return -1;
        }
        cJSON * du_obj = cJSON_GetObjectItem(dev_obj, "dev_uuid");
        if (NULL != du_obj)
        {
            if (strlen(du_obj->valuestring) < sizeof(cb_param.str_arg[i]))
            {
                strncpy(cb_param.str_arg[i], du_obj->valuestring, sizeof(cb_param.str_arg[i]));
            }
        }
        cJSON * ol_obj = cJSON_GetObjectItem(dev_obj, "online");
        if (NULL != ol_obj)
        {
            int is_online = 0;
            if (0 == strcmp(ol_obj->valuestring, "yes"))
            {
                is_online = 1;
            }
            cb_param.int_arg1[i] = is_online;
        }
        cJSON * sw_obj = cJSON_GetObjectItem(dev_obj, "switch");
        if (NULL != sw_obj)
        {
            int on_off = 0;
            if (0 == strcmp(sw_obj->valuestring, "on"))
            {
                on_off = 1;
            }
            cb_param.int_arg2[i] = on_off;
        }
    }

    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;
}



#if 0
{
"method":"up_msg",
"cc_uuid":"01001122334455",
"req_id":10000,
"attr":
{
"cmd":"fw_request",
"version":"v1.0.0"
}
}
#endif
static int dev_up_msg_fw_request(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    MSG_CB_PARAM  cb_param;
    JSON_MSG_HDL_OBJECT * handle = (JSON_MSG_HDL_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }

    memset(&cb_param, 0, sizeof(MSG_CB_PARAM));
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
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.cc_uuid))
        {
            strncpy(cb_param.cc_uuid, sub_obj->valuestring, sizeof(cb_param.cc_uuid));
        }
    }
    sub_obj = cJSON_GetObjectItem(root, "req_id");
    if (NULL != sub_obj)
    {
        cb_param.req_id = sub_obj->valueint;
    }

    cJSON * attr_obj = cJSON_GetObjectItem(root, "attr");
    if (NULL == attr_obj)
    {
        debug_error("can't find attr \n");
        return -1;
    }

    cJSON * cmd_obj = cJSON_GetObjectItem(attr_obj, "cmd");
    if (NULL != cmd_obj)
    {
        if (0 == strcmp(cmd_obj->valuestring, "fw_request"))
        {
            cb_param.e_msg = E_DEV_FW_REQUEST;
        }
    }
    cJSON * ver_obj = cJSON_GetObjectItem(attr_obj, "version");
    if (NULL != ver_obj)
    {
        if (strlen(ver_obj->valuestring) < sizeof(cb_param.str_arg))
        {
            strncpy(cb_param.str_arg[0], ver_obj->valuestring, sizeof(cb_param.str_arg[0]));
        }
    }

    cb_param.req_id = sub_obj->valueint;
    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;
}
