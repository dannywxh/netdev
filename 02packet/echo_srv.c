/*
 * srv2.c

 *
 *  Created on: 2014-9-30
 *      Author: Administrator
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pub.h"

/*
 *
 *
 * ͨ����װ�Զ������ʵ�ֽ��ճ������
 */
struct packet
{
	int len;
	char buf[1024];
};

//�����
void do_server(int conn)
{

	struct packet recvbuf;
	int n;
	while (1)
	{
		memset(&recvbuf, 0, sizeof(recvbuf));
		int ret = readn(conn, &recvbuf.len, 4); //����4���ֽ�,��ȡ���ĳ���
		if (ret == -1)
			ERR_EXIT("read");
		else if (ret < 4)
		{
			printf("client closed");
			break;
		}

		n = ntohl(recvbuf.len);

		ret = readn(conn, recvbuf.buf, n); //��ȡ���ݰ�
		if (ret == -1)
			ERR_EXIT("read");
		else if (ret < n)
		{
			printf("client closed");
			break;
		}

		fputs(recvbuf.buf, stdout);
		writen(conn, &recvbuf, 4 + n);
	}
}

//����� main
int main()
{

	signal(SIGCHLD, SIG_IGN); //����ͻ��˶Ͽ��������ʬ����

	int listenfd = tcp_server_create(1234);

	/* ����ͻ��˵�ַ */
	struct sockaddr_in peeraddr;
	//socklen_t peerlen = sizeof(peeraddr);

	int conn = 0;
	pid_t pid;

	while (1)
	{
		//	if ((conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen))
		//			< 0)

		conn = accept_timeout(listenfd, &peeraddr, 0);
		if (conn < 0)
			ERR_EXIT("accept");

		printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr),
				ntohs(peeraddr.sin_port));

		pid = fork();

		if (pid == 0)
		{
			close(listenfd);    //�ӽ��̹رռ���socket,ֻ����������socket
			do_server(conn);
			exit(EXIT_SUCCESS);  //do_server���غ���ζ�ſͻ����ѶϿ������,�ӽ�����Ҫ�ر���
		} else

			close(conn);  // ��������Ҫ�ر�����socket,ֻ�������

	}

	return 0;
}

