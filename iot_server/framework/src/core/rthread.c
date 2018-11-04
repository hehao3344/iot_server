/******************************************************************************

                  版权所有 (C), 2018-2099

 ******************************************************************************
  文 件 名   : rthread.c
  版 本 号   : 初稿
  作    者   : hehao
  生成日期   : 2018年10月22日
  最近修改   :
  功能描述   : 线程池管理模块

  修改历史   :
  1.日    期   : 2018年10月22日
    作    者   : hehao
    修改内容    :  创建文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rthread.h"

/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/
#define MAX_THREAD_NAME_LEN             (64)

/* *****************************************************************************
* 实际测试结果 16 个基础线程 1个APP连接需要4个线程
* 如果按6个APP计算一共需要 6*4+16 = 40个线程
*******************************************************************************/
/* TINY thread 和普通 thread的区别是 TINY thread 线程栈更小 */
#define RTHREAD_CATEGARY_COUNT          (2)
#define THREAD_COUNT_NORMAL             (30)
#define THREAD_COUNT_TINY               (15)

#define THREAD_NORMAL_STACK_SIZE        (1024*1024)
#define THREAD_TINY_STACK_SIZE          (256*1024)

/*----------------------------------------------*
 * 常量定义                                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 类型定义                                         *
 *----------------------------------------------*/
typedef struct _THREAD_PARAM
{
    int count;
    int stack_size;
} THREAD_PARAM;

typedef struct _THREAD_TASK
{
    int is_destroyed;
	int is_runing;
    int is_exited;
    int exit_normal;    /* 正常退出 */
    char thread_name[MAX_THREAD_NAME_LEN];
    thread_func task_fun;
    void * arg;
    struct _THREAD_TASK *next;

} THREAD_TASK;

typedef struct _THREAD_POOL_PARAM
{
    THREAD_TASK *task_queue_head;
    THREAD_TASK *task_queue_end;

    int task_queue_size;
    int thread_num;
    int idle_thread_num;
    int is_pool_destroyed;
    int stack_size;
    pthread_t *thread_queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t  queue_cond;
} THREAD_POOL_PARAM;

typedef struct _RTHREAD_OBJECT
{
    /* thread handle. */
    int init;
    int pr_count;
    THREAD_POOL_PARAM tp_param[RTHREAD_CATEGARY_COUNT];
} RTHREAD_OBJECT;

/*----------------------------------------------*
 * 全局变量定义                                       *
 *----------------------------------------------*/
static THREAD_PARAM thread_param_table[RTHREAD_CATEGARY_COUNT] =
{
    {THREAD_COUNT_NORMAL,   THREAD_NORMAL_STACK_SIZE},
    {THREAD_COUNT_TINY,     THREAD_TINY_STACK_SIZE}
};

/*----------------------------------------------*
 * 内部函数声明**                                     *
 *----------------------------------------------*/
static void * thread_pool_entrance(void *arg);
static RTHREAD_OBJECT * instance(void);
static THREAD_TASK    * thread_pool_add_task(RTHREAD_HANDLE handle,
                                             int index,
                                             const char * thread_name,
                                             thread_func func, void *arg);


/*****************************************************************************
 函 数 名  : rthread_create
 功能描述  : 创建线程池管理模块
 输入参数  : void
 输出参数  : 无
 返 回 值  : 成功 非空 失败 空
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
RTHREAD_HANDLE rthread_create(void)
{
    int thread_pool_size = THREAD_COUNT_NORMAL;
    RTHREAD_OBJECT * handle = instance();
    if (NULL == handle)
    {
        debug_error("not enough memory\n");
        goto create_failed;
    }

    if (0 == handle->init)
    {
        handle->init = 1;

        int j;
        for(j=0; j<ARRAY_SIZE(handle->tp_param); j++)
        {
            handle->tp_param[j].is_pool_destroyed = 0;
            handle->tp_param[j].task_queue_head = NULL;
            handle->tp_param[j].task_queue_end  = NULL;
            handle->tp_param[j].task_queue_size = 0;

            handle->tp_param[j].thread_num   = thread_param_table[j].count;
            handle->tp_param[j].thread_queue = (pthread_t *)calloc(1, thread_pool_size * sizeof(pthread_t));
            if (NULL == handle->tp_param[j].thread_queue)
            {
                goto create_failed;
            }
            handle->tp_param[j].idle_thread_num = thread_param_table[j].count;
            handle->tp_param[j].stack_size      = thread_param_table[j].stack_size;

            pthread_mutex_init(&handle->tp_param[j].queue_mutex, NULL);
            pthread_cond_init(&handle->tp_param[j].queue_cond, NULL);

            int i, ret;
            pthread_attr_t attr;
            size_t stack_size = thread_param_table[j].stack_size;
            ret = pthread_attr_init(&attr);
            if (0 != ret)
            {
                debug_error("pthread_attr_init failed \n");
                goto create_failed;
            }
            ret = pthread_attr_setstacksize(&attr, stack_size);
            if (0 != ret)
            {
                debug_error("pthread_attr_getstacksize failed\n");
                goto create_failed;
            }

            for(i=0; i<thread_pool_size; i++)
            {
                ret = pthread_create(&handle->tp_param[j].thread_queue[i], &attr, thread_pool_entrance, (void *)&handle->tp_param[j]);
                if (ret < 0)
                {
                    debug_error("thread create error!!!\n");
                    goto create_failed;
                }
            }

            ret = pthread_attr_destroy(&attr); /*不再使用线程属性，将其销毁*/
            if (0 != ret)
            {
                debug_error("pthread_attr_getstacksize failed \n");
                goto create_failed;
            }
        }
    }
    return handle;

