#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/ipc.h>
#include <time.h>

#include "msg_util.h"
#include "debug_util.h"

#define MAX_IOT_DEBUG_OSS_METHOD_STRING     32
#define IOT_DEBUG_TAG_MAX_LEN               32

static int g_debug_level = IOT_DEBUG_LEVEL_INFO;
static int g_debug_module = 0;
static int g_debug_qid = -1;

char g_module_name[IOT_DEBUG_TAG_MAX_LEN] = {0};

int debug_send(int way, const char* buf)
{
    IOT_MSG send_msg;
    memset(&send_msg, 0, sizeof(IOT_MSG));
    send_msg.dst_mod = IOT_MODULE_DEBUG;
    send_msg.src_mod = g_debug_module;
    send_msg.handler_mod = way;

    int len = strlen(buf);
    if (len >= IOT_MSG_BODY_LEN_MAX)
    {
        memcpy(send_msg.body, buf, IOT_MSG_BODY_LEN_MAX-1);
    }
    else
    {
        memcpy(send_msg.body, buf, len);
    }

    send_msg.body_len = strlen((char*)send_msg.body);
    if (msg_send_nowait(g_debug_qid, &send_msg))
	{
        printf("iot_debug_send: iot_msg_send_nowait error!(%s)\n", buf);
        return 1;
    }

    return 0;
}

void debug_level_set(int level)
{
    g_debug_level = level;
}

void debug_tag_set(char *name)
{
    if (name != NULL)
    {
        strncpy(g_module_name, name, IOT_DEBUG_TAG_MAX_LEN);
    }
}

int debug_init(int module, int level, char *tag)
{
    int qid = msg_queue_get(IOT_MSG_SQUEUE_DEBUG);
    if (qid < 0)
    {
        printf("%s %x fail.\n", __func__, module);
        return -1;
    }

    g_debug_module = module;
    g_debug_qid = qid;

    debug_level_set(level);
    debug_tag_set(tag);

    return 0;
}

int debug_print(int way, const char* fmt, ...)
{
    char buf[IOT_MSG_BODY_LEN_MAX] = {0};
    int len = 0;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(buf, IOT_MSG_BODY_LEN_MAX - 1, fmt, ap);
    if (len <= 0)
    {
        va_end(ap);
        return -1;
    }

    if (len > IOT_MSG_BODY_LEN_MAX - 1 )
    {
        len = IOT_MSG_BODY_LEN_MAX - 1;
    }

    if (g_debug_level > way)
    {
        printf("%s\n", &buf[1]);
        va_end(ap);
        return 0;
    }

    if (buf[len - 1] == '\n')
    {
        buf[len - 1] = '\0';
    }

    va_end(ap);

    return debug_send(way, buf);
}


