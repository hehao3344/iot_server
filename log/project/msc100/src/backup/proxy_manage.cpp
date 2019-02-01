#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include "protocol/protocol.h"
#include "db/group_manage.h"

#include "msg_handle/msg_define.h"
#include "msg_handle/msg_handle.h"

#include "hash_value.h"
#include "proxy_manage.h"

/* 一共100个线程 每个线程管理1000个连接 */
#define DEV_TIMEOUT_SEC     120
#define MAX_THREAD_COUNT    100
#define MAX_MSG_LEN         1024
#define MAX_EVENTS          256  /* 实际作用不大 */

typedef struct _DEVICE_CONN_PARAM
{
    struct list_head list;
    unsigned int next_sec;
    int  sock_fd;
} DEVICE_CONN_PARAM;

typedef struct _PROXY_MGR_OBJECT
{
    struct  list_head head_dev;

    RTHREAD_HANDLE rthread_dev_center;
    TTASK_HANDLE   ttask_dev_center[MAX_THREAD_COUNT];
    DISPATCH_PARAM dis_param[MAX_THREAD_COUNT];

    pthread_mutex_t   mutex;

    int epoll_fd;

} PROXY_MGR_OBJECT;

typedef struct _DISPATCH_PARAM
{
    int index;

    struct list_head head;
    PROXY_MGR_OBJECT handle;
} DISPATCH_PARAM;

static int dev_thread_center(void *param);

PROXY_MANAGE_HANDLE proxy_mgr_create(void)
{
    int i;
    char rthread_name[32];
    PROXY_MGR_OBJECT *handle = (PROXY_MGR_OBJECT *)calloc(1, sizeof(PROXY_MGR_OBJECT));
	if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->max_thread_count = MAX_THREAD_COUNT;

    // device connection.
    INIT_LIST_HEAD(&handle->head_dev);
    pthread_mutex_init(&handle->mutex, NULL);

    // handle request count: 50 * 100 = 5000.
    handle->rthread_dev_center = rthread_create();
    if (NULL == handle->rthread_dev_center)
    {
        debug_error("rthread_create failed \n");
        goto create_failed;
    }
    handle->epoll_fd  = epoll_create(MAX_EVENTS);
    if (handle->epoll_fd < 0)
    {
        perror("epoll_create");
        goto create_failed;
    }

    DISPATCH_PARAM * dispatch_param = NULL;
    for (i=0; i<ARRAY_SIZE(handle->ttask_dev_center); i++)
    {
        // 0-99, 100-199, 200-299 etc, total 5000.
        dispatch_param = (DISPATCH_PARAM *)malloc(sizeof(DISPATCH_PARAM));
        if (NULL != dispatch_param)
        {
            dispatch_param->index = i;
            INIT_LIST_HEAD(&dispatch_param->head);
            list_add_tail(&dispatch_param->head, &handle->head_dev);
            dispatch_param->proxy_obj = handle;

            memset(rthread_name, 0, sizeof(rthread_name));
            snprintf(rthread_name, "dis_center_%d", i);

            handle->dis_param[i] = dispatch_param;
            handle->ttask_dev_center[i] = rthread_add(handle->rthread_dev_center,
                                                      rthread_name,
                                                      dev_thread_center,
                                                      (long)thread_pool_param);
            if (NULL == handle->ttask_dev_center[i])
            {
                debug_error("rthread_add failed \n");
                goto create_failed;
            }
        }
    }

    return handle;

create_failed:
    proxy_mgr_destroy(handle);
    return NULL;
}

int proxy_mgr_dev_add(PROXY_MANAGE_HANDLE handle, int sock_fd)
{
    DEVICE_CONN_PARAM* list_node = NULL;

    list_node = (DEVICE_CONN_PARAM *)calloc(1, sizeof(DEVICE_CONN_PARAM));
    if (NULL == list_node)
    {
        debug_print("not enough memory \n");
        return -1;
    }

    os_mutex_lock(&handle->mutex);
    list_node->sock_fd = sock_fd;
    list_node->next_sec = get_real_time_sec() + DEV_TIMEOUT_SEC;
    list_add_tail(&list_node->list, &handle->head_dev);
    os_mutex_unlock(&handle->mutex);

    return 0;
}

void proxy_mgr_flush(PROXY_MANAGE_HANDLE handle)
{
    //visit_mgr_flush(handle->hvisit_mgr);
    //device_mgr_flush(handle->hdev_mgr);
}

void proxy_mgr_destroy(PROXY_MANAGE_HANDLE handle)
{
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return;
    }

    int i;
    DISPATCH_PARAM* list_node = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    for (i=0; i<ARRAY_SIZE(handle->ttask_dev_center); i++)
    {
        if (NULL != handle->rthread_dev_center)
        {
            rthread_delete(handle->rthread_dev_center, handle->ttask_dev_center[i]);
            rthread_wait_exit(handle->rthread_dev_center, handle->ttask_dev_center[i]);
            rthread_free(handle->rthread_dev_center, handle->ttask_dev_center[i]);
            handle->ttask_dev_center[i] = NULL;

            list_del(&handle->dis_param[i].head);
        }
    }

    // device.
    list_for_each_safe(pos, n, &handle->head_dev)
    {
        list_node = list_entry(pos, DEVICE_CONN_PARAM, list);
        if (NULL != list_node)
        {
            pthread_mutex_lock(&handle->mutex);
            tcp_close(list_node->sock_fd);
            list_del(&list_node->list);
            free(list_node);
            pthread_mutex_unlock(&handle->mutex);
        }
    }
    list_del(&handle->head_dev);
    pthread_mutex_destroy(&handle->mutex);

    if (NULL != handle->epoll_fd)
    {
        close(handle->epoll_fd);
    }

    free(handle);
}

