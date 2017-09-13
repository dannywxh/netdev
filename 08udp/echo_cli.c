/*
 * udp.c
 *
 *  Created on: 2014-10-4
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

void echo_cli(int sock)
{

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //server 端的地址

   /* connect 不是必须，但是调用可以解决异步错误的响应
    * 此时client的套接字只能发送给指定的server，即addr的地址
    * */
	connect(sock,(struct sockaddr*)&addr,sizeof(addr));

	int ret;
	char sendbuf[1024] = { 0 };
	char recvbuf[1024] = { 0 };
	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		 /* 如果之前调用过connect（），可以这么写,甚至调用send发送数据*/
		//sendto(sock, sendbuf, strlen(sendbuf), 0, NULL,0);
		//send(sock,sendbuf,strlen(sendbuf),0);
		sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*) &addr,
						sizeof(addr));

		ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);

		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}

		fputs(recvbuf, stdout);
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}
}


int main(void)
{

	int st = socket(PF_INET, SOCK_DGRAM, 0); //创建UDP Socket

	echo_cli(st);

	return 0;

}


