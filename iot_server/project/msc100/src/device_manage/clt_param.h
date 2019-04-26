#ifndef __CLT_PARAM_H
#define __CLT_PARAM_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_OPEN_ID_LEN     40
#define MAX_CC_ID_LEN       16

#define PROXY_SERVER_IP     "192.168.10.1"
#define MAX_CLIENTS         (20000)
#define MAX_ID_ARRAY        4


typedef struct _CC_DEV_NODE
{
    char id[MAX_ID_ARRAY][MAX_ID_LEN];
    int  on_off[MAX_ID_ARRAY];
    int  on_line[MAX_ID_ARRAY];
} CC_DEV_NODE;

typedef void (* sock_exit_callback)(void *arg, int sock_fd);

typedef void (* dump_callback)(void *arg, char *buffer, int length);

typedef struct _CLT_PARAM_OBJECT* CLT_PARAM_HANDLE;

CLT_PARAM_HANDLE dev_param_create(int max_dev_count);
int  dev_param_set_callback(CLT_PARAM_HANDLE handle, dump_callback cb, void *arg);
int  dev_param_add_connect_sock(CLT_PARAM_HANDLE handle, int sock_fd);
int  dev_param_register(CLT_PARAM_HANDLE handle, char *cc_id, int hash_value, int sock_fd);
int  dev_heart_beat(CLT_PARAM_HANDLE handle, char *cc_id);
int  dev_param_update(CLT_PARAM_HANDLE handle, char *cc_id, SUB_DEV_NODE * sub_dev);
int  dev_param_id_is_valid(CLT_PARAM_HANDLE handle, char *id);
void dev_param_set_sock_exit_cb(CLT_PARAM_HANDLE handle, sock_exit_callback cb, void *arg);
void dev_param_get_proxy_server_addr(CLT_PARAM_HANDLE handle, char *ip, int len);
void dev_param_keep_alive(CLT_PARAM_HANDLE handle, char *id);
void dev_param_flush(CLT_PARAM_HANDLE handle);
void dev_param_sock_fd_flush(CLT_PARAM_HANDLE handle);
int  dev_param_remove(CLT_PARAM_HANDLE handle, char *id);
int  dev_param_get_count(CLT_PARAM_HANDLE handle);
int  dev_param_get_sock_fd(CLT_PARAM_HANDLE handle, char *id, int *sock_fd);
void dev_param_dump(CLT_PARAM_HANDLE handle);
void dev_param_destroy(CLT_PARAM_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif //__DEV_PARAM_H
