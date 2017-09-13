/*
 * pub.c
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
#include <fcntl.h>

#include "pub.h"

/* ����socket*/
int tcp_server_create(int port) //��������portָ���˿ںŵ�server��socket
{
	int st = socket(AF_INET, SOCK_STREAM, 0); //����TCP Socket
	int on = 1;
	if (setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		printf("setsockopt failed %s\n", strerror(errno));
		return 0;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		printf("bind port %d failed %s\n", port, strerror(errno));
		return 0;
	}

	if (listen(st, SOMAXCONN) < 0)
	{
		printf("listen failed %s\n", strerror(errno));
		return 0;
	}

	return st; //����listen��socket������
}

/*
 * �������û�����һ�ξͷ��͸�socket�Ļ�����
 *
 * ͨ��ѭ����ȡָ�����ȵ����ݵ�������
 * ��Ϊ������͵����ݱȽϴ�,��������״������,
 * ��Ҫ��֤��ȡ���ݵ�������
 *
 * */

ssize_t readn(int fd, void *buf, size_t count)
{

	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char*) buf;

	while (nleft > 0)
	{
		if ((nread = read(fd, bufp, nleft)) < 0) //<0��ʾ����
		{
			if (errno == EINTR)
				continue;
			return -1;
		}

		else if (nread == 0)  //=0��ʾ�Է��ر�
			return count - nleft;

		bufp += nread;
		nleft -= nread;
	}

	return count;  //�����ʾȫ������
}

ssize_t writen(int fd, void *buf, size_t count)
{

	size_t nleft = count;
	ssize_t nwritten;
	char *bufp = (char*) buf;

	while (nleft > 0)
	{
		if ((nwritten = write(fd, bufp, nleft)) < 0) //<0��ʾ����
		{
			if (errno == EINTR)
				continue;
			return -1;
		}

		else if (nwritten == 0)  //=0��ʾ�Է��ر�
			continue;

		bufp += nwritten;
		nleft -= nwritten;
	}

	return count;  //�����ʾȫ������
}

// ʵ��readline ���ճ��

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	while (1)
	{
		int ret = recv(sockfd, buf, len, MSG_PEEK); //����͵��,��ȡ��

		if (ret == -1 && errno == EINTR)
			continue;
		return ret;
	}
}

// ��ȡһ������,ͨ���ж��Ƿ���'\n'��ʵ��
ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = (char*) buf;
	int nleft = maxline;
	while (1)
	{
		ret = recv_peek(sockfd, bufp, nleft); //��һ�²��Ƴ�
		if (ret < 0)
			return ret;
		else if (ret == 0)
			return ret;

		nread = ret;
		int i;
		for (i = 0; i < nread; i++) //�ж��Ƿ���'\n'
		{
			if (bufp[i] == '\n')
			{
				ret = readn(sockfd, bufp, i + 1);
				if (ret != i + 1)
					exit(EXIT_FAILURE);

				return ret;
			}
		}

		if (nread > nleft)
			exit(EXIT_FAILURE);

		nleft -= nread;
		ret = readn(sockfd, bufp, nread);

		if (ret != nread)
			exit(EXIT_FAILURE);

		bufp += nread;
	}
	return -1;
}

/* ��select ��ʵ�ֳ�ʱ ,������ζ����������ʹ��selectģ��
 *
 * ����ʱ��accept
 *
 * �ɹ���δ��ʱ�����������ӵ��׽��֣���ʱ���أ�1����errno��ETIMEDOUT
 *
 * */

int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
	{
		fd_set accept_fdset;
		struct timeval timeout;
		FD_ZERO(&accept_fdset);
		FD_SET(fd, &accept_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_sec = 0;

		do
		{
			ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);

		} while (ret < 0 && errno == EINTR);
		if (ret == -1)
			return -1;
		else if (ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
	}

	if (addr != NULL)
		ret = accept(fd, (struct sockaddr*) addr, &addrlen);
	else
		ret = accept(fd, NULL, NULL);

	if (ret == -1)
		ERR_EXIT("accept");

	return ret;
}

