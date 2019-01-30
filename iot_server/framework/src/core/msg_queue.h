#ifndef __MSG_QUEUE_H
#define __MSG_QUEUE_H

#include <core/core.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_JSON_STRING_LEN     256

typedef struct _MSG_QUEUE_OBJECT * MSG_QUEUE_HANDLE;

MSG_QUEUE_HANDLE msg_queue_create(void);

void msg_queue_destroy(MSG_QUEUE_HANDLE handle);

int msg_queue_pull(MSG_QUEUE_HANDLE handle, char * buf, int buf_len);

int msg_queue_push(MSG_QUEUE_HANDLE handle, char * buf, int buf_len);

#ifdef __cplusplus
}
#endif

#endif // __MSG_QUEUE_H

