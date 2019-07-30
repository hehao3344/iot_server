#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <core/core.h>

#include "msg_handle/json_msg_clt.h"
#include "msg_handle/json_format.h"
#include "dev_param.h"
#include "clt_param.h"
#include "hash_value.h"
#include "ws.h"
#include "clt_mgr.h"

#define MAX_MSG_LEN     1024
#define MAX_EVENTS      128 // 256     /* 实际作用不大 */
#define MAXSOCKET       MAX_EVENTS

#define WSS_RESP "HTTP/1.1 101 Switching Protocols\r\n\
Upgrade: websocket\r\n\
Sec-WebSocket-Version: 13\r\n\
Connection: Upgrade\r\n\
Server: workerman/3.3.6\r\n\
Sec-WebSocket-Accept: "

typedef struct _CLT_MGR_OBJECT
{
    int sock_fd;
    int epoll_fd;

    CLT_PARAM_HANDLE hclt_param;

    JMC_HANDLE       h_jmc;
    WS_HANDLE        ws_handle;

    RTHREAD_HANDLE   rthread_center;
    TTASK_HANDLE     ttask_center;

    RTHREAD_HANDLE   rthread_flush_center;
    TTASK_HANDLE     ttask_flush_center;

    DEV_PARAM_HANDLE hdev_param;
} CLT_MGR_OBJECT;

static CLT_MGR_OBJECT * instance(void);
static void clt_thread_center(long user_info);
static void clt_flush_center(long user_info);
static void sock_exit_fn(void * arg, int sock_fd);
static int json_msg_fn(void * arg, CLT_MSG_CB_PARAM * cb_param, void * ext_arg);

