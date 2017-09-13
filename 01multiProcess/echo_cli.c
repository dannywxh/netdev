/*
 * echo_cli.c
 *
 *  Created on: 2014-10-8
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

#include "pub.h"

/*
 * 客户端
 */
int main(void)
{
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");

	//指定服务端的地址
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(1234);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0)
		ERR_EXIT("connet");

	char sendbuf[1024] = { 0 };
	char recvbuf[1024] = { 0 };

	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{

		write(sock, sendbuf, strlen(sendbuf));

		int ret = read(sock, recvbuf, sizeof(recvbuf));
		if (ret == -1)
			ERR_EXIT("read");
		else if (ret <= 0)
		{
			printf("server closed");
			break;
		}

		fputs(recvbuf, stdout);
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);

	return 0;
}
