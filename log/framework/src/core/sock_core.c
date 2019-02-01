#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "debug.h"
#include "sock_core.h"

#ifdef WIN32
#include <Winsock.h>
static int init_flags = FALSE;
#endif

static int map_socket_retval(int value);

void sock_init(void)
{
#ifdef WIN32
    if (!init_flags)
    {
        WSADATA     wsaData;
        WSAStartup(MAKEWORD(1,1),&wsaData);
        init_flags = TRUE;
    }
#endif
}

void sock_exit(void)
{
#ifdef WIN32
    if (init_flags)
    {
        WSACleanup();
        init_flags = FALSE;
    }
#endif
}

int udp_open(void)
{
    sock_init();

    return socket(AF_INET, SOCK_DGRAM, 0);
}

int udp_bind(int sock_fd, unsigned short port)
{
    int ret = -1;
#ifdef WIN32
    SOCKADDR_IN svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(port);
    if (SOCKET_ERROR != bind(sock_fd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)))
    {
        debug_print("bind port %d ok \n", port);
        ret = 0;
    }
#else
    struct  sockaddr_in svr_addr;
    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sin_family       = AF_INET;
    svr_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    svr_addr.sin_port         = htons(port);
    if (-1 != bind(sock_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_in)))
    {
        debug_print("bind port %d ok \n", port);
        ret = 0;
    }
#endif

    return ret;
}

int udp_open_bind(unsigned short port)
{
    int fd = 0;

    sock_init();
    fd = udp_open();
    if (fd <= 0)
	{
	    debug_print("open error \n");
	    return -1;
	}
    if (0 == udp_bind(fd, port))
    {
        udp_close(fd);
        debug_print("bind error \n");
        return -1;
    }

    return fd;
}

int udp_recvfrom(int sock_fd, char* ip, unsigned short *port, char *buffer, int len)
{
    int sret, ret;
    socklen_t sock_len = sizeof(struct sockaddr_in);

#ifdef WIN32
    SOCKADDR_IN sock_addr;
#else
    struct sockaddr_in sock_addr;
#endif

    sret = recvfrom(sock_fd, buffer, len, 0, (struct sockaddr *)&sock_addr, &sock_len);
    if (sret > 0)
    {
        ret = sret;
    }
    else
    {
        ret = -1;
    }

    strcpy(ip, inet_ntoa(sock_addr.sin_addr));
    *port = ntohs(sock_addr.sin_port);

	return ret;
}

int udp_recvfrom_timeout(int sock_fd, char* ip, unsigned short *port, char *buffer, int len, int msec)
{
    int ret = -1, sret, retval;
    int max_fd;
    fd_set readfds;
    struct timeval tv;
    socklen_t sock_len = sizeof(struct sockaddr_in);

    FD_ZERO(&readfds);
    FD_SET(sock_fd, &readfds);

    max_fd = sock_fd;
    tv.tv_sec  = msec/1000;
    tv.tv_usec = (msec%1000)*1000;

    retval = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (retval > 0)
    {
        /* if we woke up on client_sockfd do the data passing */
        if (FD_ISSET(sock_fd, &readfds))
        {
#ifdef WIN32
            SOCKADDR_IN sock_addr;
#else
            struct  sockaddr_in sock_addr;
#endif
            sret = recvfrom(sock_fd, buffer, len, 0, (struct sockaddr *)&sock_addr, &sock_len);
            if (sret > 0)
            {
                ret = sret;
            }
            else
            {
                ret = -1;  // no data;
            }
            strcpy(ip, inet_ntoa(sock_addr.sin_addr));
            *port = ntohs(sock_addr.sin_port);
        }
    }
    else if (retval == 0)
    {
        // timeout
        ret = -1;
    }
    else
    {
        // error
        ret = 0;
    }

    FD_CLR(sock_fd, &readfds);

	return  ret;
}

int udp_recvfrom2(int sock_fd, char *ip, unsigned short port, char *buffer, int len)
{
    struct  sockaddr_in  sock_addr;
#ifdef WIN32
    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.S_un.S_addr = inet_addr(ip);
    sock_addr.sin_port = htons(port);
#else
    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_family         = AF_INET;
    sock_addr.sin_addr.s_addr    = inet_addr(ip);
    sock_addr.sin_port           = htons(port);
#endif
    int sret, ret = -1;
    socklen_t sock_len = sizeof(struct sockaddr_in);

    sret = recvfrom(sock_fd, buffer, len, 0, (struct sockaddr *)&sock_addr, &sock_len);
    if (sret > 0)
    {
        ret = sret;
    }
    else
    {
        ret = -1;
    }

	return ret;
}

