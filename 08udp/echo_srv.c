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


void echo_srv(int sock)
{

	char recvbuf[1024]={0};
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int n;
	while(1)
	{
		peerlen=sizeof(peeraddr);
		memset(recvbuf,0,sizeof(recvbuf));
		n=recvfrom(sock,recvbuf,sizeof(recvbuf),0,(struct sockaddr*)&peeraddr,&peerlen);
		if(n==1)
		{
			if(errno==EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}
		else if(n>0)
		{
			fputs(recvbuf,stdout);
            sendto(sock,recvbuf,n,0,(struct sockaddr*)&peeraddr,peerlen);

		}

	}

}


int main(void)
{

	int st = socket(PF_INET, SOCK_DGRAM, 0); //´´½¨UDP Socket

	int port=1234;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
		{
			printf("bind port %d failed %s\n", port, strerror(errno));
			return 0;
		}


	 echo_srv(st);
	 return 0;


}




