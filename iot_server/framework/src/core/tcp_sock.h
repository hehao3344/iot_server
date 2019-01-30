#ifndef __TCP_SOCK_H
#define __TCP_SOCK_H

#include "sock.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _TCP_SOCK_OBJECT * TCP_SOCK_HANDLE;

TCP_SOCK_HANDLE tcp_sock_create(void);

/* *********************
*  成功 返回 0
*  失败 返回 -1
************************/
int  tcp_sock_open(TCP_SOCK_HANDLE handle);
void tcp_sock_close(TCP_SOCK_HANDLE handle);
void tcp_sock_set_nonblock(TCP_SOCK_HANDLE handle);
int  tcp_sock_set_block(TCP_SOCK_HANDLE handle);
int  tcp_sock_connect(TCP_SOCK_HANDLE handle, const char * ip, int port);
int  tcp_sock_connect_timeout(TCP_SOCK_HANDLE handle, const char * ip, int port, int msec);
int  tcp_sock_send(TCP_SOCK_HANDLE handle, const char * buf, int len);
int  tcp_send_timeout(TCP_SOCK_HANDLE handle, const char * buf, int len, int msec);
int  tcp_sock_recv(TCP_SOCK_HANDLE handle, char * buf, int len);
int  tcp_sock_recv_timeout(TCP_SOCK_HANDLE handle, char * buf, int len, int msec);
int  tcp_sock_get_sockfd(TCP_SOCK_HANDLE handle);
void tcp_sock_destroy(TCP_SOCK_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif

