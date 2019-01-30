#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <core/core.h>

#include <core/core.h>

#include "device_manage/device_manage.h"
#include "device_manage/proxy_manage.h"
#include "device_manage/distributed_manage.h"

#include "tcp_server.h"

#define MAX_MSG_LEN        1024
#define MAX_EVENTS         256     /* ʵ�����ò��� */

typedef struct _TCP_SERVER_OBJECT
{
    int device_sock_fd;

    DEVICE_MGR_HANDLE         hdev_mgr;
    DISTRIBUTED_MANAGE_HANDLE hdistributed_mgr;

    RTHREAD_HANDLE rthread_center;
    TTASK_HANDLE   ttask_center;

    int epoll_fd;

} TCP_SERVER_OBJECT;

static void * device_accept_center(void * arg);
static void * flush_center(void * arg);
static TCP_SERVER_OBJECT * instance(void);
static void dev_thread_center(long user_info);

TCP_SERVER_HANDLE tcp_server_create(void)
{
    TCP_SERVER_OBJECT *handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // create and bind 3 socket.
    // for device communication.
    handle->device_sock_fd = tcp_open_and_bind(PORT_DEV);
    if (handle->device_sock_fd <= 0)
    {
        debug_error("socket create failed \n");
        return NULL;
    }

    tcp_sock_set_nonblock(handle->device_sock_fd);
    handle->epoll_fd  = epoll_create(MAX_EVENTS);
    if (handle->epoll_fd < 0)
    {
        debug_error("epoll_create");
        return NULL;
    }

#if 0
    if (!tcp_listen(handle->device_sock_fd, 10))
    {
        debug_error("socket listen failed \n");
        return NULL;
    }
#endif

    handle->hdev_mgr = device_mgr_create(MAX_CLIENTS);
    if (NULL == handle->hdev_mgr)
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    DistributedMgrEnv dis_mgr_env;
    dis_mgr_env.hdev_mgr = handle->hdev_mgr;

    // create distributed and proxy.
    handle->hdistributed_mgr = distributed_mgr_create(&dis_mgr_env);
    if (NULL == handle->hdistributed_mgr)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->rthread_center = rthread_create();
    if (NULL == handle->rthread_center)
    {
        debug_error("rthread_create failed \n");
        goto create_failed;
    }

    handle->ttask_center = rthread_add(handle->rthread_center,
                                       "dev_center",
                                       dev_thread_center,
                                       (long)handle);
    if (NULL == handle->ttask_center)
    {
        debug_error("rthread_add failed \n");
        goto create_failed;
    }
    return handle;
}

void tcp_server_destroy(TCP_SERVER_HANDLE handle)
{
    if (NULL == handle)
    {
        return;
    }
    if (handle->epoll_fd > 0)
    {
        close(handle->epoll_fd);
    }
    if (handle->client_sock_fd > 0)
    {
        udp_close(handle->client_sock_fd);
    }
    if (handle->device_sock_fd > 0)
    {
        udp_close(handle->device_sock_fd);
    }
    if (handle->distributed_sock_fd > 0)
    {
        udp_close(handle->distributed_sock_fd);
    }


    os_close_thread(handle->hthread_distributed);
    os_close_thread(handle->hthread_device_msg);
    os_close_thread(handle->hthread_client_msg);
    os_close_thread(handle->hthread_flush);

    distributed_mgr_destroy(handle->hdistributed_mgr);
    proxy_mgr_destroy(handle->hproxy_mgr);
    device_mgr_destroy(handle->hdev_mgr);

    free(handle);
}

