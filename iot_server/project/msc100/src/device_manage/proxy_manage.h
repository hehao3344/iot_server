#ifndef __PROXY_MANAGER_H
#define __PROXY_MANAGER_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct ProxyManageObject * PROXY_MANAGE_HANDLE;

PROXY_MANAGE_HANDLE proxy_mgr_create( void );
boolean proxy_mgr_dev_add( PROXY_MANAGE_HANDLE handle, int32 sock_fd );
boolean proxy_mgr_clt_add( PROXY_MANAGE_HANDLE handle, int32 sock_fd );
void    proxy_mgr_flush( PROXY_MANAGE_HANDLE handle );
void    proxy_mgr_destroy( PROXY_MANAGE_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif //__PROXY_MANAGER_H
