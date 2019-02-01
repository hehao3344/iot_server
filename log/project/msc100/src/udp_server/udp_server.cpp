#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <core/core.h>

#include <core/core.h>
#include <json/platform.h>
#include <json/jsonparse.h>
#include <json/jsontree.h>
#include <json/c_types.h>

#include "device_manage/device_manage.h"
#include "device_manage/visit_manage.h"
#include "msg_handle/msg_define.h"
#include "msg_handle/msg_handle.h"
#include "udp_server.h"

#define  MAX_MSG_LEN  1024

typedef struct PeerMsgParam
{
    int8   peer_ip[16];
    uint16 peer_port;
    int32  sock_fd;
    int32  msg;
    int8 * buf;
    int8 * msg_body;
    int8 * tmp_buf;
} PeerMsgParam;

typedef struct UdpServerObject
{
    int32 client_sock_fd;
    int32 device_sock_fd;
    int32 proxy_sock_fd;

    int32 client_port;
    int32 proxy_port;
    DEVICE_MGR_HANDLE hdev_mgr;
    VISIT_MGR_HANDLE  hvisit_mgr;

    THREAD_HANDLE hthread_register;
    THREAD_HANDLE hthread_transfer;
    THREAD_HANDLE hthread_client_msg;
    THREAD_HANDLE hthread_client_flush;
    THREAD_HANDLE hthread_visit_flush;
} UdpServerObject;

static void * register_center( void * arg );
static void * proxy_center( void * arg );
static void * client_msg_center( void * arg );
static void * flush_client_center( void * arg );
static void * flush_visit_center( void * arg );

static UdpServerObject * instance( void );
void ICACHE_FLASH_ATTR   json_parse(struct jsontree_context *json, char *ptrJSONMessage);