int udp_recvfrom2_timeout(int sock_fd, char *ip, unsigned short port,
                               char *buffer, int len, int msec)
{
    struct  sockaddr_in  sock_addr;
#ifdef WIN32
    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.S_un.S_addr = inet_addr(ip);
    sock_addr.sin_port = htons(port);
#else
    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_family         = AF_INET;
    sock_addr.sin_addr.s_addr    = inet_addr(ip);
    sock_addr.sin_port           = htons(port);
#endif

    int ret = -1, sret, retval;
    int max_fd;
    fd_set readfds;
    struct timeval tv;
    socklen_t  sock_len = sizeof(struct sockaddr_in);

    FD_ZERO(&readfds);
    FD_SET(sock_fd, &readfds);

    max_fd = sock_fd;
    tv.tv_sec  = msec/1000;
    tv.tv_usec = (msec%1000)*1000;
    retval = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (retval > 0)
    {
        // if we woke up on client_sockfd do the data passing
        if (FD_ISSET(sock_fd, &readfds))
        {
            sret = recvfrom(sock_fd, buffer, len, 0, (struct sockaddr *)&sock_addr, &sock_len);
            if (sret > 0)
            {
                ret = sret;
            }
            else
            {
                ret = -1;  // no data;
            }
        }
    }
    else if (retval == 0)
    {
        // timeout
        ret = -1;
    }
    else
    {
        // error
        ret = 0;
    }
    FD_CLR(sock_fd, &readfds);

	return ret;
}

int udp_sendto(int sock_fd, char* ip, unsigned short port, char *buffer, int len)
{
    struct sockaddr_in addr;
    addr.sin_family        = AF_INET;
    addr.sin_port          = htons(port);
    addr.sin_addr.s_addr   = inet_addr(ip);
    memset(&addr.sin_zero, 0, 8);
    // bzero(&(addr.sin_zero), 8);

    return sendto(sock_fd, buffer, len, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
}

void udp_close(int sock_fd)
{
    if (sock_fd > 0)
    {
#ifdef WIN32
        closesocket(sock_fd);
#else
        close(sock_fd);
#endif
    }
    sock_exit();
}

int tcp_open(void)
{
    sock_init();

    return socket(AF_INET, SOCK_STREAM, 0);
}

int tcp_set_reuse_addr(int sock_fd)
{
    int reuse = 1;
    int ret  = -1;
    if (sock_fd > 0)
    {
#ifdef WIN32
        if (-1 != setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)))
        {
            ret = 0;
        }
#else // linux
        if (-1 != setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)))
        {
            ret = 0;
        }
#endif
    }

    return ret;
}

int tcp_open_set_reuse_and_bind(unsigned short port)
{
    int   sock_fd = 0;
    int   reuse = 1;
    struct  sockaddr_in  svr_addr;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd <= 0)
    {
        return -1;
    }

#ifdef WIN32
    // set reuse addr param
    if (-1 == setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)))
    {
        return -1;
    }

    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(port);
    if (SOCKET_ERROR != bind(sock_fd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)))
    {
        debug_print("bind port %d ok \n", port);
    }
    else
    {
        debug_print("bind port %d failed \n", port);
        tcp_close(sock_fd);
    }

#else
    // set reuse addr param
    if (-1 == setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)))
    {
        return -1;
    }

    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sin_family         = AF_INET;
    svr_addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    svr_addr.sin_port           = htons(port);
    if (-1 != bind(sock_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_in)))
    {
        debug_print("bind port %d ok \n", port);
    }
#endif

    return sock_fd;
}

int tcp_open_and_bind(unsigned short port)
{
    int   sock_fd = 0;
    struct  sockaddr_in  svr_addr;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd <= 0)
    {
        debug_print("socket failed \n");
        return -1;
    }
#ifdef WIN32
    memset(&svr_addr, 0, sizeof(svr_addr));

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(port);
    if (SOCKET_ERROR != bind(sock_fd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)))
    {
        debug_print("tcp bind port %d ok \n", port);
    }
#else
    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sin_family         = AF_INET;
    svr_addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    svr_addr.sin_port           = htons(port);
    if (-1 != bind(sock_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_in)))
    {
        debug_print("tcp Bind port %d ok \n", port);
    }
    else
    {
        debug_print("tcp Bind port %d failed \n", port);
        close(sock_fd);
        sock_fd = 0;
    }
