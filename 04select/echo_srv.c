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

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pub.h"

//#define FD_SETSIZE 1024

/*
 * selectʵ�ֵķ����,IO����,���ö����
 *
 * �����鱣��ÿ�����ӵ�socket
 *
 * ��ʱselect��������: 1������socket ,n�����ӵ�socket
 */
int do_select(int listenfd)
{

	int conn;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	int i;
	int client[FD_SETSIZE]; //�Զ������ͻ������ӵ�����
	int maxi = 0;

	for (i = 0; i < FD_SETSIZE; i++)  //��ʼ��Ϊ-1
		client[i] = -1;

	int nready;
	int maxfd = listenfd;  //Ϊ�˵õ�select��һ��������Ҫ�����fd��

	fd_set allset;  //select ���м���
	fd_set rset;   //select �����Ķ�����

	FD_SET(listenfd, &allset);  //�������socket��select

	while (1)
	{
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (nready == -1)
		{
			if (errno == EINTR)
				continue;

			ERR_EXIT("select");
		}

		if (nready == 0)
			continue;

		if (FD_ISSET(listenfd, &rset))  //������������
		{
			peerlen = sizeof(peeraddr);

			conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen); //���������

			printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr),
					ntohs(peeraddr.sin_port));

			if (conn == -1)
			{
				ERR_EXIT("accept");
			}

			//����������ͻ���conn������
			for (i = 0; i < FD_SETSIZE; i++)
			{
				if (client[i] < 0)
				{
					client[i] = conn;
					if (i > maxi)
						maxi = i;
					break;
				}
			}

			if (i == FD_SETSIZE)
			{
				printf("too many connect!");
				exit(EXIT_SUCCESS);
			}

			//�������Ӽ���select���м���
			FD_SET(conn, &allset);

			if (conn > maxfd)
				maxfd = conn;

			if (--nready <= 0)
				continue;
		}

		//��ʼ����ÿһ��conn��ͨѶ
		for (i = 0; i <= maxi; i++)
		{
			conn = client[i];
			if (conn == -1)
				continue;

			//printf("aaaaaaa:i=%d",i);

			if (FD_ISSET(conn, &rset)) //��conn���Խ���ͨѶ
			{
				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //�Զ����read,��ȡ����

				if (ret == -1)
					ERR_EXIT("readline");

				if (ret == 0)
				{
					printf("client close\n");
					FD_CLR(conn, &allset);
					client[i] = -1;
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

	do_select(listenfd);

	close(listenfd);

	return 0;
}