static void * distributed_accept_center(void * arg)
{
    int  new_fd;
    char   peer_ip[16];
    unsigned short peer_port;

    /*
    * ����epoll_event�ṹ�����ev������ev����ע���¼���
    * ����events���ڻش���Ҫ������¼�
    */
    struct epoll_event ev, events[MAX_EVENTS];
    // �������ڴ���accept��epollר���ļ�������

    ev.data.fd = handle->distributed_sock_fd;
    //��������ļ���������Ҫepoll��ص��¼�
    /*
     * EPOLLIN�����ļ����������¼�
     *accept, recv���Ƕ��¼�
     */
    ev.events = EPOLLIN;
    /*
     * ע��epoll�¼�
     * ����epoll_ctl��&ev������ʾ��Ҫepoll���ӵ�listen_st���socket�е�һЩ�¼�
     */
    epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, handle->distributed_sock_fd, &ev);

    TCP_SERVER_OBJECT *handle = (TCP_SERVER_OBJECT *)arg;
    while(1)
    {


        int nfds = epoll_wait(epfd, events, MAXSOCKET, -1);
        if (nfds == -1)
        {
            printf("epoll_wait failed ! error message :%s \n", strerror(errno));
            break;
        }
        int i = 0;
        for (; i < nfds; i++)
        {
            if (events[i].data.fd < 0)
                continue;
            if (events[i].data.fd == listen_st)
            {
                //���տͻ���socket
                int client_st = server_accept(listen_st);
                /*
                 * ��⵽һ���û���socket���ӵ�������listen_st�󶨵Ķ˿�
                 *
                 */
                if (client_st < 0)
                {
                    continue;
                }
                //���ÿͻ���socket������
                setnonblock(client_st);
                //���ͻ���socket���뵽epoll����
                struct epoll_event client_ev;
                client_ev.data.fd = client_st;
                client_ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_st, &client_ev);
                /*
                 * ע�ͣ���epoll����listen_st���������socket����Ϣ��ʱ��
                 * ֻ���������Կͻ��˵�������Ϣ
                 * recv,sendʹ�õĶ��ǿͻ��˵�socket��������listen_st������Ϣ��
                 */
                continue;
            }
            //�ͻ������¼�����
            if (events[i].events & EPOLLIN)
            {
                //��ʾ��������ߵ�client_st���յ���Ϣ
                if (socket_recv(events[i].data.fd) < 0)
                {
                    close_socket(events[i].data.fd);
                    //�������ݳ�����߿ͻ����Ѿ��ر�
                    events[i].data.fd = -1;
                    /*����continue����Ϊ�ͻ���socket�Ѿ����ر��ˣ�
                     * �������socket���ܻ����������¼��������ִ���������¼���
                     * �������socket�Ѿ������ó�-1
                     * ���Ժ����close_socket()�������ᱨ��
                     */
                    continue;
                }
                /*
                 * �˴�����continue����Ϊÿ��socket�������ж���¼�ͬʱ���͵���������
                 * ��Ҳ�����������if������if-else��ԭ��
                 */

            }
            //�ͻ������¼�����
            if (events[i].events & EPOLLERR)
            {
                printf("EPOLLERR\n");
                //���س����¼����ر�socket������epoll�أ����ر�socket����events[i].data.fd=-1,epoll���Զ�����socket�ӳ������
                close_socket(events[i].data.fd);
                events[i].data.fd = -1;
                continue;
            }
            //�ͻ������¼�����
            if (events[i].events & EPOLLHUP)
            {
                printf("EPOLLHUP\n");
                //���ع����¼����ر�socket������epoll��
                close_socket(events[i].data.fd);
                events[i].data.fd = -1;
                continue;
            }
        }




        memset(peer_ip, 0, sizeof(peer_ip));
        if ((new_fd = tcp_accept(handle->distributed_sock_fd, peer_ip, &peer_port)) > 0)
        {
            debug_print("new connect in [distributed]\n");
            distributed_mgr_add(handle->hdistributed_mgr, new_fd);
        }
    }

    return NULL;
}

static void * dev_thread_center(long user_info)
{
    int  new_fd;
    int8   peer_ip[16];
    uint16 peer_port;
    TCP_SERVER_OBJECT *handle = (TCP_SERVER_OBJECT *)user_info;
    while(1)
    {
        memset(peer_ip, 0, sizeof(peer_ip));
        if ((new_fd = tcp_accept(handle->device_sock_fd, peer_ip, &peer_port)) > 0)
        {
            debug_print("new device connect in  %s:%d \n", peer_ip, peer_port);
            // proxy_mgr_dev_add(handle->hproxy_mgr, new_fd);
        }
    }

    return NULL;
}

static void * flush_center(void * arg)
{
    TCP_SERVER_OBJECT *handle = (TCP_SERVER_OBJECT *)arg;
    while(1)
    {
        proxy_mgr_flush(handle->hproxy_mgr);
        os_sleep_sec(10);
    }

    return NULL;
}

static TCP_SERVER_OBJECT * instance(void)
{
    static TCP_SERVER_OBJECT *handle = NULL;
    if (NULL == handle)
    {
        handle = (TCP_SERVER_OBJECT *)malloc(sizeof(TCP_SERVER_OBJECT));
        memset(handle, 0, sizeof(TCP_SERVER_OBJECT));
    }

    return handle;
}
