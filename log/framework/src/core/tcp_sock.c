/******************************************************************************

                  版权所有 (C), 2018-2099,

 ******************************************************************************
  文 件 名   : tcp_sock.c
  版 本 号   : 初稿
  作    者   : hehao
  生成日期   : 2018年10月22日
  最近修改   :
  功能描述   : TCP socket模块

  修改历史   :
  1.日    期   : 2018年10月22日
    作    者   : hehao
    修改内容    :  创建文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "debug.h"
#include "tcp_sock.h"

/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 类型定义                                         *
 *----------------------------------------------*/
typedef struct _TCP_SOCK_OBJECT
{
    int fd;
} TCP_SOCK_OBJECT;

/*----------------------------------------------*
 * 全局变量定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数声明**                                     *
 *----------------------------------------------*/
static int map_socket_retval(int value);
static int select_check_connect(int fd, int msec);

/*****************************************************************************
 函 数 名  : tcp_sock_create
 功能描述  : 创建tcp sock模块
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
TCP_SOCK_HANDLE tcp_sock_create(void)
{
    TCP_SOCK_OBJECT * handle = (TCP_SOCK_OBJECT *)calloc(1, sizeof(TCP_SOCK_OBJECT));
    if (NULL == handle)
    {
        debug_error("not enough memory \n");
        return NULL;
    }

    return handle;
}

/*****************************************************************************
 函 数 名  : tcp_sock_destroy
 功能描述  : 销毁tcpsock
 输入参数  : TCP_SOCK_HANDLE handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void tcp_sock_destroy(TCP_SOCK_HANDLE handle)
{
    if (NULL != handle)
    {
        free(handle);
    }
}

/*****************************************************************************
 函 数 名  : tcp_sock_open
 功能描述  : 打开tcp socket
 输入参数  : TCP_SOCK_HANDLE handle
 输出参数  : 无
 返 回 值  : 成功0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_open(TCP_SOCK_HANDLE handle)
{
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return -1;
    }

    if ((handle->fd = sock_open(SOCK_STREAM)) < 0)
    {
        debug_error("sock_open failed \n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : tcp_sock_close
 功能描述  : 关闭tcp sock
 输入参数  : TCP_SOCK_HANDLE handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void tcp_sock_close(TCP_SOCK_HANDLE handle)
{
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return;
    }
    if (handle->fd > 0)
    {
        shutdown(handle->fd, SHUT_RDWR);
        sock_close(handle->fd);
        handle->fd = -1;
    }
}

/*****************************************************************************
 函 数 名  : tcp_sock_get_sockfd
 功能描述  : 得到当前socket句柄
 输入参数  : TCP_SOCK_HANDLE handle
 输出参数  : 无
 返 回 值  : 成功 tcp句柄 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_get_sockfd(TCP_SOCK_HANDLE handle)
{
    if (NULL == handle)
    {
        debug_error("invalid param \n");
        return -1;
    }

    if (handle->fd <= 0)
    {
        debug_error("socket fd %d invalid \n", handle->fd);
    }

    return handle->fd;
}

/*****************************************************************************
 函 数 名  : tcp_sock_connect
 功能描述  : tcp连接到指定的ip 端口
 输入参数  : TCP_SOCK_HANDLE handle
           const char * ip 目的IP
             int port      目的端口
 输出参数  : 无
 返 回 值  : 成功0 失败-1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_connect(TCP_SOCK_HANDLE handle, const char * ip, int port)
{
    if ((NULL == handle) || (handle->fd <= 0))
    {
        debug_error("invalid param \n");
        return -1;
    }

    struct sockaddr_in remote;

    if (sock_get_addr(ip, port, &remote) < 0)
    {
        return -1;
    }

    if (-1 == connect(handle->fd, (struct sockaddr *)&remote, sizeof(struct sockaddr)))
    {
        if (errno != EINPROGRESS)
        {
            sock_close(handle->fd);
            debug_error("failed \n");
            return -1;
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : tcp_sock_connect_timeout
 功能描述  : tcp连接到指定ip，端口，带超时功能
 输入参数  : TCP_SOCK_HANDLE handle
             const char * ip  连接IP
             int port  连接端口
             int msec  超时时间：毫秒
 输出参数  : 无
 返 回 值  : 成功 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_connect_timeout(TCP_SOCK_HANDLE handle, const char * ip, int port, int msec)
{
    int ret = -1;
    int connect_ret = -1;
    struct sockaddr_in svr_addr;

    if ((NULL == handle) || (handle->fd <= 0))
    {
        debug_error("invalid param \n");
        return -1;
    }

    int flags;
    flags = fcntl(handle->fd, F_GETFL, 0);
    flags |= O_NONBLOCK; /* 设置为非阻塞 */
    fcntl(handle->fd, F_SETFL, flags);

    svr_addr.sin_family      = AF_INET;
    svr_addr.sin_port        = htons(port);
    svr_addr.sin_addr.s_addr = inet_addr(ip);

    connect_ret = connect(handle->fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr));
    if (connect_ret < 0)
    {
        if (EINPROGRESS == errno)
        {
            // debug_info("connect is in processing \n");
        }
        else
        {
            debug_error("connect failed \n");
            return -1;
        }
    }
    if (0 != select_check_connect(handle->fd, msec))
    {
        debug_error("select failed \n");
        ret = -1;
    }
    else
    {
        debug_info("select success \n");
        flags &= ~O_NONBLOCK;
        fcntl(handle->fd, F_SETFL, flags); /* block mode */
        ret = 0;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : tcp_sock_send
 功能描述  : tcp方式发送数据
 输入参数  : TCP_SOCK_HANDLE handle
             const char * buf
             int len
 输出参数  : 无
 返 回 值  :
            ret > 0   send ok.
            ret = 0   peer closed.
            ret = -1  would block or invalid input param!
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_send(TCP_SOCK_HANDLE handle, const char * buf, int len)
{
    int ret = 0;
    if ((NULL == handle) || (NULL == buf) || (len <= 0) || (handle->fd <= 0))
    {
        debug_error("invalid param \n");
        return -1;
    }

    ret = send(handle->fd, buf, len, 0);

    return map_socket_retval(ret);
}

/*****************************************************************************
 函 数 名  : tcp_send_timeout
 功能描述  : tcp发送数据，带超时
 输入参数  : TCP_SOCK_HANDLE handle
             const char * buf
             int len
             int msec 超时时间毫秒为单位
 输出参数  : 无
 返 回 值  :
            ret > 0   send ok.
            ret = 0   peer closed.
            ret = -1  would block or invalid input param!
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_send_timeout(TCP_SOCK_HANDLE handle, const char * buf, int len, int msec)
{
    int ret = -1, sret, retval;
    int  max_fd;
    fd_set writef_ds;
    struct timeval tv;

    if ((NULL == handle) || (NULL == buf) || (len <= 0)  || (handle->fd <= 0))
    {
        debug_error("invalid param \n");
        return -1;
    }

    FD_ZERO(&writef_ds);
    FD_SET(handle->fd, &writef_ds);

    max_fd = (int)handle->fd;

    tv.tv_sec  = (msec/1000);
    tv.tv_usec = (msec%1000) * 1000;

    retval = select(max_fd + 1, NULL, &writef_ds, NULL, &tv);
    if (retval > 0)
    {
        /* if we woke up on client_sockfd do the data passing */
        if (FD_ISSET(handle->fd, &writef_ds))
        {
            sret = send(handle->fd, buf, len, 0);
            ret = map_socket_retval(sret);
            if (0 == ret)
            {
                debug_error("tcp send peer closed \n");
            }
        }
    }
    else if (0 == retval)
    {
        /* timeout */
        /*printf("send would block \n"); */
        ret = -1;
    }
    else
    {
        /* error */
        debug_error("send select error \n");
        ret = 0;
    }

    FD_CLR(handle->fd, &writef_ds);

    return ret;
}

