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

#include <sys/wait.h>
#include <sys/epoll.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <algorithm>

#include "pub.h"

#define CLIENTCOUNT 1024

/*
 * c++�̳�����c++ʵ�֣���Ҫg++����
 */

typedef std::vector<struct epoll_event> EventList;
int do_epoll(int listenfd)
{

	std::vector<int> clients;
	int epollfd;
	epollfd = epoll_create1(EPOLL_CLOEXEC);

	struct epoll_event event; //��Ҫ�������Ч�Ļ�����������windows��overlap �ṹ
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET; //��Ե����
	epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

	EventList events(16);
	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	int conn;

	int count = 0;

	int i;
	int nready;

	while (1)
	{
		nready = epoll_wait(epollfd, &*events.begin(),
				static_cast<int>(events.size()), -1);

		if (nready == -1)
		{
			if (errno == EINTR)
				continue;

			ERR_EXIT("epoll_wait");
		}

		if (nready == 0)
			continue;

		if ((size_t) nready == events.size())
			events.resize(events.size() * 2);

		for (i = 0; i < nready; i++)
		{
			if (events[i].data.fd == listenfd) //��������׽���
			{
				peerlen = sizeof(peeraddr);
				conn = accept(listenfd, (struct sockaddr*) &peeraddr, &peerlen);

				if (conn == -1)
					ERR_EXIT("accept");

				printf("ip=%s prot=%d\n", inet_ntoa(peeraddr.sin_addr),
						ntohs(peeraddr.sin_port));

				printf("count=%d\n", ++count);

				clients.push_back(conn);
				active_nonblock(conn);

				event.data.fd = conn;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &event);

			} else if (events[i].events & EPOLLIN) //���������׽���
			{
				conn = events[i].data.fd;
				if (conn < 0)
					continue;

				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //�Զ����read,��ȡ����
				if (ret == -1)
					ERR_EXIT("readline");

				if (ret == 0)
				{

					printf("client close\n");

					close(conn);
					event = events[i];
					epoll_ctl(epollfd, EPOLL_CTL_DEL, conn, &event);
					clients.erase(
							std::remove(clients.begin(), clients.end(), conn),
							clients.end());

				}

				fputs(recvbuf, stdout);

				writen(conn, recvbuf, strlen(recvbuf));  //echo д����

			}
		}
	}

}

//����� main
int main(void)
{

	signal(SIGCHLD, SIG_IGN); //����ͻ��˶Ͽ��������ʬ����

	int listenfd = tcp_server_create(1234);

	do_epoll(listenfd);

	close(listenfd);

	return 0;
}
