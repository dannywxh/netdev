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
 * readline实现的server
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

//服务端 main
int main()
{

	signal(SIGCHLD, SIG_IGN); //避免客户端断开后产生僵尸进程

	int listenfd = tcp_server_create(1234);

	/* 定义客户端地址 */
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
			close(listenfd);    //子进程关闭监听socket,只负责处理连接socket
			do_server(conn);
			exit(EXIT_SUCCESS);  //do_server返回后意味着客户端已断开或出错,子进程需要关闭了
		} else

			close(conn);  // 父进程需要关闭连接socket,只负责监听

	}

	return 0;
}