/*****************************************************************************
 函 数 名  : tcp_sock_recv
 功能描述  :    tcp接收数据
 输入参数  :    TCP_SOCK_HANDLE handle
            char * buf
             int len
 输出参数  : 无
 返 回 值  :
            ret > 0   send ok.
            ret = 0   peer closed.
            ret = -1  would block or invalid input param!
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_recv(TCP_SOCK_HANDLE handle, char * buf, int len)
{
    int ret = 0;
    if ((NULL == handle) || (NULL == buf) || (len <= 0)  || (handle->fd <= 0))
    {
        debug_error("failed \n");
        return -1;
    }

    ret = recv(handle->fd, buf, len, 0);

    return map_socket_retval(ret);
}

/*****************************************************************************
 函 数 名  : tcp_sock_set_nonblock
 功能描述  : tcp设置非阻塞模式
 输入参数  : TCP_SOCK_HANDLE handle
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void tcp_sock_set_nonblock(TCP_SOCK_HANDLE handle)
{
    int flags;
    if ((NULL == handle) || (handle->fd <= 0))
    {
        debug_error("invalid param \n");
        return;
    }

    flags = fcntl(handle->fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(handle->fd, F_SETFL, flags);
}

/*****************************************************************************
 函 数 名  : tcp_sock_set_block
 功能描述  : tcp设置收发模式为阻塞模式
 输入参数  : TCP_SOCK_HANDLE handle
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_set_block(TCP_SOCK_HANDLE handle)
{
    int flags = 1;

    if ((NULL == handle) || (handle->fd <= 0))
    {
        debug_error("Invalid param !\n");
        return -1;
    }

    flags = fcntl(handle->fd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(handle->fd, F_SETFL, flags); /* block mode */

    return 0;
}

