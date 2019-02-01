#ifndef __CLIENT_MANAGER_H
#define __CLIENT_MANAGER_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define PROXY_SERVER_IP     "192.168.10.1"
#define MAX_CLIENTS         (200000)
#define MAX_ID_ARRAY        4
#define MAX_ID_LEN          16

typedef struct _SUB_DEV_NODE
{
    char id[MAX_ID_ARRAY][MAX_ID_LEN];
    int  on_off[MAX_ID_ARRAY];
    int  on_line[MAX_ID_ARRAY];
} SUB_DEV_NODE;

typedef void (* sock_exit_callback)(void *arg, int sock_fd);

typedef void (* dump_callback)(void *arg, char *buffer, int length);

typedef struct _DEV_PARAM_OBJECT* DEV_PARAM_HANDLE;

DEV_PARAM_HANDLE dev_param_create(int max_dev_count);
int  dev_param_set_callback(DEV_PARAM_HANDLE handle, dump_callback cb, void *arg);
int  dev_param_add_connect_sock(DEV_PARAM_HANDLE handle, int sock_fd);
int  dev_param_register(DEV_PARAM_HANDLE handle, char *cc_id, int hash_value, int sock_fd);
int  dev_param_update(DEV_PARAM_HANDLE handle, char *cc_id, SUB_DEV_NODE * sub_dev);
int  dev_param_id_is_valid(DEV_PARAM_HANDLE handle, char *id);
void dev_param_set_sock_exit_cb(DEV_PARAM_HANDLE handle, sock_exit_callback cb, void *arg);
void dev_param_get_proxy_server_addr(DEV_PARAM_HANDLE handle, char *ip, int len);
void dev_param_keep_alive(DEV_PARAM_HANDLE handle, char *id);
void dev_param_flush(DEV_PARAM_HANDLE handle);
void dev_param_sock_fd_flush(DEV_PARAM_HANDLE handle);
int  dev_param_remove(DEV_PARAM_HANDLE handle, char *id);
int  dev_param_get_count(DEV_PARAM_HANDLE handle);
int  dev_param_get_sock_fd(DEV_PARAM_HANDLE handle, char *id, int *sock_fd);
void dev_param_dump(DEV_PARAM_HANDLE handle);
void dev_param_destroy(DEV_PARAM_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif //__DEV_PARAM_H