UDP_SERVER_HANDLE udp_server_create( void )
{
    int32 port = 0;
    UdpServerObject *handle = instance();
    if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // create and bind 3 socket.
    port = 7010;
    handle->device_sock_fd = udp_open_bind( port );
    if ( handle->device_sock_fd <= 0 )
    {
        debug_error("socket create failed \n");
        return NULL;
    }

    port = 7012;
    handle->client_sock_fd = udp_open_bind( port );
    if ( handle->client_sock_fd <= 0 )
    {
        debug_error("socket create failed \n");
        return NULL;
    }
    handle->client_port = port;

    port = 7020;
    handle->proxy_sock_fd = udp_open_bind( port );
    if ( handle->proxy_sock_fd <= 0 )
    {
        debug_error("socket create failed \n");
        return NULL;
    }
    handle->proxy_port = port;

    handle->hdev_mgr = device_mgr_create( MAX_CLIENTS );
    if ( NULL == handle->hdev_mgr )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->hvisit_mgr = visit_mgr_create();
    if ( NULL == handle->hvisit_mgr )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    // create 4 thread.
    if ( 0 != os_create_thread( &handle->hthread_register,
                                NULL,
                                ( sp_thread_func_t )register_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_transfer,
                                NULL,
                                ( sp_thread_func_t )proxy_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_client_msg,
                                NULL, (sp_thread_func_t)client_msg_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_client_flush,
                                NULL,
                                (sp_thread_func_t)flush_client_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    if ( 0 != os_create_thread( &handle->hthread_visit_flush,
                                NULL,
                                (sp_thread_func_t)flush_visit_center,
                                ( void * )handle ) )
    {
        debug_error( "pthread_create error \n");
        return NULL;
    }

    return handle;
}

void udp_server_dump_client( UDP_SERVER_HANDLE handle )
{
    device_mgr_dump( handle->hdev_mgr );
}

void udp_server_destroy( UDP_SERVER_HANDLE handle )
{
    if ( handle->client_sock_fd > 0 )
    {
        udp_close( handle->client_sock_fd );
    }
    if ( handle->device_sock_fd > 0 )
    {
        udp_close( handle->device_sock_fd );
    }
    if ( handle->proxy_sock_fd > 0 )
    {
        udp_close( handle->proxy_sock_fd );
    }

    if ( NULL != handle->hdev_mgr )
    {
        device_mgr_destroy( handle->hdev_mgr );
    }

    os_close_thread( handle->hthread_register );
    os_close_thread( handle->hthread_transfer );
    os_close_thread( handle->hthread_client_msg );
    os_close_thread( handle->hthread_client_flush );

    device_mgr_destroy( handle->hdev_mgr );
    visit_mgr_destroy( handle->hvisit_mgr );

    free( handle );
}

///////////////////////////////////////////////////////////////////////////////
// static function.
///////////////////////////////////////////////////////////////////////////////
// message is come from device.
LOCAL int ICACHE_FLASH_ATTR register_keep_alive_set( struct jsontree_context *js_ctx, struct jsonparse_state *parser )
{
    int type;

    UdpServerObject *handle = instance();
    if ( NULL == handle )
    {
        return 0;
    }

    while ( ( type = jsonparse_next( parser ) ) != 0 )
    {
        if ( type == JSON_TYPE_PAIR_NAME)
        {
            if ( jsonparse_strcmp_value( parser, "id" ) == 0 )
            {
                int8 id[24];
                os_memset( id, 0, sizeof( id ) );
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value( parser, (char*)id, sizeof( id ) );

                PeerMsgParam *msg = ( PeerMsgParam * )js_ctx->arg;

                if ( MSG_REGISTER == msg->msg )
                {
                    int8 *json_msg = NULL;
                    if ( device_mgr_add( handle->hdev_mgr, id, msg->peer_ip, msg->peer_port, 0, 0 ) )
                    {
                        json_msg = (int8*)"{\n\"id\":\"success\"\n}\n";
                        debug_print("device: [%s] addr: [%s:%d] register.\n", id, msg->peer_ip, msg->peer_port );

                    }
                    else
                    {
                        json_msg = (int8*)"{\n\"id\":\"unexist\"\n}\n";
                        debug_print("id:[%s] unexist \n", id );
                    }

                    uint16 out_len  = MAX_MSG_LEN;
                    memset( msg->buf, 0, MAX_MSG_LEN );
                    if ( msg_handle_packet( msg->buf, &out_len, 1, MSG_REGISTER_SUCCESS, json_msg  ) )
                    {
                        udp_sendto( msg->sock_fd, msg->peer_ip, msg->peer_port, msg->buf, out_len );
                    }
                }
                else if ( MSG_KEEP_ALIVE == msg->msg )
                {
                    // debug_print("keep alive \n");
                    device_mgr_keep_alive( handle->hdev_mgr, id );
                }
            }
        }
    }

    return 0;
}

LOCAL struct jsontree_callback register_keep_alive_callback =
    JSONTREE_CALLBACK( NULL, register_keep_alive_set );

JSONTREE_OBJECT( register_keep_alive_tree,
                 JSONTREE_PAIR("id", &register_keep_alive_callback) );


static void * register_center( void * arg )
{
    int32  length;
    int8   peer_ip[16];
    uint16 peer_port;
    int8   *buffer   = NULL;
    int8   *msg_body = NULL;
    uint16 ns, msg, out_len;

    buffer = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == buffer )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    msg_body = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == msg_body )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    UdpServerObject *handle = ( UdpServerObject * )arg;
    while( 1 )
    {
        memset( buffer,  0,  MAX_MSG_LEN );
        memset( peer_ip, 0, sizeof( peer_ip ) );
        length = udp_recvfrom_timeout( handle->device_sock_fd,
                                       peer_ip,
                                       &peer_port,
                                       buffer,
                                       MAX_MSG_LEN,
                                       1000 );
        if ( length > 0 )
        {
            ns = msg = 0;
            out_len  = MAX_MSG_LEN;
            memset( msg_body, 0, MAX_MSG_LEN );
            if ( msg_handle_unpacket( buffer, length, &ns, &msg, &out_len, msg_body ) )
            {
                if ( ( 1 == ns ) && ( MSG_REGISTER == msg ) )
                {
                    struct jsontree_context js;
                    PeerMsgParam peer_msg_param;

                    os_memset( &peer_msg_param, 0, sizeof( PeerMsgParam ) );
                    os_strcpy( peer_msg_param.peer_ip, peer_ip );
                    peer_msg_param.peer_port = peer_port;
                    peer_msg_param.buf       = buffer;
                    peer_msg_param.sock_fd   = handle->device_sock_fd;
                    peer_msg_param.msg       = MSG_REGISTER;

                    js.arg = &peer_msg_param;
                    jsontree_setup( &js, (struct jsontree_value *)&register_keep_alive_tree, NULL );
                    json_parse( &js, (char*)msg_body );
                }
                else if ( ( 1 == ns ) && ( MSG_KEEP_ALIVE == msg ) )
                {
                    struct jsontree_context js;
                    PeerMsgParam peer_msg_param;

                    os_memset( &peer_msg_param, 0, sizeof( PeerMsgParam ) );
                    os_strcpy( peer_msg_param.peer_ip, peer_ip );
                    peer_msg_param.peer_port = peer_port;
                    peer_msg_param.buf       = buffer;
                    peer_msg_param.sock_fd   = handle->device_sock_fd;
                    peer_msg_param.msg       = MSG_KEEP_ALIVE;

                    js.arg = &peer_msg_param;
                    jsontree_setup( &js, (struct jsontree_value *)&register_keep_alive_tree, NULL );
                    json_parse( &js, (char*)msg_body );
                }
            }
        }
    }

    free( buffer );
    free( msg_body );

    return NULL;
}

