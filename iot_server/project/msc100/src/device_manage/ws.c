#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <core/core.h>
#include <crypto/sha1.h>
#include <crypto/base64.h>
#include <crypto/int_lib.h>

#include "ws.h"

#define MAX_KEY_HASH_LEN        128
#define MAX_KEY_HASH_BASE64_LEN 256
#define PER_LINE_MAX            256		    // client key值最大长度
#define REQUEST_LEN_MAX         2048	    // request包的最大字节数
#define SEND_DATA_LEN_MAX       (2048+10)	// 发送数据buffer
#define WEB_SOCKET_KEY_LEN_MAX  256		    // websocket key值最大长度

#define RESPONSE_HEADER_LEN_MAX 2048	    // response包的最大字节数

typedef struct _WS_OBJECT
{
    char key[WEB_SOCKET_KEY_LEN_MAX];
    char client_key[PER_LINE_MAX];
    char sha1_data[MAX_KEY_HASH_LEN];
    char sha1_base64_data[MAX_KEY_HASH_BASE64_LEN];

    char payload_data[REQUEST_LEN_MAX];
    char send_data[SEND_DATA_LEN_MAX];
} WS_OBJECT;

static char * extract_client_key(WS_HANDLE handle, const char * buffer);

WS_HANDLE ws_create(void)
{
    WS_OBJECT * handle = (WS_OBJECT *)calloc(1, sizeof(WS_OBJECT));
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    return handle;
}

void ws_destroy(WS_HANDLE handle)
{
    if (NULL != handle)
    {
        free(handle);
    }
}

char * ws_calculate_accept_key(WS_HANDLE handle, const char * buffer)
{
    int i;
    int n;
    char * ret = NULL;
    char * sha1_data_tmp = NULL;
    const char  *guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    if ((NULL == buffer) || (NULL == handle))
    {
        debug_error("invalid param.\n");
        return NULL;
    }

    char * client_key_tmp = extract_client_key(handle, buffer);
    if (NULL == client_key_tmp)
    {
        debug_error("client_key is NULL.\n");
        return NULL;
    }

    memset(handle->client_key, 0, sizeof(handle->client_key));
    snprintf(handle->client_key, sizeof(handle->client_key), "%s%s", client_key_tmp, guid);

    sha1_data_tmp = sha1_hash(handle->client_key);  // sha1加密client_key
    n = strlen(sha1_data_tmp);						// 获取加密后的key值长度

    if (n/2 > sizeof(handle->sha1_data))
    {
        debug_error("sha1_data_tmp too long.\n");
        return NULL;
    }
    memset(handle->sha1_data, 0, n/2);			    // 将sha1_data重置为0

    for (i=0; i<n; i+=2)
    {
        handle->sha1_data[i/2] = htoi(sha1_data_tmp, i, 2); // 将sha1加密后的16进制key值转换为10进制
    }

    if (0 == base64_encode(handle->sha1_data, n/2, handle->sha1_base64_data, sizeof(handle->sha1_base64_data)))
    {
        ret = handle->sha1_base64_data;
    }

    return ret;
}

/* 处理从client接收过来的数据 */
char * ws_handle_payload_data(WS_HANDLE handle, const char *buffer, const int buf_len)
{
    int  i = 0;
    char masks[4];              // 数据掩码

    char temp[8];               //转换数据长度用
    int  n;                     //转换数据长度用
    int  payload_len = 0;       //数据长度

    if ((NULL == buffer) || (NULL == handle))
    {
        debug_error("invalid param.\n");
        return NULL;
    }

    if (buf_len < 2)
    {
        debug_error("too short packet length\n");
        return NULL;
    }

    int fin = (buffer[0] & 0x80) == 0x80; // 1bit，1表示最后一帧
    if (!fin)
    {
        debug_error("client is close.\n");
        return NULL;// 超过一帧暂不处理
    }

    int mask_flag = ((buffer[1] & 0x80) == 0x80); // 是否包含掩码
    if (!mask_flag)
    {
        debug_error("no mask.\n");
        return NULL;// 不包含掩码的暂不处理
    }

    payload_len = buffer[1] & 0x7F;                                 // 获取payloadLen数值
    debug_info("payload length is %d \n", payload_len);
    if (payload_len == 126)
    {
        //如果payloadLen为126则真实长度为buffer第3,4字节
        memcpy(masks, buffer+4, 4);                                 // 获取掩码(payloadLen结束后跟4字节mask)
        payload_len  = (buffer[2]&0xFF) << 8 | (buffer[3]&0xFF);    // (将buffer第3字节与0xFF进行与运算)后左移8位在进行或运算(buffer第4字节与0xFF进行与运算)

        if (payload_len <= sizeof(handle->payload_data))
        {
            memset(handle->payload_data, 0, sizeof(handle->payload_data));
            memcpy(handle->payload_data, buffer + 4 + strlen(masks),  payload_len); // 获取buffer第8(2+2+4)字节之后的内容(数据部分)
        }
    }
    else if (payload_len == 127)
    {
        //如果payloadLen为126则真实长度为buffer第3-10字节
        memcpy(masks, buffer+10, 4);                            // 获取掩码(payloadLen结束后跟4字节mask)
        for (i=0; i<8; i++)
        {
            temp[i] = buffer[9 - i];                             // 获取buffer数据长度(第3-10字节)
        }

        memcpy(&n, temp, 8);                                    // 将数据长度赋值给n

        if (n <= sizeof(handle->payload_data))
        {
            memset(handle->payload_data, 0, sizeof(handle->payload_data));
            memcpy(handle->payload_data, buffer+10+strlen(masks), n);               // 将buffer第14(2+8+4)字节之后的n字节内容赋值给payload_data
            payload_len=n;                                                  // 设置payloadLen为n
        }
    }
    else
    {
        //如果payloadLen为0-125则payloadLen为真实数据长度
        memcpy(masks, buffer+2, 4);                         //获取掩码(payloadLen结束后跟4字节mask)
        if (payload_len <= sizeof(handle->payload_data))
        {
            memset(handle->payload_data, 0, sizeof(handle->payload_data));
            memcpy(handle->payload_data, buffer+2+strlen(masks), payload_len);    //将buffer第6(2+4)字节之后的n字节内容赋值给payload_data
        }
    }

    for (i = 0; i < payload_len; i++)
    {
        handle->payload_data[i] = (char)(handle->payload_data[i] ^ masks[i % 4]);   // 将数据与掩码进行异或运算,获得原始数据
    }

    debug_print("data(%d):%s", payload_len, handle->payload_data);

    return handle->payload_data;
}

