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
    > Created Time: 2018��08��29�� ������ 16ʱ42��21��
 ***********************************************************************/

#ifndef __HTTPS_POST__
#define __HTTPS_POST__

/*
 * @Name 			- HTTPS��POST�ύ
 * @Parame 	*host 	- ������ַ, ������
 * @Parame 	 port 	- �˿ں�, һ��Ϊ443
 * @Parame 	*url 	- url���·��
 * @Parame 	*data 	- Ҫ�ύ����������, ������Headers
 * @Parame 	 dsize 	- ��Ҫ���͵����ݰ���С, ���ⲿ���ô���, ������ͷ
 * @Parame 	*buff 	- ���ݻ���ָ��, �ǿ��������ǰmalloc
 * @Parame 	 bsize 	- ��Ҫ��ȡ�ķ��ؽ������, ���Ծ�������, ֱ����ȡ����
 *
 * @return 			- 	���ؽ������, �����ȡʧ��, �򷵻�ֵ <0
 * 						-1 : ΪPOST���������ڴ�ʧ��
 * 						-2 : ����TCP����ʧ��
 * 						-3 : SSL��ʼ�����sockfd��SSLʧ��
 *						-4 : POST�ύʧ��
 *						-5 : �ȴ���Ӧʧ��
 */
int https_post(char *host, int port, char *url, const char *data, int dsize, char *buff, int bsize);

int https_client_unit_test(void);

#endif


#ifdef __cplusplus
}
#endif

#endif // __HTTPS_CLIENT_H
