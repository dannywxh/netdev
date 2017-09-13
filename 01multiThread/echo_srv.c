/*
 * main.c
 *
 *  Created on: 2014-9-28
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

#include <signal.h>

#include <pthread.h>

#include "pub.h"

/* pthread���߳�ģ��
 *
 *
 * */

#define BUFSIZE 1024

void do_server(int conn)
{

	char recvbuf[1024] = { 0 };

	while (1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = read(conn, recvbuf, sizeof(recvbuf));

		if (ret == -1)
		{
			ERR_EXIT("read");
			break;

		} else if (ret == 0)
		{
			printf("client closed!");
			break;

		}
		fputs(recvbuf, stdout);
		write(conn, recvbuf, ret);
	}

	close(conn);
}

void *socket_thread_handler(void *arg) //�߳���ں���
{
	printf("thread is begin\n");
	int conn = *(int *) arg; //�õ�����client�˵�socket
	free((int *) arg);

	do_server(conn);

	printf("thread_is end\n");
	return NULL;
}

void socket_accept(int st)
{
	pthread_t thr_d;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //�����߳�Ϊ�ɷ���״̬
	int conn = 0;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	while (1) //ѭ��ִ��accept
	{
		memset(&client_addr, 0, sizeof(client_addr));
		//accept����������֪����client�����ӵ������accept���󷵻�
		conn = accept(st, (struct sockaddr *) &client_addr, &len);

		if (conn == -1)
		{
			printf("accept failed %s\n", strerror(errno));
			break; //accept����ѭ��break
		} else
		{
			char sIP[32];
			memset(sIP, 0, sizeof(sIP));
			//sockaddr_toa(&client_addr, sIP);
			printf("accept by %s\n", sIP);
			int *tmp = malloc(sizeof(int));
			*tmp = conn;
			{
				//������client�˵�socket��Ϊ����������һ���ɷ����߳�
				pthread_create(&thr_d, &attr, socket_thread_handler, tmp);
			}
		}

	}

	pthread_attr_destroy(&attr); //����
}

int main()
{

	signal(SIGCHLD, SIG_IGN); //����ͻ��˶Ͽ��������ʬ����

	int listenfd = tcp_server_create(1234);

	socket_accept(listenfd);

	return 0;
}
