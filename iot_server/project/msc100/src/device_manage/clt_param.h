#ifndef __CLT_PARAM_H
#define __CLT_PARAM_H

#include <core/core.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_OPEN_ID_ARRAY   4
#define MAX_OPEN_ID_LEN     40
#define MAX_APP_CLIENTS     (2000)

typedef void (* sock_exit_callback)(void *arg, int sock_fd);

typedef void (* dump_callback)(void *arg, char *buffer, int length);

typedef struct _CLT_PARAM_OBJECT* CLT_PARAM_HANDLE;

CLT_PARAM_HANDLE clt_param_create(int max_clt_count);
void clt_param_set_sock_exit_cb(CLT_PARAM_HANDLE handle, sock_exit_callback cb, void *arg);
int clt_param_add_connect_sock(CLT_PARAM_HANDLE handle, int sock_fd);
int clt_param_bind(CLT_PARAM_HANDLE handle,  char * dev_uuid, char * openid);
int clt_param_unbind(CLT_PARAM_HANDLE handle,  char * openid);
int clt_param_get_dev_uuid_by_openid(CLT_PARAM_HANDLE handle, char * openid, char * buf, int buf_len);
int clt_param_heart_beat(CLT_PARAM_HANDLE handle, char *gopenid, int sock_fd);
int clt_param_sock_fd_is_exist(CLT_PARAM_HANDLE handle, int sock_fd);
int clt_param_group_openid_is_valid(CLT_PARAM_HANDLE handle, char *gopenid);
int clt_param_remove(CLT_PARAM_HANDLE handle, char *gopenid);
int clt_param_get_count(CLT_PARAM_HANDLE handle);
int clt_param_get_sock_fd(CLT_PARAM_HANDLE handle, char *gopenid, int *sock_fd);
void clt_param_flush(CLT_PARAM_HANDLE handle);
void clt_param_sock_fd_flush(CLT_PARAM_HANDLE handle);
void clt_param_destroy(CLT_PARAM_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif //__DEV_PARAM_H
