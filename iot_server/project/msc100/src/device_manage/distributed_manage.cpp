#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "device_manage/device_manage.h"
#include "device_manage/visit_manage.h"
#include "msg_handle/msg_define.h"
#include "msg_handle/msg_handle.h"

#include "distributed_manage.h"

typedef struct DistributedClientParam
{
    struct list_head list;
    int32  sock_fd;
    int32  timeout_sec;
} DistributedClientParam;

typedef struct DistributedManageObject
{
    struct  list_head head;  // distributed head;
    MUTEX_HANDLE      hmutex;
    THREADP_HANDLE    hthread_pool;
    DEVICE_MGR_HANDLE hdev_mgr;
} DistributedManageObject;

typedef struct ThreadPoolParam
{
    uint32 index;
    DistributedManageObject *distributed_obj;
} ThreadPoolParam;

static int32 distributed_thread_center( void *param );

DISTRIBUTED_MANAGE_HANDLE distributed_mgr_create( DistributedMgrEnv *env )
{
    int32 i;
    DistributedManageObject *handle = ( DistributedManageObject * )calloc( 1, sizeof( DistributedManageObject ) );
	if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->hdev_mgr = env->hdev_mgr;

    INIT_LIST_HEAD( &handle->head );

    os_mutex_open( &handle->hmutex, NULL );

    // handle request count: 50 * 100 = 5000.
    int32 max_thread_count = 50;
    handle->hthread_pool = thread_pool_create( max_thread_count );
    if ( NULL == handle->hthread_pool )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    ThreadPoolParam *thread_pool_param = NULL;
    for ( i=0; i<max_thread_count; i++ )
    {
        // 0-99, 100-199, 200-299 etc, total 5000.
        thread_pool_param = ( ThreadPoolParam* )malloc( sizeof( ThreadPoolParam ) );
        if ( NULL != thread_pool_param )
        {
            thread_pool_param->index = i;
            thread_pool_param->distributed_obj = handle;
            thread_pool_dispatch( handle->hthread_pool, distributed_thread_center, thread_pool_param );
        }
    }

    return handle;
}

boolean distributed_mgr_add( DISTRIBUTED_MANAGE_HANDLE handle, int32 sock_fd )
{
    DistributedClientParam* list_node = NULL;

    list_node = ( DistributedClientParam * )calloc( 1, sizeof( DistributedClientParam ) );
    if ( NULL == list_node )
    {
        debug_print("not enough memory \n");
        return FALSE;
    }

    os_mutex_lock( &handle->hmutex );
    list_node->timeout_sec = get_real_time_sec() + 10; // time out is 10 secs.
    list_node->sock_fd = sock_fd;
    list_add_tail( &list_node->list, &handle->head );
    os_mutex_unlock( &handle->hmutex );

    return TRUE;
}

void distributed_mgr_destroy( DISTRIBUTED_MANAGE_HANDLE handle )
{
    DistributedClientParam* list_node = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        list_node = list_entry( pos, DistributedClientParam, list );
        if ( NULL != list_node )
        {
            os_mutex_lock( &handle->hmutex );
            tcp_close( list_node->sock_fd );
            list_del( &list_node->list );
            free( list_node );
            os_mutex_unlock( &handle->hmutex );
        }
    }
    list_del( &handle->head );
    os_mutex_close( &handle->hmutex );

    // destroy all thread.
    thread_pool_destroy( handle->hthread_pool );

    free( handle );
}

static int32 distributed_thread_center( void *param )
{
    ThreadPoolParam *arg = ( ThreadPoolParam * )param;

    int32 index = arg->index;
    DistributedManageObject *handle = arg->distributed_obj;
    int8   id[24];
    int8   buffer[128];
    int8   msg_body[128];
    int32  recv_len = 0;
    uint16 msg, out_len;

    DistributedClientParam* list_node = NULL;

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    int32 count = 0;
    int32 cur_start_index = ( 100 * ( index ) );

    while( 1 )
    {
        count = 0;
        list_for_each_safe( pos, n, &handle->head )
        {
            list_node = list_entry( pos, DistributedClientParam, list );
            if ( NULL != list_node )
            {
                // 0-99 100-199 etc.
                if ( ( count >= cur_start_index ) && ( count < ( cur_start_index + 100 ) ) )
                {
                    recv_len = tcp_recv_timeout( list_node->sock_fd, buffer, sizeof( buffer ), 10 );
                    if ( recv_len > 0 )
                    {
                        msg = 0;
                        out_len = sizeof( msg_body );
                        memset( id, 0, sizeof( id ) );
                        memset( msg_body, 0, out_len );

                        if ( msg_handle_unpacket( msg_body, &out_len, id, &msg, buffer, recv_len ) )
                        {
                            if ( MSG_A_PROXY_SVR_ADDR == msg )
                            {
                                int8 ip[16];

                                memset( ip, 0, sizeof( ip ) );
                                if ( device_mgr_get_proxy_server_addr( handle->hdev_mgr, id, ip ) )
                                {
                                    uint16 out_len = sizeof( buffer );
                                    memset( msg_body, 0, sizeof( msg_body ) );
                                    sprintf( msg_body, "{\n\"ip\":\"%s\"\n}\n", ip );
                                    memset( id, 0, sizeof( id ) );
                                    if ( msg_handle_packet( buffer, &out_len, id, MSG_R_PROXY_SVR_ADDR, msg_body, strlen( msg_body ) ) )
                                    {
                                        debug_print("send proxy server %s \n", msg_body );
                                        tcp_send( list_node->sock_fd, buffer, out_len );
                                    }
                                }
                            }
                        }
                    }
                    else if ( 0 == recv_len ) // peer closed.
                    {
                        os_mutex_lock( &handle->hmutex );
                        tcp_close( list_node->sock_fd );
                        list_del( &list_node->list );
                        free( list_node );
                        os_mutex_unlock( &handle->hmutex );
                        debug_print("[distributed]peer closed ? \n");
                    }
                    else
                    {
                        // time out, may be block.
                        if ( get_real_time_sec() >= list_node->timeout_sec )
                        {
                            os_mutex_lock( &handle->hmutex );
                            tcp_close( list_node->sock_fd );
                            list_del( &list_node->list );
                            free( list_node );
                            os_mutex_unlock( &handle->hmutex );
                            debug_print("[distributed] time out \n");
                        }
                    }
                }

                count++;
            }
        }
        os_sleep_ms( 10 );
    }

    free( arg );

    return 0;
}
