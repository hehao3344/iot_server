/*
* @brief can be used in win32 and linux.
*/
#ifndef __SOCK_CORE_H
#define __SOCK_CORE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void  sock_init(void);
void  sock_exit(void);

int  udp_open(void);
int  udp_bind(int sock_fd, unsigned short port);
int  udp_open_bind(unsigned short port);
int  udp_recvfrom(int sock_fd, char* IP, unsigned short *port, char *buffer, int len);
int  udp_recvfrom_timeout(int sock_fd, char* IP, unsigned short *port, char *buffer, int len, int msec);
int  udp_sendto(int sock_fd, char* IP, unsigned short port, char *buffer, int len);
void udp_close(int sock_fd);
int  udp_recvfrom2 (int sockFD, char *IP, unsigned short port, char *buffer, int len);
int  udp_recvfrom2_timeout(int sockFD, char *IP, unsigned short port, char *buffer, int len, int msec);

int  tcp_open(void);
int  tcp_set_reuse_addr(int sock_fd);
int  tcp_open_set_reuse_and_bind(unsigned short port);
int  tcp_open_and_bind(unsigned short port);
int  tcp_listen(int sock_fd, int count);
int  tcp_accept(int sock_fd, char *peerIP, unsigned short *peerPort);
int  tcp_set_nonblock(int sock_fd);
int  tcp_connect(int sock_fd, char* IP, unsigned short port);
int  tcp_connect_timeout(int fd, char* IP, unsigned short port, uint msec);
int  tcp_get_peer_addr(int sock_fd, char *ip, unsigned short *port);
int  tcp_open_connect(char* IP, unsigned short port);
int  tcp_open_connect_timeout(char* IP, unsigned short port, uint msec);
int  tcp_recv_timeout(int sock_fd, char *buffer, int len, int msec);
int  tcp_recv(int sock_fd, char *buffer, int len);
int  tcp_recv_peek(int sock, char *buffer, int len);
int  tcp_send_timeout(int sock_fd, char *buffer, int len, int msec);
int  tcp_send(int sock_fd, const char *buffer, int len);
void tcp_close(int sock_fd);

void get_sock_addr_by_ip_and_port(struct sockaddr_in *psAddr, char* IP, unsigned short port);
int  get_local_ip(char *ip);
int  get_local_ip_by_name(char *ip, char *devName);

int  get_local_bind_port(int sock_fd, unsigned short *port);

#ifdef __cplusplus
}
#endif

#endif  // __SOCK_CORE_H
