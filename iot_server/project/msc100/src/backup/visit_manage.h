#ifndef __VISIT_MANAGE_H
#define __VISIT_MANAGE_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct VisitMgrObject* VISIT_MGR_HANDLE;

VISIT_MGR_HANDLE visit_mgr_create( void );
boolean visit_mgr_add( VISIT_MGR_HANDLE handle, int8 *id, int32 sock_fd, uint32 hash_value );
boolean visit_mgr_get_sock_fd( VISIT_MGR_HANDLE handle, int8 *id, uint32 *array_size, int32 sock_fd[10], uint32 hash_value[10] );
void    visit_mgr_remove( VISIT_MGR_HANDLE handle, int8 *id, uint32 hash_value );
void    visit_mgr_keep_alive( VISIT_MGR_HANDLE handle, int8 *id, uint32 hash_value );
void    visit_mgr_flush( VISIT_MGR_HANDLE handle );
uint32  visit_mgr_get_count( VISIT_MGR_HANDLE handle );
void    visit_mgr_destroy( VISIT_MGR_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif //__VISIT_MANAGE_H