static void dev_thread_center(long param)
{
    DISPATCH_PARAM *arg = (DISPATCH_PARAM *)param;
    if (NULL == arg)
    {
        return;
    }

    PROXY_MGR_OBJECT * handle = arg->proxy_obj;
    int  index                = arg->index;









    char   id[24];
    unsigned int i, array_size;
    int  sock_fd[10];
    unsigned int hash_value[10];

    char   *buffer   = (char *)malloc(MAX_MSG_LEN);
    char   *msg_body = (char *)malloc(MAX_MSG_LEN);
    int  recv_len = 0;
    uint16 msg, out_len;

    DEVICE_CONN_PARAM* list_node = NULL;

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    int count = 0;
    int cur_start_index = (100 * (index));

    // debug_print("dev thread %d created \n", index);
    while(1)
    {
        count = 0;
        list_for_each_safe(pos, n, &handle->head_dev)
        {
            list_node = list_entry(pos, DEVICE_CONN_PARAM, list);
            if (NULL != list_node)
            {
                // 0-99 100-199 etc.
                if ((count >= cur_start_index) && (count < (cur_start_index + 100)))
                {
                    recv_len = tcp_recv_timeout(list_node->sock_fd, buffer, MAX_MSG_LEN, 10);
                    if (recv_len > 0)
                    {
                        out_len = MAX_MSG_LEN;
                        memset(msg_body, 0, MAX_MSG_LEN);
                        memset(id, 0, sizeof(id));

                        // printf("==== device recved msg ... %d \n", recv_len);
                        if (msg_handle_unpacket(msg_body, &out_len, id, &msg, buffer, recv_len))
                        {
                            if (MSG_A_REGISTER == msg)
                            {
                                char *json_msg = NULL;
                                char   ip[16];
                                uint16 port;
                                char   hash_buf[24];
                                unsigned int hash_value = 0;

                                memset(ip, 0, sizeof(ip));
                                if (tcp_get_peer_addr(list_node->sock_fd, ip, &port))
                                {
                                    memset(hash_buf, 0, sizeof(hash_buf));
                                    sprintf(hash_buf, "%s%d", ip, port);
                                    hash_value = string_to_hash(hash_buf); // convert to hash value for speed up.

                                    if (device_mgr_add(handle->hdev_mgr, id, list_node->sock_fd, hash_value))
                                    {
                                        json_msg = (char*)"{\n\"id\":\"success\"\n}\n";
                                        debug_print("device: [%s] register.\n", id);
                                    }
                                    else
                                    {
                                        json_msg = (char*)"{\n\"id\":\"unexist\"\n}\n";
                                        debug_print("id:[%s] unexist \n", id);
                                    }
                                }
                                else
                                {
                                    json_msg = (char*)"{\n\"addr\":\"error\"\n}\n";
                                    debug_print("id:[%s] socket failed \n", id);
                                }

                                // send MSG_R_REGISTER herel.
                                out_len = MAX_MSG_LEN;
                                if (msg_handle_packet(buffer, &out_len, id, MSG_R_REGISTER, json_msg, strlen(json_msg)))
                                {
                                    tcp_send(list_node->sock_fd, buffer, out_len);
                                }
                                list_node->next_sec = get_real_time_sec() + 180; // we delay the timeout when we recv msg.
                            }
                            else if (MSG_A_KEEP_ALIVE == msg)
                            {
                                // printf("%s keep alive \n", id);
                                device_mgr_keep_alive(handle->hdev_mgr, id);
                                list_node->next_sec = get_real_time_sec() + 180; // we delay the timeout when we recv msg.
                            }
                            else if (MSG_A_PROXY == msg)
                            {
                                array_size = (sizeof(sock_fd) / sizeof(sock_fd[0]));
                                if (visit_mgr_get_sock_fd(handle->hvisit_mgr, id, &array_size, sock_fd, hash_value))
                                {

                                    uint16 in_buf_len = out_len;
                                    uint16 out_len = MAX_MSG_LEN;

                                    if (msg_handle_packet(buffer, &out_len, id, MSG_A_PROXY, msg_body, in_buf_len))
                                    {
                                        debug_print("proxy [device to client]msg total %d ! \n", array_size);
                                        for (i=0; i<array_size; i++)
                                        {
                                            debug_print("send proxy: %s \n", msg_body);
                                            if (0 != tcp_send(sock_fd[i], buffer, out_len))
                                            {
                                                visit_mgr_keep_alive(handle->hvisit_mgr, id, hash_value[i]);
                                            }
                                            else
                                            {
                                                visit_mgr_remove(handle->hvisit_mgr, id, hash_value[i]);
                                            }
                                        }
                                    }
                                }

                                list_node->next_sec = get_real_time_sec() + 180; // we delay the timeout when we recv msg.
                            }
                        }
                    }
                    else if (0 == recv_len) // peer closed.
                    {
                        debug_print("peer closed ? \n");
                        os_mutex_lock(&handle->mutex);
                        tcp_close(list_node->sock_fd);
                        list_del(&list_node->list);
                        free(list_node);
                        os_mutex_unlock(&handle->mutex);
                    }
                    else
                    {
                        // time out, may be block.
                        if (get_real_time_sec() >= (long long)list_node->next_sec)
                        {
                            os_mutex_lock(&handle->mutex);
                            tcp_close(list_node->sock_fd);
                            list_del(&list_node->list);
                            free(list_node);
                            os_mutex_unlock(&handle->mutex);
                            debug_print("[device] time out \n");
                        }
                    }
                }
                count++;
            }
        }
        os_sleep_ms(10);
    }

    if (NULL != buffer)
    {
        free(buffer);
    }
    if (NULL != msg_body)
    {
        free(msg_body);
    }
    free(arg);

    return 0;
}
