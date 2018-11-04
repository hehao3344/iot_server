#ifndef __DISTRIBUTED_MANAGER_H
#define __DISTRIBUTED_MANAGER_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct PeerMsgParam
{
    //int8   peer_ip[16];
    //uint16 peer_port;
    //int8 * tmp_buf;
    int32  sock_fd;
    int32  msg;
    int8 * buf;
    int8 * msg_body;
    void * arg;
} PeerMsgParam;

typedef struct DistributedMgrEnv
{
    DEVICE_MGR_HANDLE hdev_mgr;
} DistributedMgrEnv;

typedef struct DistributedManageObject * DISTRIBUTED_MANAGE_HANDLE;

DISTRIBUTED_MANAGE_HANDLE distributed_mgr_create( DistributedMgrEnv *env );
boolean distributed_mgr_add( DISTRIBUTED_MANAGE_HANDLE handle, int32 sock_fd );
void    distributed_mgr_destroy( DISTRIBUTED_MANAGE_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif //__DISTRIBUTED_MANAGER_H