#endif

    return sock_fd;
}

int tcp_set_nonblock(int sock_fd)
{
    if (sock_fd < 0)
    {
        debug_error("invalid param \n");
        return -1;
    }

    int flags;
    flags = fcntl(sock_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(sock_fd, F_SETFL, flags);

    return 0;
}

int tcp_listen(int sock_fd, int count)
{
    if (listen(sock_fd, count) < 0)
    {
        return -1;
    }

    return 0;
}

int tcp_accept(int sock_fd, char *peer_ip, unsigned short *peer_port)
{
    int new_fd = 0;

	struct sockaddr_in	addr;
	size_t				len;

	len = sizeof(struct sockaddr_in);
	if ((new_fd = accept(sock_fd, (struct sockaddr *) &addr, (socklen_t *) &len)) < 0)
    {
        debug_error("socket accept failed \n");
		return -1;
	}

    strcpy(peer_ip, inet_ntoa(addr.sin_addr));
    *peer_port = ntohs(addr.sin_port);

    return new_fd;
}

// ip - host, should convert to network
int tcp_connect(int sock_fd, char* ip, unsigned short port)
{
    int     ret = -1;
    struct  sockaddr_in  svr_addr;

    svr_addr.sin_family      = AF_INET;
    svr_addr.sin_port        = htons(port);
    svr_addr.sin_addr.s_addr = inet_addr(ip);

    debug_print("connectting server: %s:%d \n", ip, port);
    if (-1 != connect(sock_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr)))
    {
        debug_print("connect to server: %s ok \n", ip);
        ret = 0;
    }

    return ret;
}

int tcp_connect_timeout(int fd, char* ip, unsigned short port, uint msec)
{
    int ret = -1;
    struct timeval tm;
    fd_set w_set;
    struct sockaddr_in svr_addr;

#ifdef WIN32
    uint  flags = 1;
    if (0 != ioctlsocket(fd, FIONBIO, (uint*)&flags))
    {
        return -1;
    }
#else
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
#endif

    svr_addr.sin_family      = AF_INET;
    svr_addr.sin_port        = htons(port);
    svr_addr.sin_addr.s_addr = inet_addr(ip);

    connect(fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr));

    tm.tv_sec  = (msec/1000);
    tm.tv_usec = (msec%1000) * 1000;
    FD_ZERO(&w_set);
    FD_SET(fd, &w_set);

    if (select(fd + 1, NULL, &w_set, NULL, &tm) <= 0)
    {
        //debug_print("Network failed in connect\n");
    }
    else
    {
        if(FD_ISSET(fd, &w_set))
        {
#ifdef WIN32
            flags = 0;
            if (0 != ioctlsocket(fd, FIONBIO, (uint*)&flags))
            {
                debug_print("ioctlsocket error \n");
                return 0;
            }
#else
            flags &= ~O_NONBLOCK;
            fcntl(fd,F_SETFL, flags); // block mode
#endif
            ret = 0;
        }
    }

	return ret;
}

int tcp_open_connect(char* ip, unsigned short port)
{
    int fd  = 0;
    struct sockaddr_in svr_addr;
    sock_init();
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0)
	{
	    debug_error("open error \n");
	    return -1;
	}

    svr_addr.sin_family      = AF_INET;
    svr_addr.sin_port        = htons(port);
    svr_addr.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr)))
    {
        debug_print("tcp connect %s failed \n", ip);
        tcp_close(fd);
        return -1;
    }
    debug_print("connect to server: %s ok \n", ip);

    return fd;
}

int tcp_open_connect_timeout(char* ip, unsigned short port, unsigned int msec)
{
    int   fd    = 0;
    uint  flags = 1;
    struct timeval tm;
    fd_set w_set;
    struct sockaddr_in svr_addr;
    sock_init();
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0)
	{
	    debug_print("open error \n");
	    return -1;
	}

#ifdef WIN32
    if ( 0 != ioctlsocket(fd, FIONBIO, (uint*)&flags)) // nonblock
    {
        return -1;
    }
#else
    flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags); // nonblock
