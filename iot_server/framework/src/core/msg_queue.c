#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "msg_queue.h"

#define CACHE_BUF_NUM   (50)    /* 可以缓存XX条消息 */

typedef struct _RECV_BUF_NODE
{
    struct list_head list;
    int    token;                       /* 是否被占用 */
    char   buffer[MAX_JSON_STRING_LEN];
} RECV_BUF_NODE;

typedef struct _MSG_QUEUE_OBJECT
{
    struct  list_head head;
    pthread_mutex_t mutex;
} MSG_QUEUE_OBJECT;

MSG_QUEUE_HANDLE msg_queue_create(void)
{
    int i;
    RECV_BUF_NODE * buf_node  = NULL;
    MSG_QUEUE_OBJECT * handle = NULL;
    handle = (MSG_QUEUE_OBJECT *)calloc(1, sizeof(MSG_QUEUE_OBJECT));
    if (NULL == handle)
    {
        media_mgmt_err("not enough memory \n");
        return NULL;
    }

    INIT_LIST_HEAD(&handle->head);
    for (i=0; i<CACHE_BUF_NUM; i++)
    {
        buf_node = (RECV_BUF_NODE *)calloc(1, sizeof(RECV_BUF_NODE));
        if (NULL !=buf_node)
        {
            list_add_tail(&buf_node->list, &handle->head);
        }
    }
    pthread_mutex_init(&handle->mutex, NULL);

    return handle;

create_failed:
    msg_queue_destroy(handle);
    return NULL;
}

void msg_queue_destroy(MSG_QUEUE_HANDLE handle)
{
    if (NULL != handle)
    {
        struct list_head *pos = NULL;
        struct list_head *n   = NULL;
        RECV_BUF_NODE * node  = NULL;

        list_for_each_safe(pos, n, &handle->head)
        {
            node = list_entry(pos, RECV_BUF_NODE, list);
            if (NULL != node)
            {
                list_del(&node->list);
                free(node);
            }
        }
        pthread_mutex_destroy(&handle->mutex);

        free(handle);
    }
}

int msg_queue_pull(MSG_QUEUE_HANDLE handle, char * buf, int buf_len)
{
    int ret = -1;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    RECV_BUF_NODE * node  = NULL;

    if ((NULL == buf) || (buf_len < MAX_JSON_STRING_LEN))
    {
        media_mgmt_err("invalid param\n");
        return -1;
    }

    pthread_mutex_lock(&handle->mutex);
    list_for_each_safe(pos, n, &handle->head)
    {
        node = list_entry(pos, RECV_BUF_NODE, list);
        if ((NULL != node) && (1 == node->token))
        {
            memcpy(buf, node->buffer, buf_len);
            memset(node->buffer, 0, sizeof(node->buffer));
            node->token  = 0;
            ret = 0;
            break;
        }
    }
    pthread_mutex_unlock(&handle->mutex);

    return ret;
}

int msg_queue_push(MSG_QUEUE_HANDLE handle, char * buf, int buf_len)
{
    int ret = -1;
    if ((NULL == handle) || (NULL == buf) || (buf_len >= MSG_QUEUE_BUF_LEN) ||
        (buf_len <= 0))
    {
        media_mgmt_err("invalid param!\n");
        return -1;
    }
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;
    RECV_BUF_NODE * node  = NULL;

    pthread_mutex_lock(&handle->mutex);
    list_for_each_safe(pos, n, &handle->head)
    {
        node = list_entry(pos, RECV_BUF_NODE, list);
        if (0 == node->token)
        {
            memset(node->buffer, 0, sizeof(node->buffer));
            memcpy(node->buffer, buf, buf_len);
            node->token  = 1;
            ret = 0;
            break;
        }
    }
    pthread_mutex_unlock(&handle->mutex);

    return ret;
}
