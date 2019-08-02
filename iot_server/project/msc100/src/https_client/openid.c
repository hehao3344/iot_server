#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <core/core.h>

#include "https_client.h"
#include "db/id_mgr.h"
#include "openid.h"

#define WX_GET_OPENID_PORT  443

#define MKIT_APPID         "wx4b02b7856ddaaf30"
#define MKIT_APPSECRET     "3592273b6d8edfc0a6a9c4a66dbd1edf"
#define WX_GET_OPENID_HOST  "api.weixin.qq.com"
#define WX_GET_OPENID_PAGE  "/sns/jscode2session?"
#define WX_GET_OPENID_DATA  "appid=%s&secret=%s&js_code=%s&grant_type=authorization_code"

/* 该部分代码还未加入socket连接监控机制 */

#define MAX_MSG_LEN     1024
#define MAX_EVENTS      128 // 256     /* 实际作用不大 */
#define MAXSOCKET       MAX_EVENTS

typedef struct _OPENID_OBJECT
{
    int sock_fd;
    int epoll_fd;

    RTHREAD_HANDLE   rthread_center;
    TTASK_HANDLE     ttask_center;

    RTHREAD_HANDLE   rthread_flush_center;
    TTASK_HANDLE     ttask_flush_center;

    ID_MGR_HANDLE    hid_mgr;

} OPENID_OBJECT;

static OPENID_OBJECT * instance(void);
static void openid_thread_center(long user_info);
static void openid_flush_center(long user_info);

OPENID_HANDLE openid_create(void)
{
    OPENID_OBJECT * handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    handle->hid_mgr = id_mgr_create();
    if (NULL == handle->hid_mgr)
    {
        debug_error("id_mgr_create failed \n");
        goto create_failed;
    }

    // for client communication.
    handle->sock_fd = tcp_open_and_bind(PORT_OPENID);
    if (handle->sock_fd <= 0)
    {
        debug_error("socket create failed \n");
        goto create_failed;
    }

    tcp_set_nonblock(handle->sock_fd);
    handle->epoll_fd  = epoll_create(MAX_EVENTS);
    if (handle->epoll_fd < 0)
    {
        debug_error("epoll_create");
        goto create_failed;
    }

    if (0 != tcp_listen(handle->sock_fd, 10))
    {
        debug_error("socket listen failed \n");
        goto create_failed;
    }

    handle->rthread_center = rthread_create();
    if (NULL == handle->rthread_center)
    {
        debug_error("rthread_create failed \n");
        goto create_failed;
    }

    handle->ttask_center = rthread_add(handle->rthread_center,
                                       "openid_center",
                                       openid_thread_center,
                                       (long)handle);
    if (NULL == handle->ttask_center)
    {
        debug_error("rthread_add failed \n");
        goto create_failed;
    }

    handle->rthread_flush_center = rthread_create();
    if (NULL == handle->rthread_flush_center)
    {
        debug_error("rthread_flush_center failed \n");
        goto create_failed;
    }

    handle->ttask_flush_center = rthread_add(handle->rthread_flush_center,
                                             "openid_flush_center",
                                             openid_flush_center,
                                            (long)handle);
    if (NULL == handle->ttask_center)
    {
        debug_error("rthread_add failed \n");
        goto create_failed;
    }

    return handle;

create_failed:
    openid_destroy(handle);
    return NULL;
}