/*
 * ���÷�����I/O
 */

void active_nonblock(int fd)
{
	int ret;
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
		ERR_EXIT("fcntl");

	flag |= O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flag);
	if (ret == -1)
		ERR_EXIT("fcntl");
}

/*
 * ��������I/O
 */

void deactive_nonblock(int fd)
{
	int ret;
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
		ERR_EXIT("fcntl");

	flag &= ~O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flag);
	if (ret == -1)
		ERR_EXIT("fcntl");
}

/*  *
 * ����ʱ��connect,����ӵ����ʱ����Ҫ
 *addr:Ҫ���ӵĶԷ��ĵ�ַ
 * �ɹ���δ��ʱ�����������ӵ��׽��֣���ʱ���أ�1����errno��ETIMEDOUT
 *
 * */

int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
		active_nonblock(fd); //���óɷ�����ģʽ

	ret = connect(fd, (struct sockaddr*) addr, addrlen);
	if (ret < 0 && errno == EINPROGRESS)
	{
		fd_set connect_fdset;
		struct timeval timeout;
		FD_ZERO(&connect_fdset);
		FD_SET(fd, &connect_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_sec = 0;

		do
		{
			//����д���ϣ�һ���������� ����д
			ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);

		} while (ret < 0 && errno == EINTR);

		if (ret == 0)

		{
			ret = -1;
			errno = ETIMEDOUT;
		}

		else if (ret < 0)
			ret = -1;

		else if (ret == 1)
		{
			//ret=1 ���ܳɹ���Ҳ����socket�������󣬴�����Ҫͨ��getsockopt����ȡ
			int err;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err,
					&socklen);

			if (sockoptret == -1)
				return -1;
			if (err == 0)
				ret = 0;
			else
			{
				errno = err;
				ret = -1;
			}
		}
	}

	if (wait_seconds > 0)
	{
		deactive_nonblock(fd);
	}
	return ret;
}

/*
 * ����ʱ��⣬����������
 *
 * δ��ʱ������0��ʧ�ܷ��أ�1����ʱ���أ�����errno��ETIMEDOUT

 */

int read_timeout(int fd, unsigned int wait_seconds)
{
	int ret = 0;
	if (wait_seconds > 0)
	{
		fd_set read_fdset;
		struct timeval timeout;

		FD_ZERO(&read_fdset);
		FD_SET(fd, &read_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd + 1, &read_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;

		} else if (ret == 1)
			ret = 0;
	}

	return ret;
}

/*
 * read_timeout ʹ�����Ӵ���
 */

/*
 int read_timeout_test()
 {
 int ret;
 ret = read_timeout(fd, 5);
 if (ret == 0)
 {
 // read(fd,...);
 } else if (ret == -1 && errno == ETIMREDOUT)
 {
 printf("time out\n");
 //...
 } else
 {
 ERR_EXIT("read_timeout");
 }
 }
 */
/*
 * д��ʱ��⣬����д����
 *
 * δ��ʱ������0��ʧ�ܷ��أ�1����ʱ���أ�����errno��ETIMEDOUT
 *
 *
 */

int write_timeout(int fd, unsigned int wait_seconds)
{
	int ret = 0;
	if (wait_seconds > 0)
	{
		fd_set write_fdset;
		struct timeval timeout;

		FD_ZERO(&write_fdset);
		FD_SET(fd, &write_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd + 1, NULL, NULL, &write_fdset, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;

		} else if (ret == 1)
			ret = 0;
	}

	return ret;
}

void setdaemon()
{
	pid_t pid, sid;
	pid = fork();
	if (pid < 0)
	{
		printf("fork failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	if ((sid = setsid()) < 0)
	{
		printf("setsid failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (chdir("/") < 0)
	{
		printf("chdir failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

}

