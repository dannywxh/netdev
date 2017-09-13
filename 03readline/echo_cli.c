/*
 * readline_cli.c
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

void do_cli(int socket)
{

	char sendbuf[1024] = { 0 };
	char recvbuf[1024] = { 0 };

	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) //此处会阻塞socket的响应,改为select模型
	{
		writen(socket, sendbuf, strlen(sendbuf));

		int ret = readline(socket, recvbuf, sizeof(recvbuf));

		if (ret == -1)
			ERR_EXIT("readline");
		else if (ret == 0)
		{
			printf("server closed!\n");
			break;
		}

		fputs(recvbuf, stdout);

		memset(&sendbuf, 0, sizeof(sendbuf));
		memset(&recvbuf, 0, sizeof(sendbuf));
	}

	close(socket);

}

void handle_sigpipe(int sig)
{

	printf("sig=%d\n", sig);
}

//客户端main
int main(void)
{

	signal(SIGPIPE, handle_sigpipe); //避免管道破裂导致进程退出

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
/*
	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(socklen_t);


	if (getsockname(sock, (struct sockaddr*) &localaddr, &addrlen) < 0)
		ERR_EXIT("getsockname");


	printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr),
			ntohs(localaddr.sin_port));
*/

	do_cli(sock);

	return 0;
}
