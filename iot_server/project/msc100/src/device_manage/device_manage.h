#ifndef __CLIENT_MANAGER_H
#define __CLIENT_MANAGER_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void ( * dump_callback )( void *arg, int8 *buffer, int32 length );

#define MAX_CLIENTS (200000)

typedef struct DeviceMgrObject* DEVICE_MGR_HANDLE;

DEVICE_MGR_HANDLE device_mgr_create( int32 max_client_count );
boolean device_mgr_set_callback( DEVICE_MGR_HANDLE handle, dump_callback cb, void *arg );
boolean device_mgr_add( DEVICE_MGR_HANDLE handle, int8 *id, int32 sock_fd, uint32 hash_value );
boolean device_mgr_id_is_valid( DEVICE_MGR_HANDLE handle, int8 *id );
boolean device_mgr_get_proxy_server_addr( DEVICE_MGR_HANDLE handle, int8 *id,  int8 *ip );
void    device_mgr_keep_alive( DEVICE_MGR_HANDLE handle, int8 *id );
void    device_mgr_flush( DEVICE_MGR_HANDLE handle );
boolean device_mgr_remove( DEVICE_MGR_HANDLE handle, int8 *id );
int32   device_mgr_get_count( DEVICE_MGR_HANDLE handle );
boolean device_mgr_get_sock_fd( DEVICE_MGR_HANDLE handle, int8 *id, int32 *sock_fd );
void    device_mgr_dump( DEVICE_MGR_HANDLE handle );
void    device_mgr_destroy( DEVICE_MGR_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif //__CLIENT_MANAGER_H