#endif

    svr_addr.sin_family      = AF_INET;
    svr_addr.sin_port        = htons(port);
    svr_addr.sin_addr.s_addr = inet_addr(ip);

    connect(fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr));

    tm.tv_sec  = msec/1000;
    tm.tv_usec = (msec%1000) * 1000;
    FD_ZERO(&w_set);
    FD_SET(fd, &w_set);

    if (select(fd + 1, NULL, &w_set, NULL, &tm) <= 0)
    {
        debug_print("network error in connect\n");
        tcp_close(fd);
		return -1;
    }
    else
    {
        if(FD_ISSET(fd,&w_set))
        {
#ifdef WIN32
            flags = 0;
            if (0 != ioctlsocket(fd, FIONBIO, (uint*)&flags))
            {
                debug_print(" ioctlsocket error \n");
                return 0;
            }
#else
            flags &= ~O_NONBLOCK;
            fcntl(fd,F_SETFL, flags);// block mode
#endif
            return fd;
        }
        else
        {
            tcp_close(fd);
			return -1;
        }
    }

	return -1;
}

// return value
// >0, success
// =0, peer closed or sock error, we should reopen socket;
// -1, time out
int tcp_recv_timeout(int sock_fd, char *buffer, int len, int msec)
{
    int max_fd;
	int ret, sret, retval;
    fd_set  readfds;
    struct  timeval tv;

    FD_ZERO(&readfds);
    FD_SET(sock_fd, &readfds);

    max_fd = sock_fd;

    tv.tv_sec = msec/1000;
    tv.tv_usec  = (msec%1000)*1000;

    retval = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (retval > 0)
    {
        // if we woke up on client_sockfd do the data passing
        if (FD_ISSET(sock_fd, &readfds))
        {
            // memset(buffer, 0, len);
            sret = recv(sock_fd, buffer, len, 0);
            ret = map_socket_retval(sret);
            if (0 == ret)
            {
                debug_print("recv peer closed \n");
            }
        }
    }
    else if (retval == 0)
    {
        // timeout
        ret = -1;
    }
    else
    {
        // error
        ret = 0;
    }

    FD_CLR(sock_fd, &readfds);

	return ret;
}

int tcp_get_peer_addr(int sock_fd, char *ip, unsigned short *port)
{
    int ret = -1;
    struct  sockaddr_in sa;
    socklen_t len = sizeof(sa);
    if (0 == getpeername(sock_fd, (struct sockaddr *)&sa, &len))
    {
        strcpy(ip, inet_ntoa(sa.sin_addr));
        *port = ntohs(sa.sin_port);
        ret = 0;
    }

    return ret;
}

int tcp_recv(int sock_fd, char *buffer, int len)
{
    int ret = recv(sock_fd, buffer, len, 0);

    return map_socket_retval(ret);
}

int tcp_recv_peek(int sock, char *buffer, int len)
{
    int ret = recv(sock, buffer, len, MSG_PEEK);

    return map_socket_retval(ret);
}

// ret > 0   send ok.
// ret = 0   peer closed.
// ret = -1  would block.
int tcp_send_timeout(int sock_fd, char *buffer, int len, int msec)
{
	int ret, sret, retval;
    int  max_fd;
    fd_set writefds;
    struct timeval tv;

    FD_ZERO(&writefds);
    FD_SET(sock_fd, &writefds);

    max_fd = (int)sock_fd;

    tv.tv_sec  = msec/1000;
    tv.tv_usec = (msec%1000)*1000;

    retval = select(max_fd + 1, NULL, &writefds, NULL, &tv);
    if (retval > 0)
    {
        // if we woke up on client_sockfd do the data passing
        if (FD_ISSET(sock_fd, &writefds))
        {
            sret = send(sock_fd, buffer, len, 0);
            ret = map_socket_retval(sret);
            if (0 == ret)
            {
                debug_print("send peer closed \n");
            }
        }
    }
    else if (0 == retval)
    {
        // timeout
        // debug_print("send would block \n");
        ret = -1;
    }
    else
    {
        // error
        debug_print("send select error \n");
        ret = 0;
    }

    FD_CLR(sock_fd, &writefds);

    return ret;
}

// ret > 0   send ok.
// ret = 0   peer closed.
// ret = -1  would block.
int tcp_send(int sock_fd, const char *buffer, int len)
{
    int ret = send(sock_fd, buffer, len, 0);

    return map_socket_retval(ret);
}

void tcp_close(int sock_fd)
{
    if (sock_fd > 0)
    {
#ifdef WIN32
        closesocket(sock_fd);
#else
        close(sock_fd);
#endif
    }
    sock_exit();
}

