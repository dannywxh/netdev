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
 * epollʵ�ֵķ����,IO����
 */
void do_epoll(int listen_st)
{
	struct epoll_event ev, events[CLIENTCOUNT];	//����epoll_event�ṹ��ı���,ev����ע���¼�,�������ڻش�Ҫ������¼�
	active_nonblock(listen_st);						//��socket����Ϊ��������ʽ
	int epfd = epoll_create1(EPOLL_CLOEXEC);    // �������ڴ���accept��epollר�õ��ļ�������

	ev.data.fd = listen_st;							//������Ҫ������¼���ص��ļ�������
	ev.events = EPOLLIN |EPOLLET| EPOLLERR | EPOLLHUP;		//����Ҫ������¼�����
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_st, &ev); //**** ע��epoll�¼�

	int i = 0;
	int conn = 0;

	struct sockaddr_in peeraddr;
	socklen_t peerlen;

	while (1)
	{
		int nfds = epoll_wait(epfd, events, CLIENTCOUNT, -1); //***** �ȴ�epoll�¼��ķ���
		if (nfds == -1)
		{
			printf("epoll_wait failed %s\n", strerror(errno));
			break;
		}

		for (i = 0; i < nfds; i++)
		{
			if (events[i].data.fd < 0)
				continue;

			if (events[i].data.fd == listen_st) //��⵽һ��SOCKET�û����ӵ��˰󶨵�SOCKET�˿ڣ������µ����ӡ�
			{

				conn = accept(listen_st, (struct sockaddr*) &peeraddr,
						&peerlen); //���������
				if (conn == -1)
				{
					ERR_EXIT("accept");
				}

				if (conn >= 0)
				{
					active_nonblock(conn);
					ev.data.fd = conn;
					ev.events = EPOLLIN | EPOLLERR | EPOLLHUP; //����Ҫ������¼�����
					epoll_ctl(epfd, EPOLL_CTL_ADD, conn, &ev); //������client�˵�socket����������epoll
					continue;
				}
			}

			if (events[i].events & EPOLLIN) //socket�յ�����
			{
				conn = events[i].data.fd;

				char recvbuf[1024] = { 0 };
				int ret = readline(conn, recvbuf, 1024); //�Զ����read,��ȡ����
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

				writen(conn, recvbuf, strlen(recvbuf));  //echo д����

			}

			if (events[i].events & EPOLLERR) //socket����
			{
				conn = events[i].data.fd;
				//user_logout(st);
				events[i].data.fd = -1;

				printf("EPOLLERR");
			}

			if (events[i].events & EPOLLHUP) //socket���Ҷϡ�
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


//����� main
int main(void)
{

	signal(SIGCHLD, SIG_IGN); //����ͻ��˶Ͽ��������ʬ����

	int listenfd = tcp_server_create(1234);

	do_epoll(listenfd);

	close(listenfd);

	return 0;
}


