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
 * 通过封装自定义包来实现解决粘包问题
 */
struct packet
{
	int len;
	char buf[1024];
};

//服务端
void do_server(int conn)
{

	struct packet recvbuf;
	int n;
	while (1)
	{
		memset(&recvbuf, 0, sizeof(recvbuf));
		int ret = readn(conn, &recvbuf.len, 4); //先收4个字节,获取包的长度
		if (ret == -1)
			ERR_EXIT("read");
		else if (ret < 4)
		{
			printf("client closed");
			break;
		}

		n = ntohl(recvbuf.len);

		ret = readn(conn, recvbuf.buf, n); //读取数据包
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

