#ifndef __WS_H
#define __WS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <core/core.h>

typedef struct _WS_OBJECT * WS_HANDLE;

typedef enum{
    WDT_MINDATA = -20,      // 0x0����ʶһ���м����ݰ�
    WDT_TXTDATA = -19,      // 0x1����ʶһ��txt�������ݰ�
    WDT_BINDATA = -18,      // 0x2����ʶһ��bin�������ݰ�
    WDT_DISCONN = -17,      // 0x8����ʶһ���Ͽ������������ݰ�
    WDT_PING = -16,         // 0x8����ʶһ���Ͽ������������ݰ�
    WDT_PONG = -15,         // 0xA����ʾһ��pong�������ݰ�
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
