#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include "device_manage/device_manage.h"
//#include "device_manage/visit_manage.h"

#include "protocol/protocol.h"
#include "db/group_manage.h"

#include "msg_handle/msg_define.h"
#include "msg_handle/msg_handle.h"

#include "hash_value.h"
//#include "distributed_manage.h"
#include "proxy_manage.h"

#define MAX_MSG_LEN 1024

typedef struct DeviceConnectParam
{
    struct list_head list;
    uint32 next_sec;
    int32  sock_fd;
} DeviceConnectParam;

typedef struct ClientConnectParam
{
    struct list_head list;
    uint32 next_sec;
    int32  sock_fd;
    int32  login_success;
} ClientConnectParam;


typedef struct ProxyManageObject
{
    struct  list_head head_dev;
    MUTEX_HANDLE      hmutex_dev;
    THREADP_HANDLE    hthread_pool_dev;

    struct  list_head head_clt;
    MUTEX_HANDLE      hmutex_clt;
    THREADP_HANDLE    hthread_pool_clt;

    int32             max_thread_count;

    GROUP_MGR_HANDLE  hgroup_mgr;

} ProxyManageObject;

typedef struct DispatchParam
{
    uint32 index;
    struct list_head   head;
    ProxyManageObject *proxy_obj;
} DispatchParam;

static int32 dev_thread_center( void *param );
static int32 clt_thread_center( void *param );

static boolean client_get_register_param( int8 * in_buf, int8 * sn, int8 * home_group,
                                          int8 *email, int8 * password, int8 * random );
static boolean client_get_login_param( int8 * in_buf, int8 *sn, int8 * password );
static boolean client_get_sn( int8 * in_buf, int8 *sn );
static int32   client_get_upload_dev( int8 * in_buf, int8 *sn, int32 array_size, int8 *mac[], int8 *name[], int8 *version[] );
static boolean client_get_sn_mac( int8 * in_buf, int8 *sn, int8 * mac );

PROXY_MANAGE_HANDLE proxy_mgr_create( void )
{
    int32 i;
    ProxyManageObject *handle = ( ProxyManageObject * )calloc( 1, sizeof( ProxyManageObject ) );
	if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    //handle->hdev_mgr = env->hdev_mgr;
    //handle->hvisit_mgr = visit_mgr_create();
    //if ( NULL == handle->hvisit_mgr )
    //{
    //    debug_error("not enough memory \n");
    //    return NULL;
    //}

    handle->hgroup_mgr = group_mgr_create();
    if ( NULL == handle->hgroup_mgr )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // device connection.
    INIT_LIST_HEAD( &handle->head_dev );
    os_mutex_open( &handle->hmutex_dev, NULL );

    // handle request count: 50 * 100 = 5000.
    handle->max_thread_count = 10;
    handle->hthread_pool_dev = thread_pool_create( handle->max_thread_count );
    if ( NULL == handle->hthread_pool_dev )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    DispatchParam * dispatch_param = NULL;
    for ( i=0; i<handle->max_thread_count; i++ )
    {
        // 0-99, 100-199, 200-299 etc, total 5000.
        dispatch_param = ( DispatchParam * )malloc( sizeof( DispatchParam ) );
        if ( NULL != dispatch_param )
        {
            dispatch_param->index = i;
            INIT_LIST_HEAD( &dispatch_param->head );
            list_add_tail( &dispatch_param->head, &handle->head_dev );
            dispatch_param->proxy_obj = handle;
            thread_pool_dispatch( handle->hthread_pool_dev, dev_thread_center, dispatch_param );
        }
    }

    // client connection.
    INIT_LIST_HEAD( &handle->head_clt );
    os_mutex_open( &handle->hmutex_clt, NULL );
    // handle request count: 10 * 100 = 1000.
    handle->max_thread_count = 10;
    handle->hthread_pool_clt = thread_pool_create( handle->max_thread_count );
    if ( NULL == handle->hthread_pool_clt )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    for ( i=0; i<handle->max_thread_count; i++ )
    {
        // 0-99, 100-199, 200-299 etc, total 5000.
        dispatch_param = ( DispatchParam * )malloc( sizeof( DispatchParam ) );
        if ( NULL != dispatch_param )
        {
            dispatch_param->index = i;
            INIT_LIST_HEAD( &dispatch_param->head );
            list_add_tail( &dispatch_param->head, &handle->head_clt );
            dispatch_param->proxy_obj = handle;

            thread_pool_dispatch( handle->hthread_pool_clt, clt_thread_center, dispatch_param );
        }
    }

    return handle;
}

