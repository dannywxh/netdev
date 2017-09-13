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

/*
 * 客户端
 */
void do_client(int conn)
{

	struct packet sendbuf;
	struct packet recvbuf;
	memset(&sendbuf, 0, sizeof(sendbuf));
	memset(&recvbuf, 0, sizeof(sendbuf));

	int n;
	while (fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL)
	{
		n = strlen(sendbuf.buf);
		sendbuf.len = htonl(n);
		writen(conn, &sendbuf,4+n); //4是packet的len字段

		int ret = readn(conn, &recvbuf.len, 4);
		if (ret == -1)
			ERR_EXIT("read");
		else if (ret < 4)
		{
			printf("client closed");
			break;
		}

		n = ntohl(recvbuf.len);

		ret = readn(conn, recvbuf.buf, n);
		if (ret == -1)
			ERR_EXIT("read");
		else if (ret < n)
		{
			printf("client closed");
			break;
		}

		fputs(recvbuf.buf, stdout);
		memset(&sendbuf, 0, sizeof(sendbuf));
		memset(&recvbuf, 0, sizeof(recvbuf));
	}
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