create_failed:
    rthread_destroy(handle);
    return NULL;
}

/*****************************************************************************
 函 数 名  : rthread_destroy
 功能描述  : 销毁线程池管理模块
 输入参数  : RTHREAD_HANDLE handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void rthread_destroy(RTHREAD_HANDLE handle)
{
    if (NULL != handle)
    {
        int j;
        for(j=0; j<ARRAY_SIZE(handle->tp_param); j++)
        {
            handle->tp_param[j].is_pool_destroyed = 1;

            pthread_cond_broadcast(&handle->tp_param[j].queue_cond);    /* 通知所有线程线程池销毁了 */
            int i;
            for (i=0; i<handle->tp_param[j].thread_num; i++)            /* 等待线程全部执行 */
            {
                pthread_join(handle->tp_param[j].thread_queue[i], NULL);
            }

            /* 销毁任务队列 */
            THREAD_TASK *temp = NULL;
            while (handle->tp_param[j].task_queue_head)
            {
                temp = handle->tp_param[j].task_queue_head;
                handle->tp_param[j].task_queue_head = handle->tp_param[j].task_queue_head->next;
                free(temp);
            }

            /* 销毁线程队列 */
            if (NULL != handle->tp_param[j].thread_queue)
            {
                free(handle->tp_param[j].thread_queue);
                handle->tp_param[j].thread_queue = NULL;
            }

            pthread_mutex_destroy(&handle->tp_param[j].queue_mutex);
            pthread_cond_destroy(&handle->tp_param[j].queue_cond);
        }
        free(handle);
    }
}

/*****************************************************************************
 函 数 名  : rthread_add
 功能描述  : 利用线程池资源开一个线程（非轻量级线程）
 输入参数  : RTHREAD_HANDLE handle
             const char * thread_name：线程名
             thread_func fun：线程执行函数
             long user_info：线程执行函数参数
 输出参数  : 无
 返 回 值  : 成功 非空 失败 空
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
TTASK_HANDLE rthread_add(RTHREAD_HANDLE handle, const char * thread_name,
                         thread_func fun, long user_info)
{
    TTASK_HANDLE task_handle = NULL;
    if ((NULL == handle) || (NULL == thread_name))
    {
        debug_error("invalid param \n");
        return NULL;
    }

    task_handle = thread_pool_add_task(handle, 0, thread_name, fun, (void *)user_info);
    if (NULL != task_handle)
    {
        control_info("start thread: %s success\n", thread_name);
    }
    else
    {
        debug_error("start thread: %s failed\n", thread_name);
    }

	return task_handle;
}

/*****************************************************************************
函 数 名  : rthread_add
功能描述  : 利用线程池资源开一个线程（轻量级线程）
          线程栈大小限制为THREAD_TINY_STACK_SIZE
输入参数  : RTHREAD_HANDLE handle
          const char * thread_name：线程名
          thread_func fun：线程执行函数
          long user_info：线程执行函数参数
输出参数  : 无
返 回 值  : 成功 非空 失败 空
调用函数  :
被调函数  :

修改历史      :
1.日    期   : 2018年10月22日
 作    者   : 何浩
 修改内容   : 新生成函数

*****************************************************************************/
TTASK_HANDLE rthread_tiny_add(RTHREAD_HANDLE handle, const char * thread_name,
                              thread_func fun, long user_info)
{
    TTASK_HANDLE task_handle = NULL;
    if ((NULL == handle) || (NULL == thread_name))
    {
        debug_error("invalid param \n");
        return NULL;
    }

    task_handle = thread_pool_add_task(handle, 1, thread_name, fun, (void *)user_info);
    if (NULL != task_handle)
    {
        control_info("start thread: %s success\n", thread_name);
    }
    else
    {
        debug_error("start thread: %s failed\n", thread_name);
    }

    return task_handle;
}

