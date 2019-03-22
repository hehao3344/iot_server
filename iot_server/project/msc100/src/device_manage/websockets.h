#ifndef __WEBSOCKETS_H
#define __WEBSOCKETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

typedef struct _WEBSOCKETS_OBJECT * WEBS_HANDLE;

typedef void (* data_receive_cb)(void * arg, char * buffer, int len);

WEBS_HANDLE websockets_create(int bind_port);
void        websockets_set_recv_callback(data_receive_cb cb, void * arg);

void        websockets_destroy(WEBS_HANDLE handle);
void        websockets_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif // __WEBSOCKETS_H
