#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <core/core.h>

#include <core/core.h>
/************************************************************************
    > File Name: https_post.c
    > Author: WangMinghang
    > Mail: hackxiaowang@qq.com
    > Blog: https://www.wangsansan.com
    > Created Time: 2018��08��29�� ������ 16ʱ42��21��
 ***********************************************************************/

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

#include "https_client.h"

#define HTTP_HEADERS_MAXLEN 	512 	// Headers ����󳤶�

/*
 * Headers �������
 */
const char *HttpsPostHeaders = "User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
								"Cache-Control: no-cache\r\n"
								"Accept: */*\r\n"
								"Content-type: application/json\r\n";

/*
 * @Name 			- ����TCP����, ������������
 * @Parame *server 	- �ַ���, Ҫ���ӵķ�������ַ, ����Ϊ����, Ҳ����ΪIP��ַ
 * @Parame 	port 	- �˿�
 *
 * @return 			- ���ض�Ӧsock�������, ���ڿ��ƺ���ͨ��
 */
int client_connect_tcp(char *server, int port)
{
	int sockfd;
	struct hostent *host;
	struct sockaddr_in cliaddr;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0){
		perror("create socket error");
		return -1;
	}

	if(!(host=gethostbyname(server))){
		printf("gethostbyname(%s) error!\n", server);
		return -2;
	}

	bzero(&cliaddr,sizeof(struct sockaddr));
	cliaddr.sin_family=AF_INET;
	cliaddr.sin_port=htons(port);
	cliaddr.sin_addr=*((struct in_addr *)host->h_addr);

	if(connect(sockfd,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr))<0){
		perror("[-] error");
		return -3;
	}

	return(sockfd);
}

/*
 * @Name 			- ��װpost���ݰ���headers
 * @parame *host 	- ������ַ, ����
 * @parame  port 	- �˿ں�
 * @parame 	page 	- url���·��
 * @parame 	len 	- �������ݵĳ���
 * @parame 	content - ��������
 * @parame 	data 	- �õ���װ�����ݽ��
 *
 * @return 	int 	- ���ط�װ�õ������ݳ���
 */
int post_pack(const char *host, int port, const char *page, int len, const char *content, char *data)
{
	int re_len = strlen(page) + strlen(host) + strlen(HttpsPostHeaders) + len + HTTP_HEADERS_MAXLEN;

	char *post = NULL;
	post = malloc(re_len);
	if(post == NULL){
		return -1;
	}

	sprintf(post, "POST %s HTTP/1.0\r\n", page);
	sprintf(post, "%sHost: %s:%d\r\n",post, host, port);
	sprintf(post, "%s%s", post, HttpsPostHeaders);
	sprintf(post, "%sContent-Length: %d\r\n\r\n", post, len);
	sprintf(post, "%s%s", post, content); 		// �˴���Ҫ�޸�, ��ҵ����Ҫ�ϴ����ַ������ݵ�ʱ��, ��������ݴ��䶪ʧ��ʧ��

	re_len = strlen(post);
	memset(data, 0, re_len+1);
	memcpy(data, post, re_len);

	free(post);
	return re_len;
}

/*
 * @Name 		- 	��ʼ��SSL, ���Ұ�sockfd��SSL
 * 					��������ҪĿ����ͨ��SSL������sock
 *
 * @return 		- 	��������ɳ�ʼ�����󶨶�Ӧsockfd��SSLָ��
 */
SSL *ssl_init(int sockfd)
{
	int re = 0;
	SSL *ssl;
	SSL_CTX *ctx;

	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL){
		return NULL;
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL){
		return NULL;
	}

	/* ��socket��SSL���� */
	re = SSL_set_fd(ssl, sockfd);
	if (re == 0){
		SSL_free(ssl);
		return NULL;
	}

    /*
     * ������, WIN32��ϵͳ��, ���ܺ���Ч�Ĳ��������, �˴��������������
     */
	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	/*
     * ctxʹ�����, �����ͷ�
     */
	SSL_CTX_free(ctx);

	return ssl;
}

/*
 * @Name 			- ͨ��SSL�������Ӳ���������
 * @Parame 	*ssl 	- SSLָ��, �Ѿ���ɳ�ʼ�������˶�Ӧsock�����SSLָ��
 * @Parame 	*data 	- ׼���������ݵ�ָ���ַ
 * @Parame 	 size 	- ׼�����͵����ݳ���
 *
 * @return 			- ���ط�����ɵ����ݳ���, �������ʧ��, ���� -1
 */
