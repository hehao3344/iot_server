#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <core/core.h>
#include <libwebsockets.h>

#include "websockets.h"

#define MAX_SEND_BUF_LEN    (1024)
#define CACHE_BUF_NUM       (100)   /* 最多可以缓存XXX条消息 */

typedef struct _SEND_BUF_NODE
{
    struct list_head list;
    int    token;                       /* 是否被占用 */
    char   buffer[MAX_SEND_BUF_LEN];
} SEND_BUF_NODE;

typedef struct _WEBSOCKETS_OBJECT
{
    struct  list_head head;

    pthread_mutex_t mutex;
    data_receive_cb cb;
    void * arg;
    struct lws_context *context;           // 上下文对象指针

    char recv_data[MAX_SEND_BUF_LEN];
    char send_data[MAX_SEND_BUF_LEN];
    char out_buf[LWS_SEND_BUFFER_PRE_PADDING + MAX_SEND_BUF_LEN + LWS_SEND_BUFFER_POST_PADDING];
} WEBSOCKETS_OBJECT;

static WEBSOCKETS_OBJECT * instance(void);
static int callback_http(struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len);

static int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in);

// callback_dumb_increment
static int callback_dumb_increment(struct lws *wsi,
                                   enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len);

static WEBSOCKETS_OBJECT * instance(void);


//注册协议,一种协议，对应一套处理方案（类似驱动中的设备树）
static struct lws_protocols protocols[] =
{
    {
        "ws-protocol",
        callback_dumb_increment,
        // sizeof(struct per_session_data__dumb_increment), （一帧数据包的大小，注释表示无限制，由实际发送的数据的大小来决定）
        10,
        /* rx buf size must be >= permessage-deflate rx size
         * dumb-increment only sends very small packets, so we set
         * this accordingly.  If your protocol will send bigger
         * things, adjust this to match */
    },

    {
        "http-only", /* name */
        callback_http,/* callback */
        // sizeof (struct per_session_data__http),/* per_session_data_size */
        0, /* max frame size / rx buffer */
    },

    {
        NULL, NULL, 0, 0
    } /* terminator */
};

WEBS_HANDLE websockets_create(int bind_port)
{
    WEBSOCKETS_OBJECT *handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    SEND_BUF_NODE * buf_node  = NULL;

    int port = bind_port; // 端口号
    struct lws_context_creation_info info;  // 上下文对象的信息
    //struct lws_context *context;          // 上下文对象指针
    int opts = 0;                           // 本软件的额外功能选项

    //设置info，填充info信息体
    memset(&info,0,sizeof(info));
    info.port = port;
    info.iface=NULL;
    info.protocols = protocols;
    info.extensions = NULL;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.ssl_ca_filepath = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = opts;
    info.ka_time = 0;
    info.ka_probes = 0;
    info.ka_interval = 0;

    int i;
    INIT_LIST_HEAD(&handle->head);
    for (i=0; i<CACHE_BUF_NUM; i++)
    {
        buf_node = (SEND_BUF_NODE *)calloc(1, sizeof(SEND_BUF_NODE));
        if (NULL != buf_node)
        {
            list_add_tail(&buf_node->list, &handle->head);
        }
    }

    handle->context = lws_create_context(&info);// 创建上下文对面，管理ws
    if (handle->context == NULL)
    {
        printf("websocket context create error.\n");
        return NULL;
    }

    printf("starting server with thread:[%d]... bind port %d \n", lws_get_count_threads(handle->context), bind_port);

    pthread_mutex_init(&handle->mutex, NULL);

    return handle;
}

void websockets_set_recv_callback(data_receive_cb cb, void * arg)
{
    WEBSOCKETS_OBJECT *handle = instance();
    if (NULL != handle)
    {
        handle->cb = cb;
        handle->arg= arg;
    }
}

int websockets_send_data(WEBS_HANDLE handle, char * buffer, int len)
{
    int ret = -1;
    if (NULL == handle)
    {
        printf("invalid param \n");
        return -1;
    }

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    SEND_BUF_NODE * node  = NULL;

    pthread_mutex_lock(&handle->mutex);
    list_for_each_safe(pos, n, &handle->head)
    {
        node = list_entry(pos, SEND_BUF_NODE, list);
        if ((0 == node->token) && (len < sizeof(node->buffer)))
        {
            memset(node->buffer, 0, sizeof(node->buffer));
            memcpy(node->buffer, buffer, len);
            node->token  = 1;
            ret = 0;
            break;
        }
    }
    pthread_mutex_unlock(&handle->mutex);

    return ret;
}

