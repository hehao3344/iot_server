#ifndef __HTTPS_CLIENT_H
#define __HTTPS_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
    > File Name: https_post.h
    > Author: WangMinghang
    > Mail: hackxiaowang@qq.com
    > Blog: https://www.wangsansan.com
    > Created Time: 2018年08月29日 星期三 16时42分21秒
 ***********************************************************************/

#ifndef __HTTPS_POST__
#define __HTTPS_POST__

/*
 * @Name 			- HTTPS的POST提交
 * @Parame 	*host 	- 主机地址, 即域名
 * @Parame 	 port 	- 端口号, 一般为443
 * @Parame 	*url 	- url相对路径
 * @Parame 	*data 	- 要提交的数据内容, 不包括Headers
 * @Parame 	 dsize 	- 需要发送的数据包大小, 由外部调用传入, 不包含头
 * @Parame 	*buff 	- 数据缓存指针, 非空数组或提前malloc
 * @Parame 	 bsize 	- 需要读取的返回结果长度, 可以尽量给大, 直到读取结束
 *
 * @return 			- 	返回结果长度, 如果读取失败, 则返回值 <0
 * 						-1 : 为POST数据申请内存失败
 * 						-2 : 建立TCP连接失败
 * 						-3 : SSL初始化或绑定sockfd到SSL失败
 *						-4 : POST提交失败
 *						-5 : 等待响应失败
 */
int https_post(char *host, int port, char *url, const char *data, int dsize, char *buff, int bsize);

int https_client_unit_test(void);

#endif


#ifdef __cplusplus
}
#endif

#endif // __HTTPS_CLIENT_H
