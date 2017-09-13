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
 * select实现的服务端,IO复用,不用多进程
 *
 * 用数组保存每个连接的socket
 *
 * 此时select监听的是: 1个监听socket ,n个连接的socket
 */
int do_select(int listenfd)
{

	int conn;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	int i;
	int client[FD_SETSIZE]; //自定义管理客户端连接的数组
	int maxi = 0;

	for (i = 0; i < FD_SETSIZE; i++)  //初始化为-1
		client[i] = -1;

	int nready;
	int maxfd = listenfd;  //为了得到select第一个参数需要的最大fd号

	fd_set allset;  //select 所有集合
	fd_set rset;   //select 监听的读集合

	FD_SET(listenfd, &allset);  //加入监听socket到select

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

		if (FD_ISSET(listenfd, &rset))  //有新连接来了
		{
			peerlen = sizeof(peeraddr);

			conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen); //获得新连接

			printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr),
					ntohs(peeraddr.sin_port));

			if (conn == -1)
			{
				ERR_EXIT("accept");
			}

			//处理管理多个客户端conn的数组
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

			//将新连接加入select进行监听
			FD_SET(conn, &allset);

			if (conn > maxfd)
				maxfd = conn;

			if (--nready <= 0)
				continue;
		}

		//开始处理每一个conn的通讯
		for (i = 0; i <= maxi; i++)
		{
			conn = client[i];
			if (conn == -1)
				continue;

			//printf("aaaaaaa:i=%d",i);

			if (FD_ISSET(conn, &rset)) //有conn可以进行通讯
			{
				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //自定义的read,读取报文

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

				writen(conn, recvbuf, strlen(recvbuf));  //echo 写报文

				if (--nready <= 0)
					break;

			}

		}

	}
	return 0;
}

//服务端 main
int main(void)
{

	signal(SIGCHLD, SIG_IGN); //避免客户端断开后产生僵尸进程

	int listenfd = tcp_server_create(1234);

	do_select(listenfd);

	close(listenfd);

	return 0;
}

