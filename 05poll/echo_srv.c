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
 * poll实现的服务端,IO复用,不用多进程
 *
 * 用数组保存每个连接的socket
 * 重点需要struct pollfd
 *
 * 突破1024
 *
 */

int do_poll(int listenfd)
{

	int conn;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	int i;
	struct pollfd client[2048]; //自定义管理客户端连接的数组

	int maxi = 0;

	for (i = 0; i < 2048; i++)  //初始化为-1
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

		if (client[0].revents & POLLIN) //有新连接来了
		{
			peerlen = sizeof(peeraddr);

			conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen); //获得新连接
			if (conn == -1)
			{
				ERR_EXIT("accept");
			}

			//处理管理多个客户端conn的数组
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

		//开始处理每一个conn的通讯
		for (i = 1; i <= maxi; i++)
		{
			conn = client[i].fd;
			if (conn == -1)
				continue;

			if (client[i].events && POLLIN) //有conn可以进行通讯
			{
				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //自定义的read,读取报文
				if (ret == -1)
					ERR_EXIT("readline");

				if (ret == 0)
				{
					printf("client close\n");
					client[i].fd = -1;
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

	do_poll(listenfd);

	close(listenfd);

	return 0;
}