void get_sock_addr_by_ip_port(struct sockaddr_in *psAddr, char *ip, unsigned short port)
{
#ifdef WIN32
    psAddr->sin_family = AF_INET;
    psAddr->sin_addr.S_un.S_addr = inet_addr(ip);
    psAddr->sin_port = htons(port);
#else
    psAddr->sin_addr.s_addr = inet_addr(ip);
    psAddr->sin_family      = AF_INET;
    psAddr->sin_port        = htons(port);
#endif
}

int get_local_ip(char *ip)
{
    uint ip_local = 0;
#ifdef WIN32
    char host[128];
    struct in_addr addr;
    char *tmp = NULL;
    struct hostent* psHost;
    sock_init();
    memset(host, 0, sizeof(host));
    if (gethostname(host, sizeof(host)) < 0)
    {
        debug_print("can't get hostname");
        return -1;
    }
    psHost = gethostbyname(host);
    if (NULL == psHost)
    {
        debug_print("gethostbyname error\n");
        return -1;
    }

    memcpy(&addr, (char *)psHost->h_addr_list[0], sizeof(struct in_addr));
    tmp = inet_ntoa(addr);

    strcpy(ip, tmp);
    // memcpy((char *)ip, (char *) psHost->h_addr_list[0], psHost->h_length);
    // memcpy((char *)&ip_local, (char *) psHost->h_addr_list[0], sizeof(uint));
#else
    int  fd;
	struct sockaddr_in *ps_addr;
	struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM,  0);
    if (-1 == fd)
    {
        debug_print("socket failed \n");
        return -1;
    }

    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        debug_print("ioctl failed \n");
        return -1;
    }

    ps_addr = (struct sockaddr_in*)&ifr.ifr_addr;

    // memcpy(ip, &ps_addr->sin_addr, sizeof(uint));
    memcpy((char *)&ip_local, &ps_addr->sin_addr, sizeof(uint));

    close(fd);
#endif

    return 0;
}

int get_local_ip_by_name(char *ip, char *devName)
{
#ifdef WIN32
    uint ip_local = 0;
    char host[128];
    struct in_addr addr;
    char *tmp = NULL;
    struct hostent* psHost;
    sock_init();
    memset(host, 0, sizeof(host));
    if (gethostname(host, sizeof(host)) < 0)
    {
        debug_print("Can't get hostname");
        return FALSE;
    }
    psHost = gethostbyname(host);
    if (NULL == psHost)
    {
        debug_print("gethostbyname error\n");
        return FALSE;
    }

    memcpy(&addr, (char *)psHost->h_addr_list[0], sizeof(struct in_addr));
    tmp = inet_ntoa(addr);

    strcpy(ip, tmp);
#else
    int  fd;
    struct sockaddr_in *saddr;
	struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM,  0);
    if (-1 == fd)
    {
        debug_print("socket failed \n");
        return -1;
    }

    strncpy(ifr.ifr_name, devName, IFNAMSIZ);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        debug_print("ioctl failed = %s \n", devName);
        return -1;
    }
    saddr = (struct sockaddr_in*)&(ifr.ifr_addr);
    strcpy(ip, inet_ntoa(saddr->sin_addr));

    debug_print("get local ip:%s device %s\n", ip, devName);

    close(fd);
#endif

    return 0;
}

int get_local_bind_port(int sock_fd, unsigned short *port)
{
    int ret = -1;
    struct sockaddr_in sock_addr;
    int len = sizeof(struct sockaddr);
    if (0 == getsockname(sock_fd,  (struct sockaddr*)&sock_addr, (socklen_t *)&len))
    {
        *port = ntohs(sock_addr.sin_port);
        ret = 0;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// static function
////////////////////////////////////////////////////////////////////////////////
// ret > 0  ok.
// ret = 0  sock error or peer closed.
// ret = -1 would block.
static int map_socket_retval(int value)
{
    int ret = 0;
    if (value > 0)
    {
        ret = value;
    }
    else if (-1 == value)
    {
#ifdef WIN32
        if (WSAEINPROGRESS == WSAGetLastError())
        {
            ret = -1;
        }
        else
        {
            ret = 0;
        }
#else
        if ((errno == EINTR) || (errno == EWOULDBLOCK) || (errno == EAGAIN))
        {
            //debug_print("server would block?\n");
            ret = -1;
        }
        else
        {
            debug_print("peer closed or error?\n");
            ret = 0;
        }
#endif
    }
    else
    {
        debug_print("peer closed ?\n");
        ret = 0;
    }

    return ret;
}