/*****************************************************************************
 函 数 名  : rthread_delete
 功能描述  : 设置退出线程
 输入参数  : RTHREAD_HANDLE handle
             TTASK_HANDLE tt_handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void rthread_delete(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle)
{
    if ((NULL == handle) || (NULL == tt_handle))
    {
        debug_error("invalid param \n");
        return;
    }

    tt_handle->is_destroyed = 1;
}

/*****************************************************************************
 函 数 名  : rthread_wait_exit
 功能描述  : 等待线程的退出
 输入参数  : RTHREAD_HANDLE handle
             TTASK_HANDLE tt_handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void rthread_wait_exit(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle)
{
    if ((NULL == handle) || (NULL == tt_handle))
    {
        debug_error("invalid param \n");
        return;
    }

    /* 如果10秒该线程还不能退出 则强制走下去 */
	int i = 200;
	while(i-- > 0)
    {
		if (1 == tt_handle->is_exited)
        {
            break;
        }
        if (0 == i%50)
        {
            control_info("waitting for thread %s exit \n", tt_handle->thread_name);
        }
        usleep(50000);
    }

    if (1 == tt_handle->is_exited)
    {
        tt_handle->exit_normal = 1;
        control_info("thread %s exit done\n", tt_handle->thread_name);
    }
    else
    {
        control_info("warnning: %s exit unnormal\n", tt_handle->thread_name);
    }

}

/*****************************************************************************
 函 数 名  : rthread_free
 功能描述  : 线程退出后该函数释放相应的资源
 输入参数  : RTHREAD_HANDLE handle
           TTASK_HANDLE tt_handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void rthread_free(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle)
{
    if ((NULL == handle) || (NULL == tt_handle))
    {
        debug_error("invalid param \n");
        return;
    }
    if (1 == tt_handle->exit_normal)
    {
        free(tt_handle);
    }
    else
    {
        debug_error("thread %s un-free memory \n", tt_handle->thread_name);
    }
}

/*****************************************************************************
 函 数 名  : rthread_is_destroyed
 功能描述  : 判断线程是否銷毀
 输入参数  : RTHREAD_HANDLE handle
             TTASK_HANDLE tt_handle
 输出参数  : 无
 返 回 值  : 已经设置销毁 1 没有设置销毁 0
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int rthread_is_destroyed(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle)
{
    if ((NULL == handle) || (NULL == tt_handle))
    {
        if (handle->pr_count++ >= 500)
        {
            handle->pr_count = 0;
            control_info("waiting for ... \n");
        }
        return 0;
    }

    return tt_handle->is_destroyed;
}

/*****************************************************************************
 函 数 名  : rthread_is_running
 功能描述  : 判断加入的线程是否已经运行起来
 输入参数  : RTHREAD_HANDLE handle
             TTASK_HANDLE tt_handle
 输出参数  : 无
 返 回 值  : 0 没有运行或者参数非法 1 运行起来了
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int rthread_is_running(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle)
{
    if ((NULL == handle) || (NULL == tt_handle))
    {
        if (NULL == handle)
        {
            debug_error("invalid param: handle \n");
        }
        else
        {
            debug_error("invalid param: tt_handle\n");
        }
        return 0;
    }

    return tt_handle->is_runing;
}

/*****************************************************************************
 函 数 名  : rthread_is_exited
 功能描述  : 判断线程是否仍然在执行 注意和 rthread_is_destroyed 的区别
 输入参数  : RTHREAD_HANDLE handle
           TTASK_HANDLE tt_handle
 输出参数  : 无
 返 回 值  : 0 当前没有运行线程执行函数 1 当前在运行线程执行函数
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int rthread_is_exited(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle)
{
    if ((NULL == handle) || (NULL == tt_handle))
    {
        debug_error("invalid param \n");
        return 0;
    }

    return tt_handle->is_exited;
}

/*****************************************************************************
 函 数 名  : thread_pool_add_task
 功能描述  : 往线程池中增加一个线程
 输入参数  : RTHREAD_HANDLE handle
             int index
             const char * thread_name
             thread_func func
             void *arg
 输出参数  : 无
 返 回 值  : 成功 非空 失败 空
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
static THREAD_TASK * thread_pool_add_task(RTHREAD_HANDLE handle,
                                          int index,
                                          const char * thread_name,
                                          thread_func func, void *arg)
{
    if ((NULL == handle) || (NULL == thread_name))
    {
        debug_error("Invalid param !\n");
        return NULL;
    }
    THREAD_TASK *newtask = NULL;

    newtask = (THREAD_TASK *)calloc(1, sizeof(THREAD_TASK));
    if (NULL == newtask)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    newtask->is_destroyed = 0;  /* 设置为非退出状态 */
    newtask->is_exited    = 0;

    newtask->task_fun = func;
    newtask->arg  = arg;
    newtask->next = NULL;

    //strncpy(newtask->thread_name, thread_name, sizeof(newtask->thread_name));
    snprintf(newtask->thread_name,
                sizeof(newtask->thread_name), "[%s]:%s",
                   (index == 0) ? "normal" : "tiny", thread_name);

    pthread_mutex_lock(&handle->tp_param[index].queue_mutex);
    if (handle->tp_param[index].task_queue_head == NULL)
    {
        handle->tp_param[index].task_queue_head = handle->tp_param[index].task_queue_end = newtask;
    }
    else
    {
        handle->tp_param[index].task_queue_end = handle->tp_param[index].task_queue_end->next = newtask;
    }

    handle->tp_param[index].task_queue_size++;
    pthread_cond_signal(&handle->tp_param[index].queue_cond);

    pthread_mutex_unlock(&handle->tp_param[index].queue_mutex);

    return newtask;
}

