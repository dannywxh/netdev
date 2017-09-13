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
 * readlineʵ�ֵ�server
 */
int do_server(int socket)
{
	char recvbuf[1024];
	while (1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = readline(socket, recvbuf, 1024);

		if (ret == -1)
			ERR_EXIT("readline");

		if (ret == 0)
		{
			printf("client closed\n");
			break;
		}

		fputs(recvbuf, stdout);
		writen(socket, recvbuf, strlen(recvbuf));
	}
	close(socket);
	return 0;
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