void websockets_destroy(WEBS_HANDLE handle)
{
    if (NULL != handle)
    {
        if (NULL != handle->context)
        {
            lws_context_destroy(handle->context);
        }

        struct list_head *pos = NULL;
        struct list_head *n   = NULL;
        SEND_BUF_NODE * node  = NULL;

        list_for_each_safe(pos, n, &handle->head)
        {
            node = list_entry(pos, SEND_BUF_NODE, list);
            if (NULL != node)
            {
                list_del(&node->list);
                free(node);
            }
        }

        pthread_mutex_destroy(&handle->mutex);
        free(handle);
    }
}

/*******************************************************************************
* static function
*******************************************************************************/
// 协议回调函数之 callback_http
static int callback_http(struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{
    return 0;
}

static int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in)
{
    WEBSOCKETS_OBJECT *handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return -1;
    }

    if ((NULL == str) || (NULL == wsi_in))
    {
        printf("invalid param \n");
        return -1;
    }

    int n;
    int len;
    unsigned char *out = NULL;

    if (str_size_in < 1)
    {
        len = strlen(str);
    }
    else
    {
        len = str_size_in;
    }

    out = (unsigned char *)handle->out_buf;

    /* setup the buffer*/
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );  // 要发送的数据从此处拷贝

    /* write out*/
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);  // lws的发送函数

    return n;
}

// callback_dumb_increment
static int callback_dumb_increment(struct lws *wsi,
                                   enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len)
{
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    SEND_BUF_NODE * node  = NULL;

    WEBSOCKETS_OBJECT *handle = instance();
    if (NULL == handle)
    {
        printf("invalid param \n");
        return -1;
    }
    switch (reason)
    {
        case LWS_CALLBACK_ESTABLISHED:
        {
            printf("connection established\n");
            break;
        }
        case LWS_CALLBACK_RECEIVE:
        {
            printf("received data: [%s] len %d \n", (char *)in, (int)len);
            if (NULL != handle->cb)
            {
                handle->cb(handle->arg, (char *)in, (int)len);
            }
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: // 此处会调用的前提是：lws_callback_on_writable_all_protocol(context,&protocols[1]);被循环调用，如本程序的最后面
        {
            pthread_mutex_lock(&handle->mutex);
            list_for_each_safe(pos, n, &handle->head)
            {
                node = list_entry(pos, SEND_BUF_NODE, list);
                if ((NULL != node) && (1 == node->token))
                {
                    //websocket_write_back(wsi, node->buffer, strlen(node->buffer));
                    memcpy(handle->send_data, node->buffer, sizeof(node->buffer));
                    node->token  = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&handle->mutex);

            websocket_write_back(wsi, handle->send_data, strlen(handle->send_data));

            break;
        }
        case LWS_CALLBACK_CLOSED:
        {
            printf("LWS_CALLBACK_CLOSED peer.\n");
            break;
        }
        default:
            break;
    }

    return 0;
}

static WEBSOCKETS_OBJECT * instance(void)
{
    static WEBSOCKETS_OBJECT * handle = NULL;
    if (NULL == handle)
    {
        handle = (WEBSOCKETS_OBJECT *)calloc(1, sizeof(WEBSOCKETS_OBJECT));
        if (NULL == handle)
        {
            printf("not enough memory \n");
            return NULL;
        }
    }

    return handle;
}

void data_receive_fn(void * arg, char * buffer, int len)
{
    WEBSOCKETS_OBJECT * handle = (WEBSOCKETS_OBJECT *)arg;
    if (sizeof(handle->recv_data) > len)
    {
        memset(handle->recv_data, 0, sizeof(handle->recv_data));
        memcpy(handle->recv_data, buffer, len);
        printf("receive len:%d data:%s \n", len, handle->recv_data);
    }
}

void websockets_unit_test(void)
{
    int index;
    char buffer[64];
    WEBS_HANDLE handle = websockets_create(8020);

    websockets_set_recv_callback(data_receive_fn, handle);

    while (1)
    {
        lws_callback_on_writable_all_protocol(handle->context, &protocols[0]); // 激活 case LWS_CALLBACK_SERVER_WRITEABLE 分支,没有此句，则不能往外发送数据，只有写了此句，才能往客户端发送数据。
        lws_service(handle->context, 50);                                      // 启动ws服务

        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "index = %d ", index++);
        websockets_send_data(handle, buffer, strlen(buffer));
        sleep(1);
    }
}
