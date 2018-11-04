#ifndef __SOCK_H__
#define __SOCK_H__

#include <arpa/inet.h>

#ifdef __cplusplus
extern "C"
{
#endif

int sock_open(int sock_type);
int sock_bind(int fd, const char* ip, int port);
int sock_get_addr(const char* ip, int port, struct sockaddr_in * addr);
int sock_set_addr_reuse(int fd);
int sock_udp_bind(int sock_fd, unsigned short port);
void sock_close(int fd);

int sock_get_link(char * dev);
int sock_set_link(char * dev, int value);

#ifdef __cplusplus
}
#endif

#endif

