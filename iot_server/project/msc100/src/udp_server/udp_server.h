#ifndef __UDP_SERVER_H
#define __UDP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>
#include <device_manage/device_manage.h>

typedef struct UdpServerObject * UDP_SERVER_HANDLE;

UDP_SERVER_HANDLE udp_server_create( void );
void udp_server_dump_client( UDP_SERVER_HANDLE handle );
void udp_server_destroy( UDP_SERVER_HANDLE handle );

#ifdef __cplusplus
}
#endif

#endif // __UDP_SERVER_H
