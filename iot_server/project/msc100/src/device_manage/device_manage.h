#ifndef __CLIENT_MANAGER_H
#define __CLIENT_MANAGER_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define PROXY_SERVER_IP     "192.168.10.1"
#define MAX_CLIENTS         (200000)

typedef void (* dump_callback)(void *arg, char *buffer, int length);

typedef struct _DEVICE_MGR_OBJECT* DEVICE_MGR_HANDLE;

DEVICE_MGR_HANDLE device_mgr_create(int max_client_count);
int  device_mgr_set_callback(DEVICE_MGR_HANDLE handle, dump_callback cb, void *arg);
int  device_mgr_add(DEVICE_MGR_HANDLE handle, char *id, int sock_fd, unsigned int hash_value);
int  device_mgr_id_is_valid(DEVICE_MGR_HANDLE handle, char *id);
int  device_mgr_get_proxy_server_addr(DEVICE_MGR_HANDLE handle, char *id,  char *ip);
void device_mgr_keep_alive(DEVICE_MGR_HANDLE handle, char *id);
void device_mgr_flush(DEVICE_MGR_HANDLE handle);
int  device_mgr_remove(DEVICE_MGR_HANDLE handle, char *id);
int  device_mgr_get_count(DEVICE_MGR_HANDLE handle);
int  device_mgr_get_sock_fd(DEVICE_MGR_HANDLE handle, char *id, int *sock_fd);
void device_mgr_dump(DEVICE_MGR_HANDLE handle);
void device_mgr_destroy(DEVICE_MGR_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif //__CLIENT_MANAGER_H
