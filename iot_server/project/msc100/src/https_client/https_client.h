#ifndef __HTTPS_CLIENT_H
#define __HTTPS_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

int https_clt_post(char *host, int port, char *url, const char *data, int dsize, char *buff, int bsize);
int https_client_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif // __HTTPS_CLIENT_H
