#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "visit_manage.h"

typedef struct ClientNode
{
    struct list_head list;
    int8   id[24];
    uint32 hash_value;
    int32  sock_fd;
    uint32 next_sec;
} ClientNode;

typedef struct VisitMgrObject
{
    struct  list_head head;  // ClientNode head;
    MUTEX_HANDLE      hmutex;
}VisitMgrObject;

VISIT_MGR_HANDLE visit_mgr_create( void )
{
    VisitMgrObject *handle = ( VisitMgrObject * )calloc( 1, sizeof( VisitMgrObject ) );
	if ( NULL == handle )
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    INIT_LIST_HEAD( &handle->head );
    os_mutex_open( &handle->hmutex, NULL );

    return handle;
}

boolean visit_mgr_add( VISIT_MGR_HANDLE handle, int8 *id, int32 sock_fd, uint32 hash_value )
{
    if ( 20 != strlen( id ) )
    {
        debug_error( "id invalid \n" );
        return FALSE;
    }

    boolean node_exist = FALSE;

    ClientNode* clt_list  = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            if ( ( 0 == strcmp( clt_list->id, id ) ) &&
                 ( clt_list->hash_value == hash_value ) )
            {
                clt_list->next_sec = get_real_time_sec() + 30;
                node_exist = TRUE;
                break;
            }
        }
    }

    if ( !node_exist )
    {
        ClientNode* clt_node = ( ClientNode * )calloc( 1, sizeof( ClientNode ) );
        if ( NULL == clt_node )
        {
            debug_print("not enough memory \n");
            return FALSE;
        }

        strcpy( clt_node->id, id );
        clt_node->hash_value = hash_value;
        clt_node->sock_fd    = sock_fd;
        clt_node->next_sec   = get_real_time_sec() + 30;
        os_mutex_lock( &handle->hmutex );
        list_add_tail( &clt_node->list, &handle->head );
        os_mutex_unlock( &handle->hmutex );
    }

    return TRUE;
}

void visit_mgr_keep_alive( VISIT_MGR_HANDLE handle, int8 *id, uint32 hash_value )
{
    ClientNode* clt_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            if ( hash_value == clt_list->hash_value ) // we do not compare id here for speed up.
            {
                os_mutex_lock( &handle->hmutex );
                clt_list->next_sec = get_real_time_sec() + 30;
                os_mutex_unlock( &handle->hmutex );
            }
        }
    }
}

boolean visit_mgr_get_sock_fd( VISIT_MGR_HANDLE handle, int8 *id, uint32 *array_size, int32 sock_fd[10], uint32 hash_value[10] )
{
    uint32  cur = 0;
    boolean ret = FALSE;

    ClientNode* clt_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            if ( ( 0 == strcmp( clt_list->id, id ) ) && ( cur < *array_size ) )
            {
                os_mutex_lock( &handle->hmutex );
                sock_fd[cur]    = clt_list->sock_fd;
                hash_value[cur] = clt_list->hash_value;
                os_mutex_unlock( &handle->hmutex );
                cur++;
            }
        }
    }

    *array_size = cur;
    if ( cur > 0 )
    {
        ret = TRUE;
    }

	return ret;
}

void visit_mgr_remove( VISIT_MGR_HANDLE handle, int8 *id, uint32 hash_value )
{
    ClientNode* clt_list  = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            if ( ( 0 == strcmp( clt_list->id, id ) ) &&
                 ( clt_list->hash_value = hash_value ) )
            {
                os_mutex_lock( &handle->hmutex );
                list_del( &clt_list->list );
                free( clt_list );
                os_mutex_unlock( &handle->hmutex );
            }
        }
    }
}

void visit_mgr_flush( VISIT_MGR_HANDLE handle )
{
    ClientNode* clt_list  = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    uint32 cur_sec = get_real_time_sec();
    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            if ( clt_list->next_sec < cur_sec  )
            {
                os_mutex_lock( &handle->hmutex );
                debug_print("visit node %s exit \n", clt_list->id );
                list_del( &clt_list->list );
                free( clt_list );
                os_mutex_unlock( &handle->hmutex );
            }
        }
    }
}

uint32 visit_mgr_get_count( VISIT_MGR_HANDLE handle )
{
    uint32 count = 0;
    ClientNode* clt_list  = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            count++;
        }
    }

    return count;
}

void visit_mgr_destroy( VISIT_MGR_HANDLE handle )
{
    ClientNode* clt_list = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    list_for_each_safe( pos, n, &handle->head )
    {
        clt_list = list_entry( pos, ClientNode, list );
        if ( NULL != clt_list )
        {
            list_del( &clt_list->list );
            free( clt_list );
        }
    }
    list_del( &handle->head );
    os_mutex_close( &handle->hmutex );
    free( handle );
}
