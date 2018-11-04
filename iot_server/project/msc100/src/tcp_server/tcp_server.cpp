#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <core/core.h>

#include <core/core.h>

#include "device_manage/device_manage.h"
#include "device_manage/proxy_manage.h"
#include "device_manage/distributed_manage.h"

#include "tcp_server.h"

#define  MAX_MSG_LEN  1024

typedef struct TcpServerObject
{
    int32 device_sock_fd;
    int32 client_sock_fd;
    int32 distributed_sock_fd;

    DEVICE_MGR_HANDLE hdev_mgr;
    DISTRIBUTED_MANAGE_HANDLE hdistributed_mgr;
    PROXY_MANAGE_HANDLE       hproxy_mgr;

    THREAD_HANDLE hthread_distributed;
    THREAD_HANDLE hthread_device_msg;
    THREAD_HANDLE hthread_client_msg;
    THREAD_HANDLE hthread_flush;
    THREAD_HANDLE hthread_visit_flush;
} TcpServerObject;

// port:
// 8010 msg from device.
// 8020 msg from client.
// 8030 used for distributed.

static void * distributed_accept_center( void * arg );
static void * device_accept_center( void * arg );
static void * client_accept_center( void * arg );
static void * flush_center( void * arg );
static TcpServerObject * instance( void );

TCP_SERVER_HANDLE tcp_server_create( void )
{
    TcpServerObject *handle = instance();
    if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // create and bind 3 socket.
    // for device communication.
    handle->device_sock_fd = tcp_open_and_bind( 8010 );
    if ( handle->device_sock_fd <= 0 )
    {
        debug_error("socket create failed \n");
        return NULL;
    }
    if ( !tcp_listen( handle->device_sock_fd, 10 ) )
    {
        debug_error("socket listen failed \n");
        return NULL;
    }

    // for client communication.
    handle->client_sock_fd = tcp_open_and_bind( 8020 );
    if ( handle->client_sock_fd <= 0 )
    {
        debug_error("socket create failed \n");
        return NULL;
    }
    if ( !tcp_listen( handle->client_sock_fd, 10 ) )
    {
        debug_error("socket listen failed \n");
        return NULL;
    }

    // for distributed communication.
    handle->distributed_sock_fd = tcp_open_and_bind( 8030 );
    if ( handle->distributed_sock_fd <= 0 )
    {
        debug_error("socket create failed \n");
        return NULL;
    }
    if ( !tcp_listen( handle->distributed_sock_fd, 10 ) )
    {
        debug_error("socket create failed \n");
        return NULL;
    }

    handle->hdev_mgr = device_mgr_create( MAX_CLIENTS );
    if ( NULL == handle->hdev_mgr )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    DistributedMgrEnv dis_mgr_env;
    dis_mgr_env.hdev_mgr = handle->hdev_mgr;

    // create distributed and proxy.
    handle->hdistributed_mgr = distributed_mgr_create( &dis_mgr_env );
    if ( NULL == handle->hdistributed_mgr )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    ProxyMgrEnv proxy_mgr_env;
    proxy_mgr_env.hdev_mgr = handle->hdev_mgr;

    handle->hproxy_mgr = proxy_mgr_create( &proxy_mgr_env );
    if ( NULL == handle->hproxy_mgr )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // create 4 thread.
    if ( 0 != os_create_thread( &handle->hthread_distributed,
                                NULL,
                                ( sp_thread_func_t )distributed_accept_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_device_msg,
                                NULL,
                                ( sp_thread_func_t )device_accept_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_client_msg,
                                NULL, (sp_thread_func_t)client_accept_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_flush,
                                NULL,
                                (sp_thread_func_t)flush_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    return handle;
}

void tcp_server_destroy( TCP_SERVER_HANDLE handle )
{
    if ( handle->client_sock_fd > 0 )
    {
        udp_close( handle->client_sock_fd );
    }
    if ( handle->device_sock_fd > 0 )
    {
        udp_close( handle->device_sock_fd );
    }
    if ( handle->distributed_sock_fd > 0 )
    {
        udp_close( handle->distributed_sock_fd );
    }

    os_close_thread( handle->hthread_distributed );
    os_close_thread( handle->hthread_device_msg );
    os_close_thread( handle->hthread_client_msg );
    os_close_thread( handle->hthread_flush );

    distributed_mgr_destroy( handle->hdistributed_mgr );
    proxy_mgr_destroy( handle->hproxy_mgr );
    device_mgr_destroy( handle->hdev_mgr );

    free( handle );
}

static void * distributed_accept_center( void * arg )
{
    int32  new_fd;
    int8   peer_ip[16];
    uint16 peer_port;
    TcpServerObject *handle = ( TcpServerObject * )arg;
    while( 1 )
    {
        memset( peer_ip, 0, sizeof( peer_ip ) );
        if ( ( new_fd = tcp_accept( handle->distributed_sock_fd, peer_ip, &peer_port ) ) > 0 )
        {
            debug_print("new connect in [distributed]\n");
            distributed_mgr_add( handle->hdistributed_mgr, new_fd );
        }
    }

    return NULL;
}

static void * device_accept_center( void * arg )
{
    int32  new_fd;
    int8   peer_ip[16];
    uint16 peer_port;
    TcpServerObject *handle = ( TcpServerObject * )arg;
    while( 1 )
    {
        memset( peer_ip, 0, sizeof( peer_ip ) );
        if ( ( new_fd = tcp_accept( handle->device_sock_fd, peer_ip, &peer_port ) ) > 0 )
        {
            debug_print("new device connect in  %s:%d \n", peer_ip, peer_port );
            proxy_mgr_dev_add( handle->hproxy_mgr, new_fd );
        }
    }

    return NULL;
}

static void * client_accept_center( void * arg )
{
    int32  new_fd;
    int8   peer_ip[16];
    uint16 peer_port;
    TcpServerObject *handle = ( TcpServerObject * )arg;
    while( 1 )
    {
        memset( peer_ip, 0, sizeof( peer_ip ) );
        if ( ( new_fd = tcp_accept( handle->client_sock_fd, peer_ip, &peer_port ) ) > 0 )
        {
            if ( proxy_mgr_clt_add( handle->hproxy_mgr, new_fd ) )
            {
                debug_print("new client connect in %s:%d \n", peer_ip, peer_port );
                proxy_mgr_clt_add( handle->hproxy_mgr, new_fd );
            }
        }
    }

    return NULL;
}

static void * flush_center( void * arg )
{
    TcpServerObject *handle = ( TcpServerObject * )arg;
    while( 1 )
    {
        proxy_mgr_flush( handle->hproxy_mgr );
        os_sleep_sec( 10 );
    }

    return NULL;
}

static TcpServerObject * instance( void )
{
    static TcpServerObject *handle = NULL;
    if ( NULL == handle )
    {
        handle = ( TcpServerObject * )malloc( sizeof( TcpServerObject ) );
        memset( handle, 0, sizeof( TcpServerObject ) );
    }

    return handle;
}