char *ws_construct_packet_data(WS_HANDLE handle, const char *message, unsigned long *len)
{
    unsigned long n;

    if (NULL == message)
    {
        //判断message是否为空
        debug_error("message is NULL.\n");
        return NULL;
    }

    n = strlen(message);
    if (n < 126)
    {
        //判断n是否小于126,小于126则payload len长度位7位
        memset(handle->send_data, 0, sizeof(handle->send_data));
        handle->send_data[0] = 0x81;     // 设置第0-7位为1000 0001(FIN为1,Opcode为1)
        handle->send_data[1] = n;        // 设置第8位为0,9-15位为n(第8位为mask,9-15位为数据长度,客户端发送mask为1,服务端发送mask为0)
        memcpy(handle->send_data+2, message, n);    // 将message添加到第2个字节之后
        *len = n+2;						// 将指针指向message首地址
    }
    else if (n < 0xFFFF)
    {
        if (n < (sizeof(handle->send_data)-4))
        {
            // 当n小于0xFFFF则为126,后2字节为真实长度
            memset(handle->send_data, 0, sizeof(handle->send_data));
            handle->send_data[0] = 0x81;					            //设置第0-7位为1000 0001(FIN为1,Opcode为1)
            handle->send_data[1] = 126;					            //设置第8-15位为0111 1110
            handle->send_data[2] = (n>>8 & 0xFF);		            //设置第16-23位为n-128(将n右移8位在与1111 1111做与运算)
            handle->send_data[3] = (n & 0xFF);			            //设置第24-31位为n的右8(0-7)位
            memcpy(handle->send_data+4, message, n);    // 将message添加到第4个字节之后
            *len=n+4;						            //将指针指向message首地址
        }
    }
    else
    {
        if (n < (sizeof(handle->send_data)-10))
        {
            // 当n大于0xFFFF则payload len前7位为127,后8字节为真实长度
            memset(handle->send_data, 0, sizeof(handle->send_data));
            handle->send_data[0] = 0x81;					//设置第0-7位为1000 0001(FIN为1,Opcode为1)
            handle->send_data[1] = 127;					//设置第8-15位为0111 1111
            handle->send_data[2] = (n>>56 & 0xFF);		//设置第16-23位为n-128(将n右移8位在与1111 1111做与运算)
    		handle->send_data[3] = (n>>48 & 0xFF);		//设置第24-31位为n-128(将n右移8位在与1111 1111做与运算)
    		handle->send_data[4] = (n>>40 & 0xFF);		//设置第32-39位为n-128(将n右移8位在与1111 1111做与运算)
    		handle->send_data[5] = (n>>32 & 0xFF);		//设置第40-47位为n-128(将n右移8位在与1111 1111做与运算)
    		handle->send_data[6] = (n>>24 & 0xFF);		//设置第48-55位为n-128(将n右移8位在与1111 1111做与运算)
    		handle->send_data[7] = (n>>16 & 0xFF);		//设置第56-63位为n-128(将n右移8位在与1111 1111做与运算)
    		handle->send_data[8] = (n>>8 & 0xFF);		//设置第64-71位为n-128(将n右移8位在与1111 1111做与运算)
            handle->send_data[9] = (n & 0xFF);			//设置第72-79位为n的右8(0-7)位
            memcpy(handle->send_data+10, message, n);   //将message添加到第10个字节之后
            *len = n+10;						            //将指针指向message首地址
        }
    }

    return handle->send_data;
}

static char * extract_client_key(WS_HANDLE handle, const char * buffer)
{
    char    *start = NULL;
    char    *flag  = "Sec-WebSocket-Key: ";
    int     i = 0;
    int     buf_len = 0;

    if ((NULL == buffer) || (NULL == handle))
    {
        debug_error("invalid param.\n");
        return NULL;
    }

    memset(handle->key, 0, sizeof(handle->key));

    start = strstr(buffer, flag);
    if (NULL == start)
    {
        debug_error("can't find start flags.\n");
        return NULL;
    }

    start   += strlen(flag);    // 将指针移至key起始位置
    buf_len = strlen(buffer);   // 获取buffer长度
    for (i=0; i<buf_len; i++)
    {
        if (start[i]==0x0A || start[i]==0x0D)
        {
            break;
        }
        handle->key[i] = start[i];
    }

    return handle->key;
}
