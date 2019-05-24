#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <json/cjson.h>
#include <core/core.h>

#include "json_msg_clt.h"

typedef int (*exec_func)(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);

typedef struct _JSON_MSG_CLT_OBJECT
{
    json_msg_cb cb;
    void * arg;
} JSON_MSG_CLT_OBJECT;

typedef struct _JSON_CLT_PARAM
{
    char * method;
    char * cmd;
    exec_func exec_fn;
    int  send_resp;
} JSON_CLT_PARAM;

static int clt_up_msg_get_bind(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int clt_up_msg_bind(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int clt_up_msg_unbind(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int clt_up_msg_get_param(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int clt_up_msg_set_switch(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);
static int clt_up_msg_heart_beat(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg);

static JSON_CLT_PARAM json_handle_tbl[] =
{
    {"up_msg",      "get_bind",         clt_up_msg_get_bind,          1},  /* 获取绑定结果 */
    {"up_msg",      "bind",             clt_up_msg_bind,              1},  /* 绑定 */
    {"up_msg",      "unbind",           clt_up_msg_unbind,            1},  /* 解除绑定 */
    {"up_msg",      "get_param",        clt_up_msg_get_param,         1},  /* 获取设备属性 */
    {"up_msg",      "set_switch",       clt_up_msg_set_switch,        1},  /* 设置插座 */
    {"up_msg",      "heart_beat",       clt_up_msg_heart_beat,        1},  /* 心跳 */
};

JMC_HANDLE json_msg_clt_create(void)
{
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)calloc(1, sizeof(JSON_MSG_CLT_OBJECT));
    if (NULL == handle)
    {
        debug_error("malloc failed \n");
        return NULL;
    }

    return handle;
}

/* 调用者可以使用 strlen(resp_buf)的长度判断是否需要发送 */
int json_msg_clt_msg(JMC_HANDLE handle, char * buffer, int len, char * resp_buf, int resp_buf_len, void * ext_arg)
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

    //  {"method": "up_msg",    "open_id": "ogDt75W7bJt-DTubZvPFrQCZ8Y58",    "req_id": 123456789,    "ts": 12345678,    "attr":    {        "cmd": "get_param"    }    }
    method_obj = cJSON_GetObjectItem(root, "method");
    attr_obj   = cJSON_GetObjectItem(root, "attr");
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

void json_msg_clt_set_cb(JMC_HANDLE handle, json_msg_cb cb, void * arg)
{
    if (NULL == handle)
    {
        return;
    }
    handle->cb  = cb;
    handle->arg = arg;
}

void json_msg_clt_destroy(JMC_HANDLE handle)
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
"open_id":"XXXXXX",
"req_id":123456789,
"ts":12345678
"attr":
{
"cmd":"get_bind"
}
}
#endif
static int clt_up_msg_get_bind(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    CLT_MSG_CB_PARAM  cb_param;
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }
    memset(&cb_param, 0, sizeof(CLT_MSG_CB_PARAM));
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
    sub_obj = cJSON_GetObjectItem(root, "open_id");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.gopenid))
        {
            strncpy(cb_param.gopenid, sub_obj->valuestring, sizeof(cb_param.gopenid));
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
        if (0 == strcmp(cmd_obj->valuestring, "get_bind"))
        {
            cb_param.e_msg = E_DEV_GET_BIND;
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

    return 0;
}

#if 0
{
"method":"up_msg",
"open_id":"XXXXXX",
"req_id":123456789,
"ts":12345678
"attr":
{
"cmd":"bind",
"dev_uuid":"02001122334455" /* 中控设备的uuid */
}
}
#endif
static int clt_up_msg_bind(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    CLT_MSG_CB_PARAM  cb_param;
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }
    memset(&cb_param, 0, sizeof(CLT_MSG_CB_PARAM));
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
    sub_obj = cJSON_GetObjectItem(root, "open_id");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.gopenid))
        {
            strncpy(cb_param.gopenid, sub_obj->valuestring, sizeof(cb_param.gopenid));
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
        if (0 == strcmp(cmd_obj->valuestring, "bind"))
        {
            cb_param.e_msg = E_DEV_BIND;
        }
    }

    cJSON * dev_uuid_obj = cJSON_GetObjectItem(attr_obj, "dev_uuid");
    if (NULL != dev_uuid_obj)
    {
        snprintf(cb_param.str_arg[0], sizeof(cb_param.str_arg[0]), "%s", dev_uuid_obj->valuestring);
    }

    cb_param.req_id = sub_obj->valueint;
    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;

    return 0;
}


