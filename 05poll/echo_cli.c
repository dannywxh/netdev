/*
 * srv4.c
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
 * select 客户端的实现,
 *
 * 同时监听stdin和sockfd
 *
 * 如果不用select来管理,那么stdin将会阻塞sockfd,见srv3的客户端例子
 *
 * 对srv3的select改进
 */
void do_client(int socket)
{

	fd_set rset;
	FD_ZERO(&rset);

	int maxfd;
	int fd_stdin = fileno(stdin);

	if (fd_stdin > socket)
		maxfd = fd_stdin;
	else
		maxfd = socket;
	int nready;

	char sendbuf[1024] = { 0 };
	char recvbuf[1024] = { 0 };

	while (1)
	{
		FD_SET(fd_stdin, &rset);
		FD_SET(socket, &rset);
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (nready == -1)
			ERR_EXIT("select");

		if (nready == 0)
			continue;

		if (FD_ISSET(socket, &rset))//检测到socket有
		{
			int ret = readline(socket, recvbuf, sizeof(recvbuf));

			if (ret == -1)
				ERR_EXIT("readline");
			else if(ret == 0)
			{
				printf("server closed!\n");
				break;
			}

			fputs(recvbuf, stdout);
			memset(&recvbuf, 0, sizeof(sendbuf));
		}

		if (FD_ISSET(fd_stdin,&rset))//检测到stdin
		{
			if(fgets(sendbuf, sizeof(sendbuf), stdin)==NULL)
			{
				//break;
				// 此处不能用close(socket)，否则收不到echo信息，close()将双向管道关闭
				shutdown(socket,SHUT_WR);  //关闭单向通道
			}
			else
			{
			  writen(socket, sendbuf, strlen(sendbuf));
			  memset(&sendbuf, 0, sizeof(sendbuf));
			}
		}
	}

	//close(socket);

}



//客户端main
int main(void)
{

	signal(SIGPIPE, SIG_IGN); //避免管道破裂导致进程退出

	int sock;

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(1234);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0)
		ERR_EXIT("connet");


	do_client(sock);

	return 0;
}

