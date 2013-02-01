#include<cstdio>
#include<cstdlib>
#include<ctime>
#include<cassert>
#include<cmath>
#include<cstring>
#include<iostream>
#include<algorithm>
#include<numeric>
#include<vector>
#include<string>
#include<set>
#include<map>
#include<queue>
#include<list>
#include<sstream>
#include"csapp.h"
using namespace std;
#define LOOP(x,y,z) for((x)=(y);(x)<=(z);(x)++)
#define LOOPB(x,y,z) for((x)=(y);(x)<(z);(x)++)
#define RLOOP(x,y,z) for((x)=(y);(x)>=(z);(x)--)
#define RLOOPB(x,y,z) for((x)=(y);(x)>(z);(x)--)
#define ABS(x) ((x)<0?-(x):(x))
#define PI 3.1415926535898
#define READLENGTH MAXLINE
int i, j, k, a, m, n, s, t, l, tt, cas;
const int oo = 1 << 29;
template<class T> string i2s(T x) {
	ostringstream o;
	o << x;
	return o.str();
}
char tmp, str[500];
float f1, f2;
typedef struct {
	int fd;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
} request_arg;
typedef struct {
	int sfd;
	int cfd;
} read_from_server_arg;

void* read_from_server(void* ar) {
	Pthread_detach(Pthread_self());
	read_from_server_arg *arg = (read_from_server_arg*) ar;
	char buf[READLENGTH];
	int readsize;
	while ((readsize = recv(arg->sfd, buf, READLENGTH, 0)) > 0) {
		Rio_writen(arg->cfd, buf, readsize);
	}
	shutdown(arg->cfd, SHUT_RDWR);
	shutdown(arg->sfd, SHUT_RDWR);
	free(ar);
	printf("Server End.\n");
}
void* request(void* ar) {
	Pthread_detach(Pthread_self());
	printf("Accepted!\n");
	request_arg *arg = (request_arg*) ar;

	char buf[MAXLINE];
	int readsize;
	int x, i;
	readsize = recv(arg->fd, buf, 2, MSG_WAITALL);
	if (readsize < 2 || buf[0] != 5)
		Pthread_exit(NULL);
	//printf("1\n");
	x = buf[1];
	readsize = recv(arg->fd, buf, x, MSG_WAITALL);
	if (readsize != x)
		Pthread_exit(NULL);
	//printf("2\n");
	buf[0] = 5;
	buf[1] = 0;
	Rio_writen(arg->fd, buf, 2);
	readsize = recv(arg->fd, buf, 4, MSG_WAITALL);
	if (readsize < 2 || buf[0] != 5 || buf[1] != 1)
		Pthread_exit(NULL);
	//printf("3\n");
	int addrtype;
	uint16_t port;
	struct sockaddr_storage sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_storage));
	struct sockaddr_in *addrv4;
	struct sockaddr_in6 *addrv6;
	char* domain_name;
	switch (addrtype = buf[3]) {
	case 1:
		//ipv4
		addrv4 = (struct sockaddr_in *) &sockaddr;
		addrv4->sin_family = AF_INET;
		readsize = recv(arg->fd, &addrv4->sin_addr.s_addr, 4, MSG_WAITALL);
		if (readsize < 4)
			Pthread_exit(NULL);
		printf("IPV4 ADDR:%x\n", addrv4->sin_addr.s_addr);
		break;
	case 3:
		//domain name
		domain_name = (char*) Malloc(sizeof(char) * MAXLINE);
		readsize = recv(arg->fd, buf, 1, MSG_WAITALL);
		if (readsize < 1)
			Pthread_exit(NULL);
		x = buf[0];
		readsize = recv(arg->fd, domain_name, x, MSG_WAITALL);
		if (readsize < x)
			Pthread_exit(NULL);
		domain_name[x] = 0;
		printf("domain_name:%s\n", domain_name);
		if (inet_pton(AF_INET, domain_name, buf) == 1) {
			addrtype = 1;
			addrv4 = (struct sockaddr_in *) &sockaddr;
			addrv4->sin_family = AF_INET;
			memcpy(&addrv4->sin_addr.s_addr, buf, 4);
			printf("IPV4 ADDR:%x\n", addrv4->sin_addr.s_addr);
			break;
		} else if (inet_pton(AF_INET6, domain_name, buf) == 1) {
			addrtype = 4;
			addrv6 = (struct sockaddr_in6 *) &sockaddr;
			addrv6->sin6_family = AF_INET6;
			memcpy(&addrv6->sin6_addr, buf, 32);
			printf("IPV16 ADDR:%x %x %x %x\n",
					addrv6->sin6_addr.__u6.__s6_addr32[0],
					addrv6->sin6_addr.__u6.__s6_addr32[1],
					addrv6->sin6_addr.__u6.__s6_addr32[2],
					addrv6->sin6_addr.__u6.__s6_addr32[3]);

			break;
		} else {
			addrinfo *ai;
			if (getaddrinfo(domain_name, NULL, NULL, &ai) == 0) {
				if (ai->ai_family == AF_INET6)
					addrtype = 4;
				else if (ai->ai_family == AF_INET)
					addrtype = 1;
				else
					Pthread_exit(NULL);
				switch (addrtype) {
				case 1:
					addrv4 = (struct sockaddr_in *) &sockaddr;
					memcpy(&addrv4->sin_addr, &ai->ai_addr->sa_data[2],
							sizeof(struct in_addr));
					addrv4->sin_family = AF_INET;
					printf("IPV4 ADDR:%x\n", addrv4->sin_addr.s_addr);
					break;
				case 4:
					addrv6 = (struct sockaddr_in6 *) &sockaddr;
					memcpy(&addrv6->sin6_addr, &ai->ai_addr->sa_data[6],
							sizeof(struct in6_addr));
					addrv6->sin6_family = AF_INET6;
					printf("IPV16 ADDR:%x %x %x %x\n",
							addrv6->sin6_addr.__u6.__s6_addr32[0],
							addrv6->sin6_addr.__u6.__s6_addr32[1],
							addrv6->sin6_addr.__u6.__s6_addr32[2],
							addrv6->sin6_addr.__u6.__s6_addr32[3]);

					break;
				}
				freeaddrinfo(ai);
			} else {
				printf("invalid domain\n");
				Pthread_exit(NULL);
			}
		}
		break;
	case 4:
		//ipv6
		addrv6 = (struct sockaddr_in6 *) &sockaddr;
		addrv6->sin6_family = AF_INET6;
		readsize = recv(arg->fd, &addrv6->sin6_addr, 16, MSG_WAITALL);
		if (readsize < 16)
			Pthread_exit(NULL);
		printf("IPV16 ADDR:%x %x %x %x\n",
				addrv6->sin6_addr.__u6.__s6_addr32[0],
				addrv6->sin6_addr.__u6.__s6_addr32[1],
				addrv6->sin6_addr.__u6.__s6_addr32[2],
				addrv6->sin6_addr.__u6.__s6_addr32[3]);

		break;
	default:
		Pthread_exit(NULL);
	}
	readsize = recv(arg->fd, buf, 2, MSG_WAITALL);
	if (readsize < 2)
		Pthread_exit(NULL);
	port = ((buf[0] & 0xFF) << 8) + (buf[1] & 0xFF);
	char aa[1000];
	switch (addrtype) {
	case 1:
		addrv4->sin_port = htons(port);
		inet_ntop(AF_INET, &addrv4->sin_addr, aa, 16);
		printf("v4addr:%s,port:%d\n", aa, port);
		break;
	case 4:
		addrv6->sin6_port = htons(port);
		inet_ntop(AF_INET6, &addrv6->sin6_addr, aa, 128);
		printf("v6addr:%s,port:%d\n", aa, port);
		break;
	}
	int sfd = Socket(sockaddr.ss_family, SOCK_STREAM, 0);
	switch (addrtype) {
	case 1:
		Connect(sfd, (SA*) addrv4, sizeof(sockaddr_in));
		break;
	case 4:
		Connect(sfd, (SA*) &sockaddr, sizeof(sockaddr_in6));
		break;
	}

	buf[0] = 5;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = addrtype;
	switch (addrtype) {
	case 1:
		memcpy(&buf[4], &addrv4->sin_addr, 4);
		Rio_writen(arg->fd, buf, 8);
		break;
	case 4:
		memcpy(&buf[4], &addrv6->sin6_addr, 16);
		Rio_writen(arg->fd, buf, 20);
		break;
	}
	buf[0] = port >> 2;
	buf[1] = port & 0xFF;
	Rio_writen(arg->fd, buf, 2);

	rio_t srio;
	read_from_server_arg *rfs_arg = (read_from_server_arg*) Malloc(
			sizeof(read_from_server_arg));
	rfs_arg->sfd = sfd;
	rfs_arg->cfd = arg->fd;
	pthread_t tid;
	Pthread_create(&tid, NULL, read_from_server, rfs_arg);

	while ((readsize = recv(arg->fd, buf, READLENGTH, 0)) > 0) {
		Rio_writen(sfd, buf, readsize);
	}
	shutdown(sfd, SHUT_RDWR);
	shutdown(arg->fd, SHUT_RDWR);
	free(ar);

	printf("End.\n");
}

