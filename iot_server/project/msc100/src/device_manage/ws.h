#ifndef __WS_H
#define __WS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

typedef struct _WS_OBJECT * WS_HANDLE;

typedef enum{
    WDT_MINDATA = -20,      // 0x0：标识一个中间数据包
    WDT_TXTDATA = -19,      // 0x1：标识一个txt类型数据包
    WDT_BINDATA = -18,      // 0x2：标识一个bin类型数据包
    WDT_DISCONN = -17,      // 0x8：标识一个断开连接类型数据包
    WDT_PING = -16,         // 0x8：标识一个断开连接类型数据包
    WDT_PONG = -15,         // 0xA：表示一个pong类型数据包
    WDT_ERR = -1,
    WDT_NULL = 0
} WEBSOCKET_DATA_TYPE;

WS_HANDLE ws_create(void);
char * ws_calculate_accept_key(WS_HANDLE handle, const char * buffer);
int    ws_decode_data(WS_HANDLE handle, const unsigned char *data, unsigned int data_len, unsigned char *package, unsigned int package_max_len);
//char * ws_handle_payload_data(WS_HANDLE handle, const char *buffer, const int buf_len);
char * ws_construct_packet_data(WS_HANDLE handle, const char *message, unsigned long *len);
void   ws_destroy(WS_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif // __WS_H
