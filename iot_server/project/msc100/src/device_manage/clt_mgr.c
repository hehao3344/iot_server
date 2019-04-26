#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <core/core.h>

#include <crypto/sha1.h>
#include <crypto/base64.h>
#include <crypto/int_lib.h>


#include "msg_handle/json_msg_handle.h"
#include "dev_param.h"
#include "hash_value.h"
#include "clt_mgr.h"

#define MAX_MSG_LEN     1024
#define MAX_EVENTS      256     /* 实际作用不大 */
#define MAXSOCKET       MAX_EVENTS

typedef struct _CLT_MGR_OBJECT
{
    int sock_fd;
    int epoll_fd;

    DEV_PARAM_HANDLE hdev_param;

    JMH_HANDLE       h_jmh;

    RTHREAD_HANDLE   rthread_center;
    TTASK_HANDLE     ttask_center;

    RTHREAD_HANDLE   rthread_flush_center;
    TTASK_HANDLE     ttask_flush_center;

} CLT_MGR_OBJECT;

static CLT_MGR_OBJECT * instance(void);
static void clt_thread_center(long user_info);
static void clt_flush_center(long user_info);
static int json_msg_fn(void * arg, MSG_CB_PARAM * cb_param, void * ext_arg);
static void sock_exit_fn(void * arg, int sock_fd);

CLT_MGR_HANDLE clt_mgr_create(void)
{
    CLT_MGR_OBJECT *handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // for device communication.
    handle->sock_fd = tcp_open_and_bind(PORT_CLT);
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

    handle->hdev_param = dev_param_create(MAX_CLIENTS);
    if (NULL == handle->hdev_param)
    {
        debug_error("not enough memory \n");
        goto create_failed;
    }
    dev_param_set_sock_exit_cb(handle->hdev_param, sock_exit_fn, handle);

    handle->h_jmh = json_msg_handle_create();
    if (NULL == handle->h_jmh)
    {
        debug_error("not enough memory \n");
        goto create_failed;
    }
    json_msg_handle_set_cb(handle->h_jmh, json_msg_fn, handle);

    handle->rthread_center = rthread_create();
    if (NULL == handle->rthread_center)
    {
        debug_error("rthread_create failed \n");
        goto create_failed;
    }

    handle->ttask_center = rthread_add(handle->rthread_center,
                                       "clt_center",
                                       clt_thread_center,
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
                                             "clt_flush_center",
                                             clt_flush_center,
                                            (long)handle);
    if (NULL == handle->ttask_center)
    {
        debug_error("rthread_add failed \n");
        goto create_failed;
    }


    return handle;

create_failed:
    clt_mgr_destroy(handle);
    return NULL;
}

void clt_mgr_destroy(CLT_MGR_HANDLE handle)
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

    if (NULL != handle->hdev_param)
    {
        dev_param_destroy(handle->hdev_param);
    }
    if (NULL != handle->h_jmh)
    {
        json_msg_handle_destroy(handle->h_jmh);
    }

    free(handle);
}

