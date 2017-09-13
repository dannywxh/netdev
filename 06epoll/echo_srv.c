/*
 * Srv.c
 *
 *  Created on: 2014-9-17
 *      Author: Administrator
 */

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pub.h"

#define CLIENTCOUNT 1024


/*
 *
 * epoll实现的服务端,IO复用
 */
void do_epoll(int listen_st)
{
	struct epoll_event ev, events[CLIENTCOUNT];	//声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
	active_nonblock(listen_st);						//把socket设置为非阻塞方式
	int epfd = epoll_create1(EPOLL_CLOEXEC);    // 生成用于处理accept的epoll专用的文件描述符

	ev.data.fd = listen_st;							//设置与要处理的事件相关的文件描述符
	ev.events = EPOLLIN |EPOLLET| EPOLLERR | EPOLLHUP;		//设置要处理的事件类型
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_st, &ev); //**** 注册epoll事件

	int i = 0;
	int conn = 0;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	while (1)
	{
		int nfds = epoll_wait(epfd, events, CLIENTCOUNT, -1); //***** 等待epoll事件的发生
		if (nfds == -1)
		{
			printf("epoll_wait failed %s\n", strerror(errno));
			break;
		}

		for (i = 0; i < nfds; i++)
		{
			if (events[i].data.fd < 0)
				continue;

			if (events[i].data.fd == listen_st) //监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
			{

				conn = accept(listen_st, (struct sockaddr*) &peeraddr,
						&peerlen); //获得新连接
				if (conn == -1)
				{
					ERR_EXIT("accept");
				}

				if (conn >= 0)
				{
					active_nonblock(conn);
					ev.data.fd = conn;
					ev.events = EPOLLIN | EPOLLERR | EPOLLHUP; //设置要处理的事件类型
					epoll_ctl(epfd, EPOLL_CTL_ADD, conn, &ev); //将来自client端的socket描述符加入epoll
					continue;
				}
			}

			if (events[i].events & EPOLLIN) //socket收到数据
			{
				conn = events[i].data.fd;

				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //自定义的read,读取报文
				if (ret == -1)
				{
					events[i].data.fd = -1;
					printf("readline error");

				} else if (ret == 0)
				{
					printf("client closed!\n");
					events[i].data.fd = -1;
				}

				fputs(recvbuf, stdout);

				writen(conn, recvbuf, strlen(recvbuf));  //echo 写报文

			}

			if (events[i].events & EPOLLERR) //socket错误。
			{
				conn = events[i].data.fd;
				//user_logout(st);
				events[i].data.fd = -1;

				printf("EPOLLERR");
			}

			if (events[i].events & EPOLLHUP) //socket被挂断。
			{
				conn = events[i].data.fd;
				//user_logout(st);
				events[i].data.fd = -1;

				printf("EPOLLHUP");
			}
		}
	}
	close(epfd);
}


//服务端 main
int main(void)
{

	signal(SIGCHLD, SIG_IGN); //避免客户端断开后产生僵尸进程

	int listenfd = tcp_server_create(1234);

	do_epoll(listenfd);

	close(listenfd);

	return 0;
}