#if 0
{
"method":"up_msg",
"open_id":"XXXXXX",
"req_id":123456789,
"ts":12345678
"attr":
{
"cmd":"unbind",
"dev_uuid":"02001122334455" /* 中控设备的uuid */
}
}
#endif
static int clt_up_msg_unbind(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    CLT_MSG_CB_PARAM  cb_param;
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }
    memset(&cb_param, 0, sizeof(CLT_MSG_CB_PARAM));
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
    sub_obj = cJSON_GetObjectItem(root, "open_id");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.gopenid))
        {
            strncpy(cb_param.gopenid, sub_obj->valuestring, sizeof(cb_param.gopenid));
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
        if (0 == strcmp(cmd_obj->valuestring, "unbind"))
        {
            cb_param.e_msg = E_DEV_UNBIND;
        }
    }

    cJSON * dev_uuid_obj = cJSON_GetObjectItem(attr_obj, "dev_uuid");
    if (NULL != dev_uuid_obj)
    {
        snprintf(cb_param.str_arg[0], sizeof(cb_param.str_arg[0]), "%s", dev_uuid_obj->valuestring);
    }

    cb_param.req_id = sub_obj->valueint;
    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;

    return 0;
}

#if 0
{
"method":"up_msg",
"open_id":"XXXXXX",
"req_id":123456789,
"ts":12345678
"attr":
{
    "cmd":"get_param"
}
}
#endif
static int clt_up_msg_get_param(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    CLT_MSG_CB_PARAM  cb_param;
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }
    memset(&cb_param, 0, sizeof(CLT_MSG_CB_PARAM));
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
    sub_obj = cJSON_GetObjectItem(root, "open_id");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.gopenid))
        {
            strncpy(cb_param.gopenid, sub_obj->valuestring, sizeof(cb_param.gopenid));
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
        if (0 == strcmp(cmd_obj->valuestring, "get_param"))
        {
            cb_param.e_msg = E_DEV_GET_PARAM;
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
"open_id":"XXXXXX",
"dev_uuid":"02001122334455",
"req_id":123456789,
"attr":
{
"cmd":"set_switch",
"dev_uuid":"02001122334455", /* 为子设备的uuid */
"switch":"on"
}
}
#endif
static int clt_up_msg_set_switch(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    CLT_MSG_CB_PARAM  cb_param;
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }
    memset(&cb_param, 0, sizeof(CLT_MSG_CB_PARAM));
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
    sub_obj = cJSON_GetObjectItem(root, "open_id");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.gopenid))
        {
            strncpy(cb_param.gopenid, sub_obj->valuestring, sizeof(cb_param.gopenid));
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
        if (0 == strcmp(cmd_obj->valuestring, "set_switch"))
        {
            cb_param.e_msg = E_DEV_SET_SWITCH;
        }
    }


    cJSON * dev_uuid_obj = cJSON_GetObjectItem(attr_obj, "dev_uuid");
    if (NULL != dev_uuid_obj)
    {
        snprintf(cb_param.str_arg[0], sizeof(cb_param.str_arg[0]), "%s", dev_uuid_obj->valuestring);
    }

    cJSON * switch_obj = cJSON_GetObjectItem(attr_obj, "switch");
    if (NULL != switch_obj)
    {
        snprintf(cb_param.str_arg[1], sizeof(cb_param.str_arg[1]), "%s", switch_obj->valuestring);
    }

    cb_param.req_id = sub_obj->valueint;
    if (NULL != handle->cb)
    {
        handle->cb(handle->arg, &cb_param, ext_arg);
        ret = 0;
    }
    cJSON_Delete(root);

    return ret;
    return 0;
}

#if 0
{
"method":"up_msg",
"open_id":"XXXXXX",
"req_id":123456789,
"attr":
{
"cmd":"heart_beat"
}
}
#endif

static int clt_up_msg_heart_beat(void * arg, char *buffer, char *resp_buf, int buf_len, void * ext_arg)
{
    int ret = -1;
    cJSON *root = NULL;
    cJSON *sub_obj = NULL;
    CLT_MSG_CB_PARAM  cb_param;
    JSON_MSG_CLT_OBJECT * handle = (JSON_MSG_CLT_OBJECT *)arg;
    if (NULL == handle)
    {
        return -1;
    }
    memset(&cb_param, 0, sizeof(CLT_MSG_CB_PARAM));
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
    sub_obj = cJSON_GetObjectItem(root, "open_id");
    if (NULL != sub_obj)
    {
        if (strlen(sub_obj->valuestring) < sizeof(cb_param.gopenid))
        {
            strncpy(cb_param.gopenid, sub_obj->valuestring, sizeof(cb_param.gopenid));
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

