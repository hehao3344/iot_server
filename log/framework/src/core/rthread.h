#ifndef __RTHREAD_H
#define __RTHREAD_H

#include <core/core.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _RTHREAD_OBJECT * RTHREAD_HANDLE;
typedef struct _THREAD_TASK    * TTASK_HANDLE;

/* 函数指针 定义 */
typedef void (* thread_func)(long user_info);

RTHREAD_HANDLE rthread_create(void);

void rthread_destroy(RTHREAD_HANDLE handle);

/* 返回值 -1 失败 0  成功 */
TTASK_HANDLE rthread_add(RTHREAD_HANDLE handle, const char * thread_name,
                         thread_func fun, long user_info);

/* 返回值 -1 失败 0  成功 */
TTASK_HANDLE rthread_tiny_add(RTHREAD_HANDLE handle, const char * thread_name,
                              thread_func fun, long user_info);

/* 调用rthread_add后 需要调用rthread_free释放内存资源 */
void rthread_free(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle);

/* 退出线程 该函数相当于发了一个命令 必须调用 rthread_wait_exit 等待线程退出 */
void rthread_delete(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle);

void rthread_wait_exit(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle);

int  rthread_is_exited(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle);

int  rthread_is_destroyed(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle);

int  rthread_is_running(RTHREAD_HANDLE handle, TTASK_HANDLE tt_handle);

#ifdef __cplusplus
}
#endif

#endif
