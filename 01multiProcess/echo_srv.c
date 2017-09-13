/*
 * main.c
 *
 *  Created on: 2014-9-28
 *      Author: Administrator
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <signal.h>

#include "pub.h"



void do_server(int conn)
{

	char recvbuf[1024] = { 0 };

	while (1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = read(conn, recvbuf, sizeof(recvbuf));

		if (ret == -1)
		{
			ERR_EXIT("read");
			break;


		} else if (ret == 0)
		{
			printf("client closed!");

		}
		fputs(recvbuf, stdout);
		write(conn, recvbuf, ret);
	}

	close(conn);
}

/* fork �����ģ��
 *
 *
 * */
int main()
{

	signal(SIGCHLD,SIG_IGN); //����ͻ��˶Ͽ��������ʬ����

	int listenfd = tcp_server_create(1234);

	/* ����ͻ��˵�ַ */
	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

	int conn = 0;
	pid_t pid;

	while (1)
	{
		if ((conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen))
				< 0)
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
