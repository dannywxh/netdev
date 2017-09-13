/*
 * select_srv.c
 *
 *  Created on: 2014-10-8
 *      Author: Administrator
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <poll.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pub.h"

/*
 * pollʵ�ֵķ����,IO����,���ö����
 *
 * �����鱣��ÿ�����ӵ�socket
 * �ص���Ҫstruct pollfd
 *
 * ͻ��1024
 *
 */

int do_poll(int listenfd)
{

	int conn;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	int i;
	struct pollfd client[2048]; //�Զ������ͻ������ӵ�����

	int maxi = 0;

	for (i = 0; i < 2048; i++)  //��ʼ��Ϊ-1
		client[i].fd = -1;

	int nready;
	client[0].fd = listenfd;
	client[0].events = POLLIN;
	while (1)
	{

		nready = poll(client, maxi + 1, -1);
		if (nready == -1)
		{
			if (errno == EINTR)
				continue;

			ERR_EXIT("select");
		}

		if (nready == 0)
			continue;

		if (client[0].revents & POLLIN) //������������
		{
			peerlen = sizeof(peeraddr);

			conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen); //���������
			if (conn == -1)
			{
				ERR_EXIT("accept");
			}

			//����������ͻ���conn������
			for (i = 1; i < 2048; i++)
			{
				if (client[i].fd < 0)
				{
					client[i].fd = conn;
					client[i].events = POLLIN;
					if (i > maxi)
						maxi = i;
					break;
				}
			}

			if (i == 2048)
			{
				printf("too many connect!");
				exit(EXIT_SUCCESS);
			}



			if (--nready <= 0)
				continue;
		}

		//��ʼ����ÿһ��conn��ͨѶ
		for (i = 1; i <= maxi; i++)
		{
			conn = client[i].fd;
			if (conn == -1)
				continue;

			if (client[i].events && POLLIN) //��conn���Խ���ͨѶ
			{
				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //�Զ����read,��ȡ����
				if (ret == -1)
					ERR_EXIT("readline");

				if (ret == 0)
				{
					printf("client close\n");
					client[i].fd = -1;
					close(conn);
				}

				fputs(recvbuf, stdout);

				writen(conn, recvbuf, strlen(recvbuf));  //echo д����

				if (--nready <= 0)
					break;

			}

		}

	}
	return 0;
}

//����� main
int main(void)
{

	signal(SIGCHLD, SIG_IGN); //����ͻ��˶Ͽ��������ʬ����

	int listenfd = tcp_server_create(1234);

	do_poll(listenfd);

	close(listenfd);

	return 0;
}