boolean proxy_mgr_dev_add( PROXY_MANAGE_HANDLE handle, int32 sock_fd )
{
    DeviceConnectParam* list_node = NULL;

    list_node = ( DeviceConnectParam * )calloc( 1, sizeof( DeviceConnectParam ) );
    if ( NULL == list_node )
    {
        debug_print("not enough memory \n");
        return FALSE;
    }

    os_mutex_lock( &handle->hmutex_dev );
    list_node->sock_fd = sock_fd;
    list_node->next_sec = get_real_time_sec() + 120;
    list_add_tail( &list_node->list, &handle->head_dev );
    os_mutex_unlock( &handle->hmutex_dev );

    return TRUE;
}

boolean proxy_mgr_clt_add( PROXY_MANAGE_HANDLE handle, int32 sock_fd )
{
    ClientConnectParam* list_node = NULL;

    list_node = ( ClientConnectParam * )calloc( 1, sizeof( ClientConnectParam ) );
    if ( NULL == list_node )
    {
        debug_print("not enough memory \n");
        return FALSE;
    }

    boolean ret = FALSE;
    boolean finish = FALSE;
    int32   count  = 0;
    DispatchParam* dis_list_node = NULL;
    struct list_head *dis_pos = NULL;
    struct list_head *dis_n   = NULL;

    list_for_each_safe( dis_pos, dis_n, &handle->head_clt )
    {
        dis_list_node = list_entry( dis_pos, DispatchParam, head );
        if ( NULL != dis_list_node )
        {
            // level 2 list
            ClientConnectParam* clt_node = NULL;
            struct list_head *clt_pos = NULL;
            struct list_head *clt_n   = NULL;

            count = 0;
            list_for_each_safe( clt_pos, clt_n, &dis_list_node->head )
            {
                clt_node = list_entry( clt_pos, ClientConnectParam, list );
                if ( NULL != clt_node )
                {
                    count++;
                }
            }

            if ( count < handle->max_thread_count )
            {
                os_mutex_lock( &handle->hmutex_clt );
                list_node->sock_fd = sock_fd;
                list_node->next_sec = get_real_time_sec() + 180;
                list_add_tail( &list_node->list, &dis_list_node->head );
                os_mutex_unlock( &handle->hmutex_clt );
                finish = TRUE;
            }
        }

        if ( finish )
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

void proxy_mgr_flush( PROXY_MANAGE_HANDLE handle )
{
    //visit_mgr_flush( handle->hvisit_mgr );
    //device_mgr_flush( handle->hdev_mgr );
}

void proxy_mgr_destroy( PROXY_MANAGE_HANDLE handle )
{
#if 0
    DeviceConnectParam* list_node = NULL;
    ClientConnectParam* list_node_clt = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    // device.
    list_for_each_safe( pos, n, &handle->head_dev )
    {
        list_node = list_entry( pos, DeviceConnectParam, list );
        if ( NULL != list_node )
        {
            os_mutex_lock( &handle->hmutex_dev );
            tcp_close( list_node->sock_fd );
            list_del( &list_node->list );
            free( list_node );
            os_mutex_unlock( &handle->hmutex_dev );
        }
    }
    list_del( &handle->head_dev );
    os_mutex_close( &handle->hmutex_dev );
    // destroy all device thread.
    thread_pool_destroy( handle->hthread_pool_dev );

    // client.
    list_for_each_safe( pos, n, &handle->head_clt )
    {
        list_node_clt = list_entry( pos, ClientConnectParam, list );
        if ( NULL != list_node_clt )
        {
            os_mutex_lock( &handle->hmutex_clt );
            tcp_close( list_node->sock_fd );
            list_del( &list_node->list );
            free( list_node_clt );
            os_mutex_unlock( &handle->hmutex_clt );
        }
    }
    list_del( &handle->head_clt );
    os_mutex_close( &handle->hmutex_clt );
    // destroy all client thread.
    thread_pool_destroy( handle->hthread_pool_clt );

    visit_mgr_destroy( handle->hvisit_mgr );
#endif
    free( handle );
}

static int32 dev_thread_center( void *param )
{
#if 0
    DispatchParam *arg = ( DispatchParam *)param;
    ProxyManageObject *handle = arg->proxy_obj;
    int32  index              = arg->index;

    int8   id[24];
    uint32 i, array_size;
    int32  sock_fd[10];
    uint32 hash_value[10];

    int8   *buffer   = ( int8 * )malloc( MAX_MSG_LEN );
    int8   *msg_body = ( int8 * )malloc( MAX_MSG_LEN );
    int32  recv_len = 0;
    uint16 msg, out_len;

    DeviceConnectParam* list_node = NULL;

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    int32 count = 0;
    int32 cur_start_index = ( 100 * ( index ) );

    // debug_print("dev thread %d created \n", index );
    while( 1 )
    {
        count = 0;
        list_for_each_safe( pos, n, &handle->head_dev )
        {
            list_node = list_entry( pos, DeviceConnectParam, list );
            if ( NULL != list_node )
            {
                // 0-99 100-199 etc.
                if ( ( count >= cur_start_index ) && ( count < ( cur_start_index + 100 ) ) )
                {
                    recv_len = tcp_recv_timeout( list_node->sock_fd, buffer, MAX_MSG_LEN, 10 );
                    if ( recv_len > 0 )
                    {
                        out_len = MAX_MSG_LEN;
                        memset( msg_body, 0, MAX_MSG_LEN );
                        memset( id, 0, sizeof( id ) );

                        // printf("==== device recved msg ... %d \n", recv_len );
                        if ( msg_handle_unpacket( msg_body, &out_len, id, &msg, buffer, recv_len ) )
                        {
                            if ( MSG_A_REGISTER == msg )
                            {
                                int8 *json_msg = NULL;
                                int8   ip[16];
                                uint16 port;
                                int8   hash_buf[24];
                                uint32 hash_value = 0;

                                memset( ip, 0, sizeof( ip ) );
                                if ( tcp_get_peer_addr( list_node->sock_fd, ip, &port ) )
                                {
                                    memset( hash_buf, 0, sizeof( hash_buf ) );
                                    sprintf( hash_buf, "%s%d", ip, port );
                                    hash_value = string_to_hash( hash_buf ); // convert to hash value for speed up.

                                    if ( device_mgr_add( handle->hdev_mgr, id, list_node->sock_fd, hash_value ) )
                                    {
                                        json_msg = (int8*)"{\n\"id\":\"success\"\n}\n";
                                        debug_print("device: [%s] register.\n", id );
                                    }
                                    else
                                    {
                                        json_msg = (int8*)"{\n\"id\":\"unexist\"\n}\n";
                                        debug_print("id:[%s] unexist \n", id );
                                    }
                                }
                                else
                                {
                                    json_msg = (int8*)"{\n\"addr\":\"error\"\n}\n";
                                    debug_print("id:[%s] socket failed \n", id );
                                }

                                // send MSG_R_REGISTER herel.
                                out_len = MAX_MSG_LEN;
                                if ( msg_handle_packet( buffer, &out_len, id, MSG_R_REGISTER, json_msg, strlen( json_msg ) ) )
                                {
                                    tcp_send( list_node->sock_fd, buffer, out_len );
                                }
                                list_node->next_sec = get_real_time_sec() + 180; // we delay the timeout when we recv msg.
                            }
                            else if ( MSG_A_KEEP_ALIVE == msg )
                            {
                                // printf("%s keep alive \n", id );
                                device_mgr_keep_alive( handle->hdev_mgr, id );
                                list_node->next_sec = get_real_time_sec() + 180; // we delay the timeout when we recv msg.
                            }
                            else if ( MSG_A_PROXY == msg )
                            {
                                array_size = ( sizeof( sock_fd ) / sizeof( sock_fd[0] ) );
                                if ( visit_mgr_get_sock_fd( handle->hvisit_mgr, id, &array_size, sock_fd, hash_value ) )
                                {

                                    uint16 in_buf_len = out_len;
                                    uint16 out_len = MAX_MSG_LEN;

                                    if ( msg_handle_packet( buffer, &out_len, id, MSG_A_PROXY, msg_body, in_buf_len ) )
                                    {
                                        debug_print("proxy [device to client]msg total %d ! \n", array_size);
                                        for ( i=0; i<array_size; i++ )
                                        {
                                            debug_print("send proxy: %s \n", msg_body );
                                            if ( 0 != tcp_send( sock_fd[i], buffer, out_len ) )
                                            {
                                                visit_mgr_keep_alive( handle->hvisit_mgr, id, hash_value[i] );
                                            }
                                            else
                                            {
                                                visit_mgr_remove( handle->hvisit_mgr, id, hash_value[i] );
                                            }
                                        }
                                    }
                                }

                                list_node->next_sec = get_real_time_sec() + 180; // we delay the timeout when we recv msg.
                            }
                        }
                    }
                    else if ( 0 == recv_len ) // peer closed.
                    {
                        debug_print("peer closed ? \n");
                        os_mutex_lock( &handle->hmutex_dev );
                        tcp_close( list_node->sock_fd );
                        list_del( &list_node->list );
                        free( list_node );
                        os_mutex_unlock( &handle->hmutex_dev );
                    }
                    else
                    {
                        // time out, may be block.
                        if ( get_real_time_sec() >= (long long)list_node->next_sec )
                        {
                            os_mutex_lock( &handle->hmutex_dev );
                            tcp_close( list_node->sock_fd );
                            list_del( &list_node->list );
                            free( list_node );
                            os_mutex_unlock( &handle->hmutex_dev );
                            debug_print("[device] time out \n");
                        }
                    }
                }
                count++;
            }
        }
        os_sleep_ms( 10 );
    }

    if ( NULL != buffer )
    {
        free( buffer );
    }
    if ( NULL != msg_body )
    {
        free( msg_body );
    }
    free( arg );
#endif

    return 0;
}

static int32 clt_thread_center( void *param )
{
    DispatchParam *arg = ( DispatchParam *)param;
    ProxyManageObject *handle = arg->proxy_obj;
    //int32  index              = arg->index;

    int8   *buffer   = ( int8 * )malloc( MAX_MSG_LEN );
    int8   *msg_body = ( int8 * )malloc( MAX_MSG_LEN );
    int32  recv_len = 0;

    ClientConnectParam* list_node = NULL;

    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    // debug_print("clt thread %d created \n", index );
    while( 1 )
    {
        list_for_each_safe( pos, n, &arg->head )
        {
            list_node = list_entry( pos, ClientConnectParam, list );
            if ( NULL != list_node )
            {
                recv_len = tcp_recv_timeout( list_node->sock_fd, buffer, MAX_MSG_LEN, 20 );
                if ( recv_len > 0 )
                {
                    printf("===== recv %d bytes \n", recv_len );
                    int8 from, msg;
                    uint16 buf_len = MAX_MSG_LEN;
                    memset( msg_body, 0, buf_len );
                    uint16 out_len = msg_handle_unpacket( msg_body, buf_len, &from, &msg, buffer, recv_len );
                    if ( ( out_len <= 0 ) || ( 0x01 != from ) )
                    {
                        debug_print("msg is not come from client 0x%x \n", from);
                        continue;
                    }

                    switch( msg )
                    {
                        case CLIENT_A_DYNAMIC_PWD:
                        {
                            int8 random[8];
                            memset( random, 0, sizeof( random ) );
                            if ( 11 == strlen( (int8 *)msg_body ) ) // telephone ascii sn.
                            {
                                if ( group_mgr_client_get_random( handle->hgroup_mgr, (int8 *)msg_body, random ) )
                                {
                                    // 6 ascii random.
                                    buf_len = MAX_MSG_LEN;
                                    memset( buffer, 0, buf_len );
                                    out_len = msg_handle_packet( buffer, buf_len, 0x03, CLIENT_R_DYNAMIC_PWD, random, 6 );
                                    if ( ( out_len > 0 ) && ( out_len < MAX_MSG_LEN ) )
                                    {
                                        tcp_send( list_node->sock_fd, buffer, out_len );
                                    }
                                }
                            }

                            list_node->next_sec = get_real_time_sec() + 120; // we delay the timeout when we recv msg.
                            break;
                        }
                        case CLIENT_A_REGISTER:
                        {
                            // sn | home_group | email | password | random
                            int8 sn[12] = { 0 };
                            int8 home_group[32] = { 0 };
                            int8 email[32] = { 0 };
                            int8 password[32] = { 0 };
                            int8 random[8] = { 0 };
                            int8 body = 0;

                            // 00 成功 0x01 参数不正确 0x02动态验证码无效
                            // 0x03 动态验证码错误 0x04保存注册信息失败 0x05手机已注册过家庭组
                            if ( client_get_register_param( msg_body, sn, home_group,
                                                            email, password, random ) )
                            {
                                int32 res = group_mgr_client_register( handle->hgroup_mgr, sn, home_group,
                                                                       email, password, random );

                                if ( res == RET_DB_RANDOM_ERROR )
                                {
                                    body = 0x03;
                                }
                                else if ( res == RET_DB_REGISTED_ALREADY  )
                                {
                                    body = 0x05;
                                }
                            }
                            else
                            {
                                body = 0x01;
                            }

                            buf_len = MAX_MSG_LEN;
                            memset( buffer, 0, buf_len );
                            out_len = msg_handle_packet( buffer, buf_len, 0x03, CLIENT_R_REGISTER, &body, 1 );
                            if ( ( out_len > 0 ) && ( out_len < MAX_MSG_LEN ) )
                            {
                                tcp_send( list_node->sock_fd, buffer, out_len );
                            }

                            break;
                        }
                        case CLIENT_A_LOGIN:
                        {
                            int8 body = 0;
                            int8 sn[12] = { 0 };
                            int8 password[32] = { 0 };

                            if ( client_get_login_param( msg_body, sn, password ) )
                            {
                                int8 db_password[32] = { 0 };
                                if ( group_mgr_client_get_password( handle->hgroup_mgr, sn, db_password ) )
                                {
                                    if ( 0 == strcmp( db_password, password ) )
                                    {
                                        list_node->login_success = TRUE;
                                        body = 0x00;
                                    }
                                    else
                                    {
                                        body = 0x02;
                                    }
                                }
                                else
                                {
                                    body = 0x01; // param error.
                                }
                            }
                            else
                            {
                                body = 0x01; // param error.
                            }

                            buf_len = MAX_MSG_LEN;
                            memset( buffer, 0, buf_len );
                            out_len = msg_handle_packet( buffer, buf_len, 0x03, CLIENT_R_LOGIN, &body, 1 );
                            if ( ( out_len > 0 ) && ( out_len < MAX_MSG_LEN ) )
                            {
                                tcp_send( list_node->sock_fd, buffer, out_len );
                            }
                        }
                        case CLIENT_A_GET_DEV_LIST:
                        {
                            int32 body_len = 0;
                            int8 sn[12] = { 0 };
                            int8 buf[256] = { 0 };

                            if ( client_get_sn( msg_body, sn ) )
                            {
                                int32 i;
                                int8 *mac[16];
                                for ( i=0; i<16; i++ ) // max count is 16;
                                {
                                    mac[i] = &buf[i*13]; // prepare buffer array.
                                }

                                int32 count = group_mgr_client_get_device_mac( handle->hgroup_mgr, sn, mac );
                                for( i=0; i<count; i++ )
                                {
                                    memcpy( buf, mac[i], 12 ); // mac length.

                                    if ( i != ( count -1 ) )
                                    {
                                        // 001122334455|001122334456
                                        buf[i*12+12] = '|';
                                    }
                                }
                                if ( body_len > 0 )
                                {
                                    body_len = count*13-1;
                                }
                            }

                            buf_len = MAX_MSG_LEN;
                            memset( buffer, 0, buf_len );
                            out_len = msg_handle_packet( buffer, buf_len, 0x03, CLIENT_R_GET_DEV_LIST, buf, body_len );
                            if ( ( out_len > 0 ) && ( out_len < MAX_MSG_LEN ) )
                            {
                                tcp_send( list_node->sock_fd, buffer, out_len );
                            }
                            break;
                        }
                        case CLIENT_A_UPLOAD_DEV_LIST:
                        {
                            // sn | mac1#name1#version1|mac2#name2#version2
                            int32 i, count, op_success;
                            int8 body = 0x02;
                            int8 sn[12] = { 0 };
                            int8 mac_buf[12*4];
                            int8 name_buf[32*4];
                            int8 ver_buf[8*4];

                            int8 *mac[4], *name[4], *version[4];
                            for ( i=0; i<4; i++ )
                            {
                                mac[i]     = &mac_buf[12*i];
                                name[i]    = &name_buf[32*i];
                                version[i] = &ver_buf[8*i];
                            }

                            op_success = 0;
                            count = client_get_upload_dev( msg_body, sn, 4, mac, name, version );
                            if ( count > 0 )
                            {
                                for ( i=0; i<count; i++ )
                                {
                                    if ( RET_SUCCESS == group_mgr_add_device( handle->hgroup_mgr, sn, mac[i], name[i], version[i] ) )
                                    {
                                        op_success++;
                                    }
                                }
                                if ( op_success == op_success )
                                {
                                    body = 0x00;
                                }
                                else
                                {
                                    body = 0x03;
                                }
                            }
                            else
                            {
                                // failed;
                                body = 0x01;
                            }
                            buf_len = MAX_MSG_LEN;
                            memset( buffer, 0, buf_len );
                            out_len = msg_handle_packet( buffer, buf_len, 0x03, CLIENT_R_UPLOAD_DEV_LIST, &body, 1 );
                            if ( ( out_len > 0 ) && ( out_len < MAX_MSG_LEN ) )
                            {
                                tcp_send( list_node->sock_fd, buffer, out_len );
                            }


                            break;
                        }
                        case CLIENT_A_INQUIRE_DEV_STATUS:
                        {
                            // 13112345678|000E1A99B3C0
                            int8 sn[12] = { 0 };
                            int8 mac[12] = { 0 };
                            int32 body_len = 0;
                            DeviceParam dev_param;
                            int8 response_buf[128] = { 0 };
                            if ( client_get_sn_mac( msg_body, sn, mac ) )
                            {
                                // mac|on.off|pm2.5|filter_time
                                memset( &dev_param, 0, sizeof( DeviceParam ) );
                                if ( group_mgr_device_get_param( handle->hgroup_mgr, mac, &dev_param ) )
                                {
                                    debug_print("get |%s|%s|%s \n", mac, dev_param.on_off, dev_param.pm25, dev_param.filter_time );
                                    sprintf( response_buf, "%s|%s|%s|%s", mac, dev_param.on_off, dev_param.pm25, dev_param.filter_time );
                                    body_len = strlen( response_buf );
                                }

                                buf_len = MAX_MSG_LEN;
                                memset( buffer, 0, buf_len );
                                out_len = msg_handle_packet( buffer, buf_len, 0x03, CLIENT_R_INQUIRE_DEV_STATUS, response_buf, body_len );
                                if ( ( out_len > 0 ) && ( out_len < MAX_MSG_LEN ) )
                                {
                                    tcp_send( list_node->sock_fd, buffer, out_len );
                                }
                            }

                            break;
                        }
                        default:
                            break;
                    }
                }
                else if ( 0 == recv_len ) // peer closed.
                {
                    debug_print("client socket peer closed ? \n");
                    os_mutex_lock( &handle->hmutex_clt );
                    tcp_close( list_node->sock_fd );
                    list_del( &list_node->list );
                    free( list_node );
                    os_mutex_unlock( &handle->hmutex_clt );
                }
                else
                {
                    // time out, may be block.
                    if ( get_real_time_sec() >= (long long)list_node->next_sec )
                    {
                        debug_print("client socket peer closed timeout \n");
                        os_mutex_lock( &handle->hmutex_clt );
                        tcp_close( list_node->sock_fd );
                        list_del( &list_node->list );
                        free( list_node );
                        os_mutex_unlock( &handle->hmutex_clt );
                    }
                }
            }
        }
        os_sleep_ms( 10 );
    }

    if ( NULL != buffer )
    {
        free( buffer );
    }
    if ( NULL != msg_body )
    {
        free( msg_body );
    }

    free( arg );

    return 0;
}

static boolean client_get_register_param( int8 * in_buf, int8 * sn, int8 * home_group,
                                          int8 *email, int8 * password, int8 * random )
{
    // sn | home_group | email | password | random
    boolean ret = FALSE;
    int8 * head, *tail;

    int32 count = 0;
    // sn
    head = in_buf;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( ( tail - head ) <= 12 )
        {
            strncpy( sn, head, tail - head );
            count++;
        }
    }

    // home_group
    head = tail + 1; // +1 is '|'
    tail = head;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( ( tail - head ) <= 32 )
        {
            strncpy( home_group, head, tail - head );
            count++;
        }
    }

    // email
    head = tail + 1; // +1 is '|'
    tail = head;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( NULL != strchr( head, '@' ) ) // email should contain '@'
        {
            if ( ( tail - head ) <= 32 )
            {
                strncpy( email, head, tail - head );
                count++;
            }
        }
    }

    // password
    head = tail + 1; // +1 is '|'
    tail = head;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( ( tail - head ) <= 32 )
        {
            strncpy( password, head, tail - head );
            count++;
        }
    }

    // random
    head = tail + 1; // +1 is '|'
    if ( strlen( head ) == 6 )
    {
        strcpy( random, head );
        count++;
    }

    if ( 5 == count )
    {
        ret = TRUE;
    }

    return ret;
}