LOCAL int ICACHE_FLASH_ATTR proxy_keep_alive_set( struct jsontree_context *js_ctx, struct jsonparse_state *parser )
{
    int type;

    UdpServerObject *handle = instance();
    if ( NULL == handle )
    {
        return 0;
    }

    while ( ( type = jsonparse_next( parser ) ) != 0 )
    {
        if ( type == JSON_TYPE_PAIR_NAME)
        {
            if ( jsonparse_strcmp_value( parser, "id" ) == 0 )
            {
                int8 id[24];
                os_memset( id, 0, sizeof( id ) );
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value( parser, (char*)id, sizeof( id ) );
                // printf("proxy get id: %s \n", id );
                PeerMsgParam *msg = ( PeerMsgParam * )js_ctx->arg;
                if ( MSG_KEEP_ALIVE == msg->msg )
                {
                    device_mgr_update_proxy_addr( handle->hdev_mgr, id, msg->peer_ip, msg->peer_port );
                }
            }
        }
    }

    return 0;
}

LOCAL struct jsontree_callback proxy_keep_alive_callback =
    JSONTREE_CALLBACK( NULL, proxy_keep_alive_set );

JSONTREE_OBJECT( proxy_keep_alive_tree,
                 JSONTREE_PAIR("id", &proxy_keep_alive_callback) );

// proxy from device->proxy[server]->client.
LOCAL int ICACHE_FLASH_ATTR transfer_set( struct jsontree_context *js_ctx, struct jsonparse_state *parser )
{
    int32 type;
    int8  ip[10][16];
    uint16 port[10];
    uint32 i, array_size;

    UdpServerObject *handle = instance();
    if ( NULL == handle )
    {
        return 0;
    }

    array_size = 10;
    while ( ( type = jsonparse_next( parser ) ) != 0 )
    {
        if ( type == JSON_TYPE_PAIR_NAME)
        {
            if ( jsonparse_strcmp_value( parser, "id" ) == 0 )
            {
                int8 id[24];

                jsonparse_next(parser);
                jsonparse_next(parser);
                os_memset( id, 0, sizeof( id ) );
                jsonparse_copy_value( parser, (char*)id, sizeof( id ) );

                if ( visit_mgr_get_address( handle->hvisit_mgr, id, &array_size, ip, port ) )
                {
                    PeerMsgParam *msg = ( PeerMsgParam * )js_ctx->arg;

                    uint16 out_len = MAX_MSG_LEN;
                    if ( msg_handle_packet( msg->buf, &out_len, 2, MSG_MSC_PROXY, msg->msg_body  ) )
                    {
                        // debug_print("proxy [device to client]msg to: [%s:%d] \n", ip, port );
                        // debug_print("msg is %s \n", msg->buf+8 );
                        for ( i=0; i<array_size; i++ )
                        {
                            debug_print("send proxy to %s:%d \n", ip[i], port[i] );
                            udp_sendto( msg->sock_fd, ip[i], port[i], msg->buf, out_len );
                            visit_mgr_remove( handle->hvisit_mgr, id, ip[i], port[i] );
                        }
                    }
                }
            }
        }
    }

    return 0;
}

LOCAL struct jsontree_callback transfer_callback =
    JSONTREE_CALLBACK( NULL, transfer_set );

JSONTREE_OBJECT( transfer_tree,
                 JSONTREE_PAIR( "id",   &transfer_callback ) );


