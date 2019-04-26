#ifndef __WS_H
#define __WS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

#define PORT_DEV    8020
#define PORT_CLT    8080

typedef struct _WS_OBJECT * WS_HANDLE;

WS_HANDLE ws_create(void);
void ws_destroy(WS_HANDLE handle);
char * ws_calculate_accept_key(WS_HANDLE handle, const char * buffer);
char * ws_handle_payload_data(WS_HANDLE handle, const char *buffer, const int buf_len);
char * ws_construct_packet_data(WS_HANDLE handle, const char *message, unsigned long *len);

#ifdef __cplusplus
}
#endif

#endif // __WS_H