static boolean client_get_login_param( int8 * in_buf, int8 *sn, int8 * password )
{
    // sn | password
    boolean ret = FALSE;
    int8 * head, *tail;

    int32 count = 0;

    // sn
    head = in_buf;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( ( tail - head ) <= 12 )
        {
            strncpy( sn, head, tail - head );
            count++;
        }
    }

    // password
    head = tail + 1; // +1 is '|'

    if ( strlen( head ) <= 32 )
    {
        strcpy( password, head );
        count++;
    }

    if ( 2 == count )
    {
        ret = TRUE;
    }

    return TRUE;
}

static boolean client_get_sn( int8 * in_buf, int8 *sn )
{
    // sn
    boolean ret = FALSE;

    if ( strlen( in_buf ) <= 12 )
    {
        strcpy( sn, in_buf );
        ret = TRUE;
    }

    return TRUE;
}

static int32 client_get_upload_dev( int8 * in_buf, int8 *sn, int32 array_size, int8 *mac[], int8 *name[], int8 *version[] )
{
    // sn | mac1#name1#version1|mac2#name2#version2
    int8 * head, *tail;
    int32 i, count = 0;

    // sn
    head = in_buf;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( ( tail - head ) <= 32 )
        {
            strncpy( sn, head, tail - head );
        }
    }

    for ( i=0; i<array_size; i++ )
    {
        head = tail + 1; // +1 is '|'
        tail = head;
        tail = strchr( head, '#' );
        if ( NULL != tail )
        {
            if ( ( tail - head ) == 12 )
            {
                strncpy( mac[i], head, tail - head );
            }
        }

        head = tail + 1; // +1 is '#'
        tail = head;
        tail = strchr( head, '#' );
        if ( NULL != tail )
        {
            if ( ( tail - head ) <= 32 )
            {
                strncpy( name[i], head, tail - head );
            }
        }

        head = tail + 1; // +1 is '#'
        tail = head;
        tail = strchr( head, '|' );
        if ( NULL != tail )
        {
            if ( ( tail - head ) <= 8 )
            {
                strncpy( version[i], head, tail - head );
                count++;
            }
        }
        else
        {
            if ( strlen( head ) <= 8 ) // last section.
            {
                strcpy( version[i], head );
                count++;
                debug_print("total get %d upload device \n", count);
                break;
            }
        }
    }

    return count;
}

static boolean client_get_sn_mac( int8 * in_buf, int8 *sn, int8 * mac )
{
    // sn | mac
    boolean ret = FALSE;
    int8 * head, *tail;

    int32 count = 0;

    // sn
    head = in_buf;
    tail = strchr( in_buf, '|' );
    if ( NULL != tail )
    {
        if ( ( tail - head ) <= 12 )
        {
            strncpy( sn, head, tail - head );
            count++;
        }
    }

    // mac
    head = tail + 1; // +1 is '|'
    if ( 12 == strlen( head ) )
    {
        strcpy( mac, head );
        count++;
    }
    else
    {
        debug_print("get mac failed \n");
    }

    if ( 2 == count )
    {
        ret = TRUE;
    }

    return TRUE;
}