CLT_MGR_HANDLE clt_mgr_create(DEV_PARAM_HANDLE hdev_param)
{
    CLT_MGR_OBJECT * handle = instance();
    if ((NULL == handle) || (NULL == hdev_param))
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->hdev_param = hdev_param;
    handle->ws_handle = ws_create();
    if (NULL == handle->ws_handle)
    {
        debug_error("ws_create create failed \n");
        goto create_failed;
    }
    // for client communication.
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

    handle->hclt_param = clt_param_create(MAX_APP_CLIENTS);
    if (NULL == handle->hclt_param)
    {
        debug_error("not enough memory \n");
        goto create_failed;
    }
    clt_param_set_sock_exit_cb(handle->hclt_param, sock_exit_fn, handle);

    handle->h_jmc = json_msg_clt_create();
    if (NULL == handle->h_jmc)
    {
        debug_error("not enough memory \n");
        goto create_failed;
    }
    json_msg_clt_set_cb(handle->h_jmc, json_msg_fn, handle);

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
    if (NULL != handle->ws_handle)
    {
        ws_destroy(handle->ws_handle);
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

    if (NULL != handle->hclt_param)
    {
        clt_param_destroy(handle->hclt_param);
    }
    if (NULL != handle->h_jmc)
    {
        json_msg_clt_destroy(handle->h_jmc);
    }

    free(handle);
}

static void clt_thread_center(long param)
{
    char peer_ip[16];
    char resp_buf[1024];
    char send_buf[1024];

    unsigned short peer_port;
    char   recv_buf[MAX_MSG_LEN];
    char   dec_buf[MAX_MSG_LEN];

    CLT_MGR_OBJECT * handle = (CLT_MGR_OBJECT *)param;
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
                    //debug_info("new app msg len %d \n", recv_len);
                    int sockfd_exist = clt_param_sock_fd_is_exist(handle->hclt_param, events[i].data.fd);
                    if (0 == sockfd_exist)
                    {
                        char * accept_key = ws_calculate_accept_key(handle->ws_handle, recv_buf);
                        if (NULL != accept_key)
                        {
                            debug_info("get accept_key [%s] \n", accept_key);
                            memset(send_buf, 0, sizeof(send_buf));
                            strcat(send_buf, WSS_RESP);
                            strcat(send_buf, accept_key);
                            strcat(send_buf, "\r\n\r\n");
                            tcp_send(events[i].data.fd, send_buf, strlen(send_buf));
                            debug_info("new client in, send response: %s \n", send_buf);
                            clt_param_add_connect_sock(handle->hclt_param, events[i].data.fd);
                        }
                    }
                    else
                    {
                        int ret = ws_decode_data(handle->ws_handle,  (unsigned char *)recv_buf, recv_len, (unsigned char *)dec_buf, sizeof(dec_buf));
                        if (WDT_ERR != ret)
                        {
                            debug_info("get msg %s \n", dec_buf);
                            if (NULL != strstr(dec_buf, "{"))
                            {
                                json_msg_clt_msg(handle->h_jmc, dec_buf, strlen(dec_buf), resp_buf, sizeof(resp_buf), &(events[i].data.fd));
                            }
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
        clt_param_sock_fd_flush(handle->hclt_param);
        clt_param_flush(handle->hclt_param);
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

static int json_msg_fn(void * arg, CLT_MSG_CB_PARAM * cb_param, void * ext_arg)
{
    CLT_MGR_OBJECT *handle = (CLT_MGR_OBJECT *)arg;
    unsigned long len = 0;
    UNUSED_VALUE(handle);
    debug_info("json callback called cmd 0x%x app_id %s\n", cb_param->e_msg, cb_param->gopenid);
    switch(cb_param->e_msg)
    {
        case E_DEV_GET_DEV_INFO:
        {
            int sock_fd = *(int *)ext_arg;
            char dev_uuid[20] = {0};
            char product_key[32] = {0};
            char dev_secret[64] = {0};
            char to_app_buf[512] = {0};
            int  success_flags = 0;
            if ((0 == clt_param_get_dev_uuid_by_openid(handle->hclt_param, cb_param->gopenid, dev_uuid, sizeof(dev_uuid))) &&
                (0 == clt_param_get_product_key_by_openid(handle->hclt_param, cb_param->gopenid, product_key, sizeof(product_key))) &&
                (0 == clt_param_get_dev_secret_by_openid(handle->hclt_param, cb_param->gopenid, dev_secret, sizeof(dev_secret))))
            {

                debug_info("get dev_info: %s %s %s \n", dev_uuid, product_key, dev_secret);

                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_GET_DEV_INFO_RESP, dev_uuid, product_key, dev_secret);

                success_flags = 1;
                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                debug_info("start send msg: %s len %ld sockfd %d \n", to_app_buf, len, sock_fd);

                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                {
                    tcp_send(sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", sock_fd);
                }
            }

            if (0 == success_flags)
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_GET_DEV_INFO_RESP, "unknown", "unknown", "unknown");
                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                debug_info("start send len %ld sockfd %d msg %s \n", len, sock_fd, to_app_buf);
                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                {
                    tcp_send(sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", sock_fd);
                }
            }

            break;
        }
        case E_DEV_BIND:
        {
            int sock_fd = *(int *)ext_arg;
            char to_app_buf[256] = {0};
            int success_flags = 0;
            int ret = -1;
            debug_info("binding dev:%s openid:%s \n", cb_param->str_arg[0], cb_param->gopenid);
            ret = clt_param_bind(handle->hclt_param, cb_param->str_arg[0], cb_param->gopenid);
            if (0 == ret)
            {
                debug_info("bind successful \n");
                success_flags = 1;
            }

            if (1 == success_flags)
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_BIND_RESP, cb_param->gopenid, cb_param->req_id, 0);
            }
            else
            {
                // 0成功 1该设备已经被绑定 2无该设备
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_BIND_RESP, cb_param->gopenid, cb_param->req_id, ret);
                debug_info("send to app buf %s \n", to_app_buf);
            }
            char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
            debug_info("start send len %ld sockfd %d msg %s \n", len, sock_fd, to_app_buf);
            if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
            {
                tcp_send(sock_fd, send_buf, len);
            }
            else
            {
                debug_info("sockfd %d exit already \n", sock_fd);
            }
            break;
        }
        case E_DEV_UNBIND:
        {

            int sock_fd = *(int *)ext_arg;
            char to_app_buf[256];
            int  success_flags = 0;
            int  ret = -1;
            debug_info("unbinding dev:%s openid:%s \n", cb_param->str_arg[0], cb_param->gopenid);

            /* 把dev_uuid相关联的openID全部设置为null */
            ret = clt_param_unbind(handle->hclt_param, cb_param->str_arg[0], cb_param->gopenid);
            if (0 == ret)
            {
                debug_info("unbind successful \n");
                clt_param_remove(handle->hclt_param, cb_param->gopenid);
                success_flags = 1;
            }

            if (1 == success_flags)
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_UNBIND_RESP, cb_param->gopenid, cb_param->req_id, 0);
            }
            else
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_UNBIND_RESP, cb_param->gopenid, cb_param->req_id, ret);
            }
            char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
            debug_info("start send len %ld sockfd %d msg %s \n", len, sock_fd, to_app_buf);
            if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
            {
                tcp_send(sock_fd, send_buf, len);
            }
            else
            {
                debug_info("sockfd %d exit already \n", sock_fd);
            }
            break;
        }
        case E_DEV_GET_BIND:
        {
            int sock_fd = *(int *)ext_arg;
            char cc_uuid[20] = {0}; // MAX_ID_LEN 16
            char to_app_buf[512] = {0};
            int  success_flags = 0;
            if (0 == clt_param_get_dev_uuid_by_openid(handle->hclt_param, cb_param->gopenid, cc_uuid, sizeof(cc_uuid)))
            {
                debug_info("get dev_uuid: %s \n", cc_uuid);
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_GET_BIND_RESP, cc_uuid, cb_param->req_id, 0);
                success_flags = 1;
                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                debug_info("start send msg: %s len %ld sockfd %d \n", to_app_buf, len, sock_fd);

                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                {
                    tcp_send(sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", sock_fd);
                }
            }

            if (0 == success_flags)
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_GET_BIND_RESP, "unknown", cb_param->req_id, -1);
                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                debug_info("start send len %ld sockfd %d \n", len, sock_fd);
                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                {
                    tcp_send(sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", sock_fd);
                }
            }

            break;
        }

        case E_DEV_GET_PARAM:
        {
            int sock_fd = *(int *)ext_arg;
            char cc_uuid[20] = {0}; // MAX_ID_LEN 16
            char to_app_buf[512];
            int  success_flags = 0;
            if (0 == clt_param_get_dev_uuid_by_openid(handle->hclt_param, cb_param->gopenid, cc_uuid, sizeof(cc_uuid)))
            {
                debug_info("get dev_uuid: %s \n", cc_uuid);
                SUB_DEV_NODE sub_node;
                memset(&sub_node, 0, sizeof(SUB_DEV_NODE));
                if (0 == dev_param_get_sub_dev_node(handle->hdev_param, cc_uuid, &sub_node))
                {
                    debug_info("get sub_node %s:%d-%d %s:%d-%d %s:%d-%d %s:%d-%d \n", sub_node.id[0], sub_node.on_off[0], sub_node.on_line[0],
                                sub_node.id[1], sub_node.on_off[1], sub_node.on_line[1],
                                    sub_node.id[2], sub_node.on_off[2], sub_node.on_line[2],
                                        sub_node.id[3], sub_node.on_off[3], sub_node.on_line[3]);
                    success_flags = 1;
                    snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_GET_PARAM_RESP, cc_uuid,  0, 0,
                              sub_node.id[0], (0 == sub_node.on_line[0]) ? "unknown" : (1 ==  sub_node.on_off[0]) ? "on" : "off",
                                 sub_node.id[1], (0 == sub_node.on_line[1]) ? "unknown" : (1 ==  sub_node.on_off[1]) ? "on" : "off",
                                    sub_node.id[2], (0 == sub_node.on_line[2]) ? "unknown" : (1 ==  sub_node.on_off[2]) ? "on" : "off",
                                        sub_node.id[3], (0 == sub_node.on_line[3]) ? "unknown" : (1 ==  sub_node.on_off[3]) ? "on" : "off");

                    char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                    debug_info("start send msg: %s len %ld sockfd %d \n", to_app_buf, len, sock_fd);

                    if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                    {
                        tcp_send(sock_fd, send_buf, len);
                    }
                    else
                    {
                        debug_info("sockfd %d exit already \n", sock_fd);
                    }
                }
            }

            if (0 == success_flags)
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_GET_PARAM_RESP, cc_uuid,
                         0, -1, "unknown", "unknown",  "unknown", "unknown", "unknown", "unknown", "unknown", "unknown");
                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                debug_info("start send len %ld sockfd %d \n", len, sock_fd);

                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                {
                    tcp_send(sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", sock_fd);
                }
            }

            break;
        }
        case E_DEV_HEART_BEAT:
        {
            int sock_fd = *(int *)ext_arg;
            char to_app_buf[256];

            if (0 == clt_param_heart_beat(handle->hclt_param, cb_param->gopenid, sock_fd))
            {
                snprintf(to_app_buf, sizeof(to_app_buf), JSON_IOTS_APP_HEART_BEAT_RESP, cb_param->gopenid, cb_param->req_id);

                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_app_buf, &len);
                debug_info("start send len %ld sockfd %d msg %s \n", len, sock_fd, to_app_buf);
                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, sock_fd))
                {
                    tcp_send(sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", sock_fd);
                }
            }
            else
            {
                debug_info("can't find open id %s \n", cb_param->gopenid);
            }

            break;
        }
        case E_DEV_SET_SWITCH:
        {
            int sock_fd = 0;
            int app_sock_fd = *(int *)ext_arg;
            char cc_uuid[20] = {0}; // MAX_ID_LEN 16
            char to_buf[512] = {0};
            int  success_flags = 0;
            if (0 == clt_param_get_dev_uuid_by_openid(handle->hclt_param, cb_param->gopenid, cc_uuid, sizeof(cc_uuid)))
            {
                debug_info("get dev_uuid: %s sub_dev:%s switch:%s\n", cc_uuid, cb_param->str_arg[0], cb_param->str_arg[1]);

                if (0 == dev_param_get_sock_fd(handle->hdev_param, cc_uuid, &sock_fd))
                {
                    snprintf(to_buf, sizeof(to_buf), JSON_IOTS_CC_SET_SWITCH_REQ, cc_uuid,
                                cb_param->req_id,  12345, cb_param->str_arg[0], cb_param->str_arg[1]);

                    if (tcp_send(sock_fd, to_buf, strlen(to_buf)) > 0)
                    {
                        success_flags = 1;
                    }
                }
            }

            if (0 == success_flags)
            {
                debug_error("success_flags = %d (failed)\n", success_flags);
                memset(to_buf, 0, sizeof(to_buf));
                snprintf(to_buf, sizeof(to_buf), JSON_IOTS_APP_SET_SWITCH_RESP, cb_param->str_arg[0], cb_param->req_id, -1);

                char * send_buf = ws_construct_packet_data(handle->ws_handle, to_buf, &len);
                debug_info("start send len %ld sockfd %d msg %s \n", len, app_sock_fd, to_buf);
                if (1 == clt_param_sock_fd_is_exist(handle->hclt_param, app_sock_fd))
                {
                    tcp_send(app_sock_fd, send_buf, len);
                }
                else
                {
                    debug_info("sockfd %d exit already \n", app_sock_fd);
                }
            }

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
    CLT_MGR_HANDLE handle = clt_mgr_create(NULL);
    if (NULL == handle)
    {
        debug_error("clt_mgr_create failed \n");
        return;
    }
    sleep(100000);
}
