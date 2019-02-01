#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>
#include <device_manage/device_manage.h>

#define PORT_DEV    8010
#define PORT_CLT    8020

typedef struct _TCP_SERVER_OBJECT * TCP_SERVER_HANDLE;

TCP_SERVER_HANDLE tcp_server_create( void );
void              tcp_server_destroy( TCP_SERVER_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif // __TCP_SERVER_H