// message is come from device.
static void * proxy_center( void * arg )
{
    int32  length;
    int8   peer_ip[16];
    uint16 peer_port;
    int8 *buffer   = NULL;
    int8 *msg_body = NULL;
    int8 *tmp_buf  = NULL;
    uint16 ns, msg, out_len;

    buffer = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == buffer )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    tmp_buf = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == tmp_buf )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    msg_body = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == msg_body )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    UdpServerObject *handle = ( UdpServerObject * )arg;
    while( 1 )
    {
        memset( buffer, 0, MAX_MSG_LEN );
        memset( peer_ip, 0, sizeof( peer_ip ) );
        length = udp_recvfrom_timeout( handle->proxy_sock_fd,
                                       peer_ip,
                                       &peer_port,
                                       buffer,
                                       MAX_MSG_LEN,
                                       1000 );
        if ( length > 0 )
        {
            ns = msg = 0;
            out_len  = MAX_MSG_LEN;
            if ( msg_handle_unpacket( buffer, length, &ns, &msg, &out_len, msg_body ) )
            {
                if ( ( 2 == ns ) && ( MSG_MSC_PROXY == msg ) ) // 2 is proxy namespace.
                {
                    struct jsontree_context js;
                    PeerMsgParam peer_msg_param;

                    os_memset( &peer_msg_param, 0, sizeof( PeerMsgParam ) );
                    //os_strcpy( peer_msg_param.peer_ip, peer_ip );
                    //peer_msg_param.peer_port = peer_port;
                    peer_msg_param.buf       = buffer;
                    peer_msg_param.msg_body  = msg_body;
                    os_memset( tmp_buf, 0, MAX_MSG_LEN );
                    peer_msg_param.tmp_buf   = tmp_buf;
                    peer_msg_param.sock_fd   = handle->client_sock_fd;

                    js.arg = &peer_msg_param;
                    jsontree_setup( &js, (struct jsontree_value *)&transfer_tree, NULL );
                    json_parse( &js, (char*)msg_body );
                }
                else if ( ( 1 == ns ) && ( MSG_KEEP_ALIVE == msg ) )
                {
                    struct jsontree_context js;
                    PeerMsgParam peer_msg_param;

                    os_memset( &peer_msg_param, 0, sizeof( PeerMsgParam ) );
                    os_strcpy( peer_msg_param.peer_ip, peer_ip );
                    peer_msg_param.peer_port = peer_port;
                    peer_msg_param.buf       = buffer;
                    peer_msg_param.msg       = MSG_KEEP_ALIVE;

                    js.arg = &peer_msg_param;
                    jsontree_setup( &js, (struct jsontree_value *)&proxy_keep_alive_tree, NULL );
                    json_parse( &js, (char*)msg_body );
                }
            }
        }
    }
    free( buffer );
    free( tmp_buf );
    free( msg_body );

    return NULL;
}

LOCAL int ICACHE_FLASH_ATTR client_set( struct jsontree_context *js_ctx, struct jsonparse_state *parser )
{
    int type;

    UdpServerObject *handle = instance();
    if ( NULL == handle )
    {
        return 0;
    }

    while ( ( type = jsonparse_next( parser ) ) != 0 )
    {
        if ( type == JSON_TYPE_PAIR_NAME)
        {
            if ( jsonparse_strcmp_value( parser, "id" ) == 0 )
            {
                int8 id[24];
                int8 ip[16];
                uint16 port;

                os_memset( id, 0, sizeof( id ) );
                jsonparse_next(parser);
                jsonparse_next(parser);
                jsonparse_copy_value( parser, (char*)id, sizeof( id ) );
                //debug_print("client get id: %s \n", id );
                PeerMsgParam *msg = ( PeerMsgParam * )js_ctx->arg;
                os_memset( ip, 0, sizeof( ip ) );
                if ( device_mgr_get_proxy_address( handle->hdev_mgr, id, ip, &port ) )
                {
                    visit_mgr_add( handle->hvisit_mgr, id, msg->peer_ip, msg->peer_port );

                    uint16 out_len = MAX_MSG_LEN;
                    if ( msg_handle_packet( msg->buf, &out_len, 2, MSG_MSC_PROXY, msg->msg_body  ) )
                    {
                        debug_print("proxy msg: [%s:%d] len: [%d] \n", ip, port, out_len );
                        udp_sendto( msg->sock_fd, ip, port, msg->buf, out_len );
                    }
                }
            }
        }
    }

    return 0;
}

LOCAL struct jsontree_callback client_callback =
    JSONTREE_CALLBACK( NULL, client_set );

JSONTREE_OBJECT( client_tree,
                 JSONTREE_PAIR("id", &client_callback) );

static void * client_msg_center( void * arg )
{
    int32  length;
    uint16 peer_port;
    int8   peer_ip[16];

    int8 *buffer   = NULL;
    int8 *msg_body = NULL;
    int8 *tmp_buf  = NULL;
    uint16 ns, msg, out_len;
    buffer = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == buffer )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    msg_body = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == msg_body )
    {
        debug_error("not enough memory \n");
        return NULL;
    }
    tmp_buf = ( int8 * )malloc( MAX_MSG_LEN );
    if ( NULL == tmp_buf )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    UdpServerObject *handle = ( UdpServerObject * )arg;
    while( 1 )
    {
        memset( buffer, 0, MAX_MSG_LEN );
        memset( peer_ip, 0, sizeof( peer_ip ) );
        length = udp_recvfrom_timeout( handle->client_sock_fd,
                                       peer_ip,
                                       &peer_port,
                                       buffer,
                                       MAX_MSG_LEN,
                                       1000 );
        if ( length > 0 )
        {
            ns = msg = 0;
            out_len  = MAX_MSG_LEN;
            if ( msg_handle_unpacket( buffer, length, &ns, &msg, &out_len, msg_body ) )
            {
                if ( ( 2 == ns ) && ( MSG_MSC_PROXY == msg ) )
                {
                    struct jsontree_context js;
                    PeerMsgParam peer_msg_param;

                    os_memset( &peer_msg_param, 0, sizeof( PeerMsgParam ) );
                    os_strcpy( peer_msg_param.peer_ip, peer_ip );
                    peer_msg_param.peer_port = peer_port;
                    peer_msg_param.buf       = buffer;
                    peer_msg_param.msg_body  = msg_body;
                    peer_msg_param.tmp_buf   = tmp_buf;
                    peer_msg_param.sock_fd   = handle->proxy_sock_fd;

                    js.arg = &peer_msg_param;
                    jsontree_setup( &js, (struct jsontree_value *)&client_tree, NULL );
                    json_parse( &js, (char*)msg_body );
                }
            }
        }
    }

    free( tmp_buf );
    free( msg_body );
    free( buffer );

    return NULL;
}

static void * flush_client_center( void * arg )
{
    UdpServerObject *handle = ( UdpServerObject * )arg;
    while( 1 )
    {
        // about 60s update.
        device_mgr_flush( handle->hdev_mgr, 6 );
        os_sleep_sec( 10 );
    }

    return NULL;
}

static void * flush_visit_center( void * arg )
{
    UdpServerObject *handle = ( UdpServerObject * )arg;
    while( 1 )
    {
        visit_mgr_flush( handle->hvisit_mgr );
        os_sleep_sec( 2 );
    }

    return NULL;
}


static UdpServerObject * instance( void )
{
    static UdpServerObject *handle = NULL;
    if ( NULL == handle )
    {
        handle = ( UdpServerObject * )os_malloc( sizeof( UdpServerObject ) );
        os_memset( handle, 0, sizeof( UdpServerObject ) );
    }

    return handle;
}

/******************************************************************************
 * FunctionName : json_parse
 * Description  : parse the data as a JSON format
 * Parameters   : js_ctx -- A pointer to a JSON set up
 *                ptrJSONMessage -- A pointer to the data
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR json_parse(struct jsontree_context *json, char *ptrJSONMessage)
{
    /* Set value */
    struct jsontree_value *v;
    struct jsontree_callback *c;
    struct jsontree_callback *c_bak = NULL;

    while ((v = jsontree_find_next(json, JSON_TYPE_CALLBACK)) != NULL) {
        c = (struct jsontree_callback *)v;

        if (c == c_bak) {
            continue;
        }

        c_bak = c;

        if (c->set != NULL) {
            struct jsonparse_state js;

            jsonparse_setup(&js, ptrJSONMessage, os_strlen(ptrJSONMessage));
            c->set(json, &js);
        }
    }
}

boolean json_msg_is_valid( int8 *text )
{
    boolean ret = FALSE;
	cJSON *json = NULL;

	json = cJSON_Parse( text );
	if ( NULL == json )
    {
        debug_print("Error before: [%s]\n",cJSON_GetErrorPtr());
    }
	else
	{
	    ret = TRUE;
		cJSON_Delete(json);
	}
}
