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

typedef struct _DISTRIBUTED_MGR_OBJECT * DISTRIBUTED_MGR_HANDLE;

DISTRIBUTED_MANAGE_HANDLE distributed_mgr_create(DistributedMgrEnv *env);
MSG_QUEUE_HANDLE distributed_mgr_get_msg_queue(DISTRIBUTED_MGR_HANDLE handle);
void             distributed_mgr_destroy(DISTRIBUTED_MANAGE_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif //__DISTRIBUTED_MANAGER_H
