#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <core/core.h>

#include "https_client.h"

#define HTTP_HEADERS_MAXLEN     512     // Headers 的最大长度
#define POST_DATA_MAXLEN        512     // post data 的最大长度

const char *https_post_headers = "User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
                                 "Cache-Control: no-cache\r\n"
                                 "Accept: */*\r\n"
                                 "Content-type: application/json\r\n";

static char post_buf[1024];
static char sdata_buf[HTTP_HEADERS_MAXLEN+POST_DATA_MAXLEN];

static int client_connect_tcp(char *server, int port);
static int post_pack(const char *host, int port, const char *page, int len, const char *content, char *data, int data_buf_len);
static SSL *ssl_init(int sockfd);
static int ssl_send(SSL *ssl, const char *data, int size);
static int ssl_recv(SSL *ssl, char *buff, int size);

int https_clt_post(char *host, int port, char *url, const char *data, int dsize, char *buff, int bsize)
{
    SSL *ssl = NULL;
    int re = 0;
    int sockfd;
    int data_len = 0;

    if (dsize >= POST_DATA_MAXLEN)
    {
        debug_error("invalid param data len %d \n", bsize);
        return -1;
    }
    char *sdata = sdata_buf;
    sockfd = client_connect_tcp(host, port);
    if (sockfd < 0)
    {
        debug_error("client_connect_tcp failed \n");
        return -1;
    }

    ssl = ssl_init(sockfd);
    if (ssl == NULL)
    {
        close(sockfd);
        return -1;
    }
    int sdata_len = (int)sizeof(sdata_buf);

    data_len = post_pack(host, port, url, dsize, data, sdata, sdata_len);
    re = ssl_send(ssl, sdata, data_len);
    if (re < 0)
    {
        close(sockfd);
        SSL_shutdown(ssl);
        debug_error("ssl_send failed \n");
        return -1;
    }

    int r_len = 0;
    r_len = ssl_recv(ssl, buff, bsize);
    if (r_len < 0)
    {
        close(sockfd);
        SSL_shutdown(ssl);
        debug_error("ssl_recv failed \n");
        return -1;
    }
    close(sockfd);
    SSL_shutdown(ssl);
    ERR_free_strings();

    return r_len;
}

static int ssl_recv(SSL *ssl, char *buff, int size)
{
    int i = 0;
    int len = 0;
    char headers[HTTP_HEADERS_MAXLEN];
    if (ssl == NULL)
    {
        debug_error("invalid param \n");
        return -1;
    }

    memset(headers, 0, sizeof(headers));
    // Headers以换行结束, 此处判断头是否传输完成
    while((len = SSL_read(ssl, headers, 1)) == 1)
    {
        if (i < 4)
        {
            if (headers[0] == '\r' || headers[0] == '\n')
            {
                i++;
                if ( i >= 4)
                {
                    break;
                }
            }
            else
            {
                i = 0;
            }
        }
    }

    len = SSL_read(ssl, buff, size);

    return len;
}


static int ssl_send(SSL *ssl, const char *data, int size)
{
    int ret = 0;
    int count = 0;

    ret = SSL_connect(ssl);
    if (1 != ret)
    {
        debug_error("SSL_connect failed \n");
        return -1;
    }

    while (count < size)
    {
        ret = SSL_write(ssl, data+count, size-count);
        if (ret == -1)
        {
            debug_error("SSL_write failed \n");
            return -1;
        }
        count += ret;
    }

    return count;
}

static SSL *ssl_init(int sockfd)
{
    int ret = 0;
    SSL *ssl;
    SSL_CTX *ctx;

    SSL_library_init();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL)
    {
        debug_error("SSL_CTX_new failed \n");
        return NULL;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        debug_error("SSL_new failed \n");
        return NULL;
    }

    ret = SSL_set_fd(ssl, sockfd);
    if (ret == 0)
    {
        SSL_free(ssl);
        debug_error("SSL_set_fd failed \n");
        return NULL;
    }

    RAND_poll();
    while (RAND_status() == 0)
    {
        unsigned short rand_ret = rand() % 65536;
        RAND_seed(&rand_ret, sizeof(rand_ret));
    }

    SSL_CTX_free(ctx);

    return ssl;
}

static int client_connect_tcp(char *server, int port)
{
    int sockfd;
    struct hostent *host;
    struct sockaddr_in cliaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        debug_error("create socket error\n");
        return -1;
    }

    if (NULL == (host = gethostbyname(server)))
    {
        debug_error("gethostbyname(%s) error!\n", server);
        return -1;
    }

    bzero(&cliaddr,sizeof(struct sockaddr));
    cliaddr.sin_family=AF_INET;
    cliaddr.sin_port=htons(port);
    cliaddr.sin_addr=*((struct in_addr *)host->h_addr);

    if (connect(sockfd, (struct sockaddr *)&cliaddr, sizeof(struct sockaddr)) < 0)
    {
        debug_error("connect error \n");
        return -1;
    }

    return sockfd;
}

static int post_pack(const char *host, int port, const char *page, int len, const char *content, char *data, int data_buf_len)
{
    int re_len = 0;
    int ret = -1;
    memset(post_buf, 0, sizeof(post_buf));

    sprintf(post_buf, "POST %s HTTP/1.0\r\n", page);
    sprintf(post_buf, "%sHost: %s:%d\r\n", post_buf, host, port);
    sprintf(post_buf, "%s%s", post_buf, https_post_headers);
    sprintf(post_buf, "%sContent-Length: %d\r\n\r\n", post_buf, len);
    sprintf(post_buf, "%s%s", post_buf, content);       // 此处需要修改, 当业务需要上传非字符串数据的时候, 会造成数据传输丢失或失败

    re_len = strlen(post_buf);
    if (data_buf_len > re_len+1)
    {
        memset(data, 0, data_buf_len);
        memcpy(data, post_buf, re_len);
        ret = re_len;
    }

    return ret;
}

//https://api.weixin.qq.com/sns/jscode2session?appid=wx4b02b7856ddaaf30&secret=8df65306d5813097bc6bbb00924370ba&js_code=023bHLaF0faidd2ApmbF0GhpaF0bHLaF&grant_type=authorization_code
int Port = 443;
char *Host = "api.weixin.qq.com";
char *Page = "/sns/jscode2session?";
char *Data = "appid=wx4b02b7856ddaaf30&secret=8df65306d5813097bc6bbb00924370ba&js_code=033bsfHH1g8iL108dAGH1EtmHH1bsfHF&grant_type=authorization_code";     // 对应字符串 - {"A":"111", "B":"222"}

int https_client_unit_test(void)
{
    int read_len = 0;
    char buff[1024] = {0};

    read_len = https_clt_post(Host, Port, Page, Data, strlen(Data), buff, 512);
    if(read_len < 0){
        printf("Err = %d \n", read_len);
        return read_len;
    }

    printf("==================== Recv [%d] ====================  buff %s \n", read_len, buff);
    printf("%s\n", buff);

    return 1;
}