/*****************************************************************************
 函 数 名  : tcp_sock_recv_timeout
 功能描述  : tcp接收数据 带超时功能
 输入参数  : TCP_SOCK_HANDLE handle
             char *buf
             int len
             int msec  超时时间 毫秒
 输出参数  : 无
 返 回 值  :
            ret > 0   send ok.
            ret = 0   peer closed.
            ret = -1  would block or invalid input param!
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int tcp_sock_recv_timeout(TCP_SOCK_HANDLE handle, char *buf, int len, int msec)
{
    int max_fd;
    int ret = 0, sret, retval;
    fd_set  readfds;
    struct  timeval tv;

    if ((NULL == handle) || (NULL == buf) || (len <= 0)  || (handle->fd <= 0))
    {
        debug_error("Invalid param %p %p %d !\n", handle, buf, len);
        if (NULL != handle)
        {
            debug_error("Invalid param %d \n", handle->fd);
        }
        return -1;
    }

    FD_ZERO(&readfds);
    FD_SET(handle->fd, &readfds);

    max_fd = handle->fd;

    tv.tv_sec  = (msec/1000);
    tv.tv_usec = (msec%1000) * 1000;

    retval = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (retval > 0)
    {
        /* if we woke up on client_sockfd do the data passing */
        if (FD_ISSET(handle->fd, &readfds))
        {
            sret = recv(handle->fd, buf, len, 0);
            ret = map_socket_retval(sret);
            if (0 == ret)
            {
                debug_error("recv peer closed \n");
            }
        }
    }
    else if (retval == 0)
    {
        /* timeout */
        ret = -1;
    }
    else
    {
        /* error */
        ret = 0;
    }

    FD_CLR(handle->fd, &readfds);

    return ret;
}

/*****************************************************************************
 函 数 名  : map_socket_retval
 功能描述  : sock的返回值和工程返回值之间的映射
 输入参数  : int value
 输出参数  : 无
 返 回 值  :
        ret > 0  ok.
        ret = 0  sock error or peer closed.
        ret = -1 would block.
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
static int map_socket_retval(int value)
{
    int ret = 0;
    if (value > 0)
    {
        ret = value;
    }
    else if (-1 == value)
    {
        if ((errno == EINTR) || (errno == EWOULDBLOCK) || (errno == EAGAIN))
        {
            // printf("server would block?\n");
            ret = -1;
        }
        else
        {
            // debug_error("peer closed or error errno:0x%x ? \n", errno);
            ret = 0;
        }
    }
    else
    {
        debug_error("peer closed ?\n");
        ret = 0;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : select_check_connect
 功能描述  : select方式检查tcp connect是否成功
 输入参数  : int fd
             int msec
 输出参数  : 无
 返 回 值  :
            成功 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
static int select_check_connect(int fd, int msec)
{
    fd_set rset, wset, eset;
    struct timeval tval;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    wset = rset;
    eset = rset;
    tval.tv_sec  = msec/1000;
    tval.tv_usec = msec%1000*1000; /* 300毫秒 */
    int ready_n;
    if ((ready_n = select(fd + 1, &rset, &wset, &eset, &tval)) == 0)
    {
        errno = ETIMEDOUT;
        debug_error(" select timeout.\n");
        return -1;
    }

    if (ready_n < 0)
    {
        debug_error("select error \n");
        return -1;
    }

    if ((FD_ISSET(fd, &rset)) && (FD_ISSET(fd, &wset) ))
    {
        int error = 0;
        socklen_t len = sizeof (error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            debug_error("getsockopt error. \n");
            return -1;
        }
        // debug_error("error no is %d \n", error);
        if (ECONNREFUSED == error)
        {
            // #define ECONNREFUSED    111     /* Connection refused */
            debug_error("ip address is online, but no tcp server now, error no is %d \n", error);
            return -1;
        }
    }

    return 0;
}