int ssl_send(SSL *ssl, const char *data, int size)
{
	int re = 0;
	int count = 0;

	re = SSL_connect(ssl);

	if(re != 1){
		return -1;
	}

	while(count < size)
	{
		re = SSL_write(ssl, data+count, size-count);
		if(re == -1){
			return -2;
		}
		count += re;
	}

	return count;
}

/*
 * @Name 			- SSL��������, ��Ҫ�Ѿ���������
 * @Parame 	*ssl 	- SSLָ��, �Ѿ���ɳ�ʼ�������˶�Ӧsock�����SSLָ��
 * @Parame  *buff 	- �������ݵĻ�����, �ǿ�ָ��
 * @Parame 	 size 	- ׼�����յ����ݳ���
 *
 * @return 			- ���ؽ��յ������ݳ���, �������ʧ��, ����ֵ <0
 */
int ssl_recv(SSL *ssl, char *buff, int size)
{
	int i = 0; 				// ��ȡ����ȡ��������, ���ж�headers�Ƿ����
	int re;
	int len = 0;
	char headers[HTTP_HEADERS_MAXLEN];

	if(ssl == NULL){
		return -1;
	}

	// Headers�Ի��н���, �˴��ж�ͷ�Ƿ������
	while((len = SSL_read(ssl, headers, 1)) == 1)
	{
		if(i < 4){
			if(headers[0] == '\r' || headers[0] == '\n'){
				i++;
				if(i>=4){
					break;
				}
			}else{
				i = 0;
			}
		}
		//printf("%c", headers[0]);		// ��ӡHeaders
	}

	len = SSL_read(ssl, buff, size);
	return len;
}

int https_post(char *host, int port, char *url, const char *data, int dsize, char *buff, int bsize)
{
	SSL *ssl;
	int re = 0;
	int sockfd;
	int data_len = 0;
	int ssize = dsize + HTTP_HEADERS_MAXLEN; 	// �����͵����ݰ���С

	char *sdata = malloc(ssize);
	if(sdata == NULL){
		return -1;
	}

	// 1������TCP����
	sockfd = client_connect_tcp(host, port);
	if(sockfd < 0){
		free(sdata);
		return -2;
	}

	// 2��SSL��ʼ��, ����Socket��SSL
	ssl = ssl_init(sockfd);
	if(ssl == NULL){
		free(sdata);
		close(sockfd);
		return -3;
	}

	// 3�����POST����
	data_len = post_pack(host, port, url, dsize, data, sdata);

	// 4��ͨ��SSL��������
	re = ssl_send(ssl, sdata, data_len);
	if(re < 0){
		free(sdata);
		close(sockfd);
		SSL_shutdown(ssl);
		return -4;
	}

	// 5��ȡ������
	int r_len = 0;
	r_len = ssl_recv(ssl, buff, bsize);
	if(r_len < 0){
		free(sdata);
		close(sockfd);
		SSL_shutdown(ssl);
		return -5;
	}

	// 6���رջỰ, �ͷ��ڴ�
	free(sdata);
	close(sockfd);
	SSL_shutdown(ssl);
	ERR_free_strings();

	return r_len;
}

//https://api.weixin.qq.com/sns/jscode2session?appid=wx4b02b7856ddaaf30&secret=8df65306d5813097bc6bbb00924370ba&js_code=023bHLaF0faidd2ApmbF0GhpaF0bHLaF&grant_type=authorization_code
int Port = 443;
char *Host = "api.weixin.qq.com";
char *Page = "/sns/jscode2session?";
char *Data = "appid=wx4b02b7856ddaaf30&secret=8df65306d5813097bc6bbb00924370ba&js_code=033bsfHH1g8iL108dAGH1EtmHH1bsfHF&grant_type=authorization_code"; 	// ��Ӧ�ַ��� - {"A":"111", "B":"222"}

int https_client_unit_test(void)
{
	int read_len = 0;
	char buff[1024] = {0};

	read_len = https_post(Host, Port, Page, Data, strlen(Data), buff, 512);
	if(read_len < 0){
		printf("Err = %d \n", read_len);
		return read_len;
	}

	printf("==================== Recv [%d] ====================  buff %s \n", read_len, buff);
	printf("%s\n", buff);

	return 1;
}

