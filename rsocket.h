#ifndef _RSOCKET_H
#define _RSOCKET_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>

#define TABLE_MAX_SIZE 100
#define MSG_MAX_SIZE 100
#define T 2
#define SOCK_MRP 11
#define DROP_P 0.2

typedef struct{
	char msg[MSG_MAX_SIZE];
	int msg_len;
	struct sockaddr src;
}recv_buf;

typedef struct{
	short id;
}recv_msg_id;

typedef struct{
	short id;
	struct sockaddr dest;
	char msg[MSG_MAX_SIZE];
	int msg_len;
	int flag;
	time_t sent_at;
}unack_msg;


int r_socket(int domain, int type, int protocol);

int r_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int r_close(int sockfd);

#endif