/*****************************************************************************
 函 数 名  : thread_pool_entrance
 功能描述  : 线程池中各线程的入口
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : 成功 非空 失败 空
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
static void * thread_pool_entrance(void *arg)
{
    if (NULL == arg)
    {
        debug_error("Invalid param !\n");
        return NULL;
    }

    THREAD_POOL_PARAM * tp_param = (THREAD_POOL_PARAM *)arg;

    while(1)
    {
        pthread_mutex_lock(&tp_param->queue_mutex);

        while ((0 == tp_param->task_queue_size) && (0 == tp_param->is_pool_destroyed))
        {
            pthread_cond_wait(&tp_param->queue_cond, &tp_param->queue_mutex);
        }

        if (tp_param->is_pool_destroyed)
        {
            pthread_mutex_unlock(&(tp_param->queue_mutex));
            pthread_exit(NULL);
        }

        tp_param->idle_thread_num--;

        control_info("idle thread count: [%d] in [%s] thread pool\n",
                            tp_param->idle_thread_num,
                                (tp_param->stack_size == THREAD_NORMAL_STACK_SIZE) ? "normal" : "tiny" );

        THREAD_TASK *work = NULL;
        work = tp_param->task_queue_head;
        tp_param->task_queue_head = tp_param->task_queue_head->next;
        if (tp_param->task_queue_head == NULL)
        {
            tp_param->task_queue_end = NULL;
        }

        tp_param->task_queue_size--;
        pthread_mutex_unlock(&tp_param->queue_mutex);

        work->is_runing = 1;
        work->is_exited = 0;

        if (NULL != work->task_fun)
        {
            // control_info("thread [%s] enter \n", work->thread_name);
            work->task_fun((long)work->arg);
            control_info("thread %s exit \n", work->thread_name);
        }

        work->is_runing = 0;
        work->is_exited = 1;
        tp_param->idle_thread_num++;
    }

    return NULL;
}

/*****************************************************************************
 函 数 名  : instance
 功能描述  : 单实例对象初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  : 成功 非空 失败 空
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
static RTHREAD_OBJECT * instance(void)
{
    static RTHREAD_OBJECT * handle = NULL;

    if (NULL == handle)
    {
        handle = (RTHREAD_OBJECT *)calloc(1, sizeof(RTHREAD_OBJECT));
        if (NULL == handle)
        {
            debug_error("not enough memory \n");
            return NULL;
        }
    }

    return handle;
}