static void clt_thread_center(long param)
{
    char peer_ip[16];
    char resp_buf[256];

    unsigned short peer_port;
    char   recv_buf[MAX_MSG_LEN];

    CLT_MGR_OBJECT * handle = (CLT_MGR_OBJECT *)param;
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return;
    }

    /*
    * 声明epoll_event结构体变量ev，变量ev用于注册事件，
    * 数组events用于回传需要处理的事件
    */
    struct epoll_event ev, events[MAX_EVENTS];
    // 生成用于处理accept的epoll专用文件描述符

    ev.data.fd = handle->sock_fd;
    // 设置这个文件描述符需要epoll监控的事件
    /*
     * EPOLLIN代表文件描述符读事件
     * accept, recv都是读事件
     */
    ev.events = EPOLLIN;
    /*
     * 注册epoll事件
     * 函数epoll_ctl中&ev参数表示需要epoll监视的listen_st这个socket中的一些事件
     */
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

                // 设置客户端socket非阻塞
                tcp_set_nonblock(new_fd);

                dev_param_add_connect_sock(handle->hdev_param, new_fd);

                //将客户端socket加入到epoll池中
                struct epoll_event client_ev;
                client_ev.data.fd = new_fd;
                client_ev.events  = EPOLLIN | EPOLLERR | EPOLLHUP;
                epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, new_fd, &client_ev);
                /*
                 * 注释：当epoll池中new_fd这个服务器socket有消息的时候
                 * 只可能是来自客户端的连接消息
                 * recv, send使用的都是客户端的socket，不会向listen_st发送消息的
                 */
                continue;
            }

            // 客户端有事件到达
            if (events[i].events & EPOLLIN)
            {
                // 表示服务器这边的client_st接收到消息
                memset(recv_buf, 0, sizeof(recv_buf));
                int recv_len = tcp_recv(events[i].data.fd, recv_buf, sizeof(recv_buf));
                if (recv_len <= 0)
                {
                    tcp_close(events[i].data.fd);
                    // 接收数据出错或者客户端已经关闭
                    events[i].data.fd = -1;
                    /*这里continue是因为客户端socket已经被关闭了，
                     * 但是这个socket可能还有其他的事件，会继续执行其他的事件，
                     * 但是这个socket已经被设置成-1
                     * 所以后面的close_socket()函数都会报错
                     */
                    continue;
                }
                else
                {
                    /* 处理消息 */
                    debug_info("recv client msg %s \n", recv_buf);




                }
                /*
                 * 此处不能continue，因为每个socket都可能有多个事件同时发送到服务器端
                 * 这也是下面语句用if而不是if-else的原因，
                 */
            }

            //客户端有事件到达
            if (events[i].events & EPOLLERR)
            {
                debug_error("EPOLLERR\n");
                //返回出错事件，关闭socket，清理epoll池，当关闭socket并且events[i].data.fd=-1, epoll会自动将该socket从池中清除
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

static void clt_flush_center(long param)
{

    CLT_MGR_OBJECT * handle = (CLT_MGR_OBJECT *)param;
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return;
    }
    while(1)
    {
        dev_param_sock_fd_flush(handle->hdev_param);
        dev_param_flush(handle->hdev_param);
        sleep(3);
    }
}

/*******************************************************************************
*static function
*******************************************************************************/
static CLT_MGR_OBJECT * instance(void)
{
    static CLT_MGR_OBJECT *handle = NULL;
    if (NULL == handle)
    {
        handle = (CLT_MGR_OBJECT *)malloc(sizeof(CLT_MGR_OBJECT));
        memset(handle, 0, sizeof(CLT_MGR_OBJECT));
    }

    return handle;
}

static int json_msg_fn(void * arg, MSG_CB_PARAM * cb_param, void * ext_arg)
{
    CLT_MGR_OBJECT *handle = (CLT_MGR_OBJECT *)arg;

    UNUSED_VALUE(handle);
    debug_info("json callback called cmd 0x%x cc_id %s\n", cb_param->e_msg, cb_param->cc_uuid);
    switch(cb_param->e_msg)
    {
        case E_DEV_REGISTER:
        {
            int sock_fd = *(int *)ext_arg;
            int hash_value = (int)string_to_hash(cb_param->cc_uuid);
            dev_param_register(handle->hdev_param, cb_param->cc_uuid, hash_value, sock_fd);
            break;
        }
        case E_DEV_HEART_BEAT:
        {
            dev_heart_beat(handle->hdev_param, cb_param->cc_uuid);
            break;
        }
        case E_DEV_INFORM_STATUS:
        {
            int i;
            SUB_DEV_NODE sub_dev;
            memset(&sub_dev, 0, sizeof(SUB_DEV_NODE));
            for (i=0; i<MAX_ID_ARRAY; i++)
            {
                strncpy(sub_dev.id[i],  cb_param->str_arg[i], sizeof(sub_dev.id[i]));
                sub_dev.on_line[i] = cb_param->int_arg1[i];
                sub_dev.on_off[i]  = cb_param->int_arg2[i];
            }
            dev_param_update(handle->hdev_param, cb_param->cc_uuid, &sub_dev);
            break;
        }
        default:
            break;
    }

    return 0;
}

static void sock_exit_fn(void * arg, int sock_fd)
{
    CLT_MGR_OBJECT *handle = (CLT_MGR_OBJECT *)arg;
    if (NULL != handle)
    {
        /* epoll 事件监控中删除sock_fd */
        epoll_ctl(handle->epoll_fd, EPOLL_CTL_DEL, sock_fd, NULL);
        tcp_close(sock_fd);
    }
}

void clt_mgr_unit_test(void)
{
    CLT_MGR_HANDLE handle = clt_mgr_create();
    if (NULL == handle)
    {
        debug_error("clt_mgr_create failed \n");
        return;
    }
    sleep(100000);
}