void* ipv6_listen_thread(void* ar) {

	Pthread_detach(Pthread_self());
	pthread_t tid;
	struct addrinfo hints, *res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, "12344", &hints, &res);
	int lfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	Bind(lfd, res->ai_addr, res->ai_addrlen);
	Listen(lfd, SOMAXCONN);
	while (1) {
		request_arg *arg = (request_arg *) Malloc(sizeof(request_arg));
		arg->addrlen = sizeof(arg->clientaddr);
		arg->fd = Accept(lfd, (SA *) &arg->clientaddr, &arg->addrlen);
		Pthread_create(&tid, NULL, request, arg);
	}
	return 0;
}

int main() {
	printf(
			"Simple IPv4&IPv6 dual stack socks5 server starting.\nAuthor: Zx.MYS\nVersion: 0.1\n\
Warning: This is not a stable release.\n");
	pthread_t tid, tmp;
	Pthread_create(&tmp, NULL, ipv6_listen_thread, NULL);
	struct addrinfo hints, *res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, "12344", &hints, &res);
	int lfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	Bind(lfd, res->ai_addr, res->ai_addrlen);
	Listen(lfd, SOMAXCONN);
	while (1) {
		request_arg *arg = (request_arg *) Malloc(sizeof(request_arg));
		arg->addrlen = sizeof(arg->clientaddr);
		arg->fd = Accept(lfd, (SA *) &arg->clientaddr, &arg->addrlen);
		Pthread_create(&tid, NULL, request, arg);
	}
	return 0;
}
