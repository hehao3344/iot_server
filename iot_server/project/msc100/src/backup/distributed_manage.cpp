#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "device_manage/device_manage.h"
#include "device_manage/visit_manage.h"
#include "msg_handle/msg_define.h"
#include "msg_handle/msg_handle.h"

#include "distributed_manage.h"

#define MAX_CLT_TIMEOUT         10
#define MAX_RTHREAD_COUNT       50
#define MAX_CLT_IN_THREAD       100

typedef struct _DISTRIBUTED_MGR_OBJECT
{
    struct  list_head head;  // distributed head;
    // pthread_mutex_t   mutex;

    RTHREAD_HANDLE rthread_center;
    TTASK_HANDLE   ttask_center[MAX_RTHREAD_COUNT];


    MSG_QUEUE_HANDLE hmsg_queue[MAX_RTHREAD_COUNT];

    DEVICE_MGR_HANDLE hdev_mgr;
} DISTRIBUTED_MGR_OBJECT;

typedef struct _RTHREAD_PARAM
{
    int index;
    DISTRIBUTED_MGR_OBJECT *distributed_obj;
} RTHREAD_PARAM;

static void distributed_thread_center(long user_info);

DISTRIBUTED_MGR_HANDLE distributed_mgr_create(DistributedMgrEnv *env)
{
    int i;
    char rthread_name[32];
    DISTRIBUTED_MGR_OBJECT *handle = (DISTRIBUTED_MGR_OBJECT *)calloc(1, sizeof(DISTRIBUTED_MGR_OBJECT));
	if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    handle->hdev_mgr = env->hdev_mgr;

    INIT_LIST_HEAD(&handle->head);

    os_mutex_open(&handle->hmutex, NULL);

    // handle request count: 50 * 100 = 5000.
    handle->rthread_center = rthread_create();
    if (NULL == handle->rthread_center)
    {
        debug_error("rthread_create failed \n");
        goto create_failed;
    }

    RTHREAD_PARAM *thread_pool_param = NULL;
    for (i=0; i<ARRAY_SIZE(handle->ttask_center); i++)
    {
        // 0-99, 100-199, 200-299 etc, total 5000.
        thread_pool_param = (RTHREAD_PARAM*)malloc(sizeof(RTHREAD_PARAM));
        if (NULL != thread_pool_param)
        {
            memset(rthread_name, 0, sizeof(rthread_name));
            snprintf(rthread_name, "dis_center_%d", i);
            thread_pool_param->index = i;
            thread_pool_param->distributed_obj = handle;
            handle->hmsg_queue[i] = msg_queue_create();
            if (NULL == handle->hmsg_queue[i])
            {
                debug_error("msg_queue_create failed \n");
                goto create_failed;
            }

            handle->ttask_center[i] = rthread_add(handle->rthread_center,
                                                  rthread_name,
                                                  distributed_thread_center,
                                                  (long)thread_pool_param);
            if (NULL == handle->ttask_center[i])
            {
                debug_error("rthread_add failed \n");
                goto create_failed;
            }
        }
    }

    return handle;

create_failed:
    distributed_mgr_destroy(handle);
    return NULL;
}

MSG_QUEUE_HANDLE distributed_mgr_get_msg_queue(DISTRIBUTED_MGR_HANDLE handle)
{
    return handle->hmsg_queue[0];
}

void distributed_mgr_destroy(DISTRIBUTED_MGR_HANDLE handle)
{
    int i;
    DISTRIBUTED_CLT_PARAM* list_node = NULL;
    struct list_head *pos = NULL;
    struct list_head *n   = NULL;

    for (i=0; i<ARRAY_SIZE(handle->ttask_center); i++)
    {
        if (NULL != handle->rthread_center)
        {
            rthread_delete(handle->rthread_center, handle->ttask_center[i]);
            rthread_wait_exit(handle->rthread_center, handle->ttask_center[i]);
            rthread_free(handle->rthread_center, handle->ttask_center[i]);
            handle->ttask_center[i] = NULL;
        }
    }

    for (i=0; i<ARRAY_SIZE(handle->hmsg_queue); i++)
    {
        if (NULL != handle->hmsg_queue[i])
        {
            msg_queue_destroy(handle->hmsg_queue[i]);
        }
    }

    free(handle);
}

static void distributed_thread_center(long user_info)
{
    RTHREAD_PARAM *arg = (RTHREAD_PARAM *)param;

    int ret = -1;
    int index = arg->index;
    DISTRIBUTED_MGR_OBJECT *handle = arg->distributed_obj;
    char  buffer[MAX_JSON_STRING_LEN];

    DISTRIBUTED_CLT_PARAM * list_node = NULL;

    while(1)
    {
        if (1 == rthread_is_destroyed(handle->rthread_center, handle->ttask_center[index]))
        {
            control_info("system_param_center destroyed \n");
            break;
        }

        ret = msg_queue_pull(handle->ttask_center[index], buffer, sizeof(buffer));
        if (0 == ret)
        {
            /* 根据欲访问的设备ID找到设备并发送相关消息到设备 */
        }

        os_sleep_ms(10);
    }

    free(arg);

    return 0;
}
