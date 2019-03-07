#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libwebsockets.h>

#include "log/debug_util.h"
#include "log/msg_util.h"

#include "core/core.h"
#include "db/id_mgr.h"
#include "device_manage/dev_mgr.h"
#include "db/dev_db_mgr.h"

int main1(void);

int main(int argc, char** argv)
{
    debug_init(IOT_MODULE_IOT_SERVER, IOT_DEBUG_LEVEL_INFO, (char *)"iot_server");

    main1();

    //id_mgr_unit_test();
    //dev_db_mgr_unit_test();
    //dev_mgr_unit_test();

    return 0;
}


// 协议回调函数之 callback_http
static int callback_http(struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{
    return 0;
}

static int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in)   //此函数从别处借来，再此非常感谢
{
    if (str == NULL || wsi_in == NULL)
    {
        return -1;
    }

    int n;
    int len;
    unsigned char *out = NULL;

    if (str_size_in < 1)
        len = strlen(str);
    else
        len = str_size_in;

    out = (unsigned char *)malloc(sizeof(unsigned char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
    //* setup the buffer*/
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );  //要发送的数据从此处拷贝
    //* write out*/
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);  //lws的发送函数

    //  printf("[websocket_write_back] %s\n", str);
    //* free the buffer*/
    free(out);

    return n;
}

// callback_dumb_increment
static int callback_dumb_increment(struct lws *wsi,
                                   enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len)
{
    switch (reason)
    {
        case LWS_CALLBACK_ESTABLISHED:
            printf("connection established\n");
            break;
        case LWS_CALLBACK_RECEIVE:
        {
            printf("received data: [%s] len %d \n", (char *)in, (int)len);
            lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY, (unsigned char *)"seeya", 5);
        }
        break;

        case LWS_CALLBACK_SERVER_WRITEABLE://此处会调用的前提是：lws_callback_on_writable_all_protocol(context,&protocols[1]);被循环调用，如本程序的最后面
        {
            printf("writeable+45\n");
            websocket_write_back(wsi ,"shfjksahfhie",12);
            sleep(1);
        }
        break;
        default:
            break;
    }

    return 0;
}

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


int main1(void)
{
    int port = 7681; // 端口号
    struct lws_context_creation_info info; // 上下文对象的信息
    struct lws_context *context;           // 上下文对象指针
    int opts = 0;                          // 本软件的额外功能选项

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

    context = lws_create_context(&info);// 创建上下文对面，管理ws
    if (context == NULL)
    {
        printf(" Websocket context create error.\n");
        return -1;
    }

    printf("starting server with thread: %d...\n", lws_get_count_threads(context));

    while (1)
    {
        lws_callback_on_writable_all_protocol(context,&protocols[1]); // 激活 case LWS_CALLBACK_SERVER_WRITEABLE 分支,没有此句，则不能往外发送数据，只有写了此句，才能往客户端发送数据。
        lws_service(context, 50);                                     // 启动ws服务
    }

    usleep(10);
    lws_context_destroy(context);

    return 0;
}


