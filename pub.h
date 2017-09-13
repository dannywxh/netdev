/*
 * pub.h
 *
 *  Created on: 2014-9-28
 *      Author: Administrator
 */

#ifndef PUB_H_
#define PUB_H_

#define ERR_EXIT(m)\
	do\
	{\
      perror(m);\
      exit(EXIT_FAILURE);\
    }\
    while(0)



int tcp_server_create(int port); //创建参数port指定端口号的server端socket

ssize_t readn(int fd, void *buf, size_t count);

ssize_t writen(int fd, void *buf, size_t count);

// 实现readline 解决粘包
ssize_t recv_peek(int sockfd, void *buf, size_t len);

//读取一行数据,通过判断是否有'\n'来实现
ssize_t readline(int sockfd, void *buf, size_t maxline);

int read_timeout(int fd, unsigned int wait_seconds);

int write_timeout(int fd, unsigned int wait_seconds);

int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

void active_nonblock(int socket);

void deactive_nonblock(int socket);


void setdaemon();
#endif /* PUB_H_ */
