/******************************************************************************

                  版权所有 (C), 2018-2099

 ******************************************************************************
  文 件 名   : sock.c
  版 本 号   : 初稿
  作    者   : hehao
  生成日期   : 2018年10月22日
  最近修改   :
  功能描述   : 网络基础API模块

  修改历史   :
  1.日    期   : 2018年10月22日
    作    者   : hehao
    修改内容    :  创建文件

******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "debug.h"
#include "sock.h"

/*----------------------------------------------*
 * 宏定义                                          *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 类型定义                                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数声明**                                     *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : sock_open
 功能描述  : 创建socket
 输入参数  : int sock_type
 输出参数  : 无
 返 回 值  : 成功 > 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int sock_open(int sock_type)
{
    int fd = socket(AF_INET, sock_type, 0);
    if (fd <= 0)
    {
        debug_error("sock_open failed errno: 0x%x \n", errno);
        return -1;
    }

    return fd;
}

/*****************************************************************************
 函 数 名  : sock_close
 功能描述  : 关闭socket
 输入参数  : int fd
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
void sock_close(int fd)
{
    if (fd > 0)
    {
        close(fd);
    }
}

/*****************************************************************************
 函 数 名  : sock_udp_bind
 功能描述  : udp socket绑定端口
 输入参数  : int sock_fd
             unsigned short port
 输出参数  : 无
 返 回 值  : 成功 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int sock_udp_bind(int sock_fd, unsigned short port)
{
    struct  sockaddr_in svr_addr;
    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sin_family       = AF_INET;
    svr_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    svr_addr.sin_port         = htons(port);
    if (-1 == bind(sock_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_in)))
    {
        debug_error("udp bind failed \n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : sock_bind
 功能描述  : 根据IP地址绑定端口
 输入参数  : int fd
             const char * ip
             int port
 输出参数  : 无
 返 回 值  : 成功 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int sock_bind(int fd, const char * ip, int port)
{
    struct sockaddr_in local_addr;
    if (sock_get_addr(ip, port, &local_addr) < 0)
    {
        return -1;
    }

    if (-1 == bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr)))
    {
        debug_error("bind %d failed \n", port);
        return -1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : sock_get_addr
 功能描述  : 根据ip地址和端口号得到sockaddr_in类型的地址
 输入参数  : const char * ip
             int port
             struct sockaddr_in *addr
 输出参数  : 无
 返 回 值  : 成功 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年10月22日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int sock_get_addr(const char * ip, int port, struct sockaddr_in *addr)
{
    int ip_n = 0;

    if ((NULL == ip) || ('\0' == ip[0]))
    {
        ip_n = 0;
    }
    else if (INADDR_NONE == inet_addr(ip))
    {
        struct hostent* hent = NULL;
        if (NULL == (hent = gethostbyname(ip)))
        {
            debug_error("failed ip is %s \n", ip);
            return -1;
        }
        memcpy(&ip_n, *(hent->h_addr_list), sizeof(ip_n));
    }
    else
    {
        ip_n = inet_addr(ip);
    }

    addr->sin_family      = AF_INET;
    addr->sin_addr.s_addr = ip_n;
    addr->sin_port        = htons(port);

    return 0;
}

/*****************************************************************************
 函 数 名  : sock_set_addr_reuse
 功能描述  : 设置socket为reuse属性
 输入参数  : int fd
 输出参数  : 无
 返 回 值  : 成功 0 失败 -1
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月5日
    作    者   : 何浩
    修改内容   : 新生成函数

*****************************************************************************/
int sock_set_addr_reuse(int fd)
{
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0)
    {
        debug_error("reuse address failed \n");
        return -1;
    }

    return 0;
}

/* 得到物理连接状态 -1 出错 1 已经连接 0 未连接 */
int sock_get_link(char * dev)
{
    int ret;
    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd <= 0)
    {
        debug_error("socket error \n");
        return -1;
    }

    strcpy(ifr.ifr_name, dev);
    if (ioctl(fd , SIOCGIFFLAGS, &ifr) < 0)
    {
        debug_error("error \n");
        close(fd);
        return -1;
    }

    if(ifr.ifr_flags & IFF_RUNNING)
    {
        // IFF_RUNNING	资源已分配.
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    close(fd);

    return ret;
}

/* value 0: set net link to down value 1: set net link to up */
int sock_set_link(char * dev, int value)
{
    int ret;
    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd <= 0)
    {
        debug_error("socket error \n");
        return -1;
    }

    strcpy(ifr.ifr_name, dev);
    if (0 == value)
    {
        if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
        {
            debug_error( "get link status failed \n");
            close(fd);
            return -1;
        }
        ifr.ifr_ifru.ifru_flags &= ~IFF_UP;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
        {
            debug_error( "set link status failed \n");
            close(fd);
            return -1;
        }
    }
    else if (1 == value)
    {
        ifr.ifr_flags |= IFF_UP|IFF_RUNNING;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
        {
            debug_error("set link status failed \n");
            close(fd);
            return -1;
        }
    }
    close(fd);

    return 0;
}