void openid_destroy(OPENID_HANDLE handle)
{
    if (NULL == handle)
    {
        return;
    }

    if (handle->epoll_fd > 0)
    {
        close(handle->epoll_fd);
    }
    if (handle->sock_fd > 0)
    {
        tcp_close(handle->sock_fd);
    }
    if ((NULL != handle->rthread_center) && (NULL != handle->ttask_center))
    {
        rthread_delete(handle->rthread_center, handle->ttask_center);
        rthread_wait_exit(handle->rthread_center, handle->ttask_center);
        rthread_free(handle->rthread_center, handle->ttask_center);
        handle->ttask_center = NULL;
    }
    if ((NULL != handle->rthread_flush_center) && (NULL != handle->ttask_flush_center))
    {
        rthread_delete(handle->rthread_flush_center, handle->ttask_flush_center);
        rthread_wait_exit(handle->rthread_flush_center, handle->ttask_flush_center);
        rthread_free(handle->rthread_flush_center, handle->ttask_flush_center);
        handle->ttask_flush_center = NULL;
    }
    id_mgr_destroy(handle->hid_mgr);
    free(handle);
}

static void openid_thread_center(long param)
{
    char peer_ip[16];

    unsigned short peer_port;
    char   recv_buf[MAX_MSG_LEN];

    OPENID_OBJECT * handle = (OPENID_OBJECT *)param;
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return;
    }

    struct epoll_event ev, events[MAX_EVENTS];

    ev.data.fd = handle->sock_fd;
    ev.events = EPOLLIN;
    epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, handle->sock_fd, &ev);

    while(1)
    {
        int nfds = epoll_wait(handle->epoll_fd, events, MAXSOCKET, -1);
        if (nfds == -1)
        {
            printf("epoll_wait failed ! error message :%s \n", strerror(errno));
            break;
        }
        int i = 0;
        for (i=0; i<nfds; i++)
        {
            if (events[i].data.fd < 0)
            {
                debug_error("events[%d].data.fd \n", i);
                continue;
            }
            if (events[i].data.fd == handle->sock_fd)
            {
                // 接收客户端socket
                int new_fd = 0;
                memset(peer_ip, 0, sizeof(peer_ip));

                if ((new_fd = tcp_accept(handle->sock_fd, peer_ip, &peer_port)) < 0)
                {
                    debug_error("accept error \n");
                    continue;
                }
                debug_info("new connect in [%s:%d]\n", peer_ip, peer_port);

                tcp_set_nonblock(new_fd);

                struct epoll_event client_ev;
                client_ev.data.fd = new_fd;
                client_ev.events  = EPOLLIN | EPOLLERR | EPOLLHUP;
                epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, new_fd, &client_ev);

                continue;
            }
            if (events[i].events & EPOLLIN)
            {
                // 表示服务器这边的client_st接收到消息
                memset(recv_buf, 0, sizeof(recv_buf));

                int recv_len = tcp_recv(events[i].data.fd, recv_buf, sizeof(recv_buf));
                if (recv_len <= 0)
                {
                    tcp_close(events[i].data.fd);
                    events[i].data.fd = -1;
                    continue;
                }
                else
                {
                    /* 处理消息 */
                    debug_info("new app msg len %d buffer %s \n", recv_len, recv_buf);
                    char * msg_code = strstr(recv_buf, "&code=");
                    if (NULL != msg_code)
                    {
                        int  read_len = 0;
                        char post_recv_buf[512] = {0};
                        char post_data[128] = {0};
                        char code_data[64] = {0};
                        strncpy(code_data, msg_code+strlen("&code="), sizeof(code_data));
                        snprintf(post_data, sizeof(post_data), WX_GET_OPENID_DATA, MKIT_APPID, MKIT_APPSECRET, code_data);

                        read_len = https_clt_post(WX_GET_OPENID_HOST, WX_GET_OPENID_PORT, WX_GET_OPENID_PAGE, post_data, strlen(post_data), post_recv_buf, sizeof(post_recv_buf));
                        if (read_len < 0)
                        {
                            // debug_info("https post failed = %d \n", read_len);
                            /* epoll 事件监控中删除sock_fd */
                            epoll_ctl(handle->epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                            tcp_close(events[i].data.fd);
                        }
                        else
                        {
                            char open_id[64] = {0};
                            char send_buf[512] = {0};
#define NGINX_HTTP_RESPONSE   "HTTP/1.1 200 OK\n\
Date: Mon, 30 Jul 2019 08:50:55 GMT \n\
Cache-Control:  no-cache  \n\
Connection: keep-alive  \r\n\r\n\
{\"code\":\"%d\",\
\"openid\":\"%s\",\
\"device_name\":\"%s\",\
\"product_key\":\"%s\",\
\"dev_secret\":\"%s\"\
}"
                            // {"session_key":"P8CTtsbaadhyEQRWOvJ4YA==","openid":"ogDt75W7bJt-DTubZvPFrQCZ8Y58"}
                            char * open_id_head = strstr(post_recv_buf, "\"openid\":\"");
                            char * open_id_tail = strstr(post_recv_buf, "\"}");

                            if ((NULL != open_id_head) && (NULL != open_id_tail) && ((open_id_tail - (open_id_head + strlen("\"openid\":\""))) < sizeof(open_id)))
                            {
                                char dev_uuid[20] = {0};
                                char product_key[32] = {0};
                                char dev_secret[64] = {0};

                                open_id_head += strlen("\"openid\":\"");
                                memset(open_id, 0, sizeof(open_id));
                                memcpy(open_id, open_id_head, open_id_tail - open_id_head);

                                if ((0 == id_mgr_get_uuid_by_group_openid(handle->hid_mgr, open_id, dev_uuid, sizeof(dev_uuid))) &&
                                    (0 == id_mgr_get_product_key_by_group_openid(handle->hid_mgr, open_id, product_key, sizeof(product_key))) &&
                                    (0 == id_mgr_get_dev_secret_by_group_openid(handle->hid_mgr, open_id, dev_secret, sizeof(dev_secret))))
                                {
                                    snprintf(send_buf, sizeof(send_buf), NGINX_HTTP_RESPONSE, 0,  open_id, dev_uuid, product_key, dev_secret);
                                }
                                else
                                {
                                    /* 未绑定 */
                                    snprintf(send_buf, sizeof(send_buf), NGINX_HTTP_RESPONSE, -1,  open_id, "null", "null", "null");
                                }
                            }
                            else
                            {
                                /* openid获取失败 */
                                snprintf(send_buf, sizeof(send_buf), NGINX_HTTP_RESPONSE, -2,  "null", "null", "null", "null");
                            }

                            debug_info("get openid = %s send %s \n", open_id, send_buf);

                            tcp_send(events[i].data.fd, send_buf, strlen(send_buf));
                            epoll_ctl(handle->epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                            tcp_close(events[i].data.fd);
                        }
                    }
                }
            }

            //客户端有事件到达
            if (events[i].events & EPOLLERR)
            {
                debug_error("EPOLLERR\n");
                tcp_close(events[i].data.fd);
                events[i].data.fd = -1;
                continue;
            }
            //客户端有事件到达
            if (events[i].events & EPOLLHUP)
            {
                debug_error("EPOLLHUP\n");
                // 返回挂起事件，关闭socket，清理epoll池
                tcp_close(events[i].data.fd);
                events[i].data.fd = -1;
                continue;
            }
        }
    }
}

static void openid_flush_center(long param)
{

    OPENID_OBJECT * handle = (OPENID_OBJECT *)param;
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return;
    }
    while(1)
    {
        sleep(3);
    }
}

/*******************************************************************************
*static function
*******************************************************************************/
static OPENID_OBJECT * instance(void)
{
    static OPENID_OBJECT *handle = NULL;
    if (NULL == handle)
    {
        handle = (OPENID_OBJECT *)malloc(sizeof(OPENID_OBJECT));
        memset(handle, 0, sizeof(OPENID_OBJECT));
    }

    return handle;
}

void openid_unit_test(void)
{
    OPENID_HANDLE handle = openid_create();
    if (NULL == handle)
    {
        debug_error("openid_create failed \n");
        return;
    }
    sleep(100000);
}
