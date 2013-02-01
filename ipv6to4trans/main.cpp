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

char ipv4address[200];
char ipv4port[200];
char listenport[200];

typedef struct {
	int fd;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
} request_arg;
typedef struct {
	int sfd;
	int cfd;
	int client_sent_size;
	int server_sent_size;
	pthread_t tid;
	sem_t sem;
	int closed;
} read_from_server_arg;

void* read_from_server(void* ar) {
	Pthread_detach(Pthread_self());
	read_from_server_arg *arg = (read_from_server_arg*) ar;
	int* totsize = &arg->server_sent_size;
	pthread_t tid = arg->tid;
	char buf[READLENGTH];
	int readsize;
	while ((readsize = recv(arg->sfd, buf, READLENGTH, 0)) > 0) {
		//totsize += readsize;
		Rio_writen(arg->cfd, buf, readsize);
	}
	if (shutdown(arg->cfd, SHUT_RDWR))
		;
	if (shutdown(arg->sfd, SHUT_RDWR))
		;
	flockfile(stdout);
	printf("%d:Server End.\n", tid);
	funlockfile(stdout);
	free(ar);
}
void* request(void* ar) {

	Pthread_detach(Pthread_self());
	request_arg *arg = (request_arg*) ar;
	rio_t rio;
	flockfile(stdout);
	printf("%d:Accepted!\n", Pthread_self());
	funlockfile(stdout);
	struct sockaddr_storage sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_storage));
	struct sockaddr_in *addrv4;
	char buf[READLENGTH];
	char *domain_name = ipv4address;
	if (inet_pton(AF_INET, domain_name, buf) == 1) {
		addrv4 = (struct sockaddr_in *) &sockaddr;
		memcpy(&addrv4->sin_addr.s_addr, buf, 4);
		addrv4->sin_family = AF_INET;
	} else {
		addrinfo *ai;
		if (getaddrinfo(domain_name, NULL, NULL, &ai) == 0) {
			if (ai->ai_family == AF_INET6){
				printf("IPV6 DOMAIN!");
				Pthread_exit(NULL);
			}else if (ai->ai_family == AF_INET) {
			
			}
			addrv4 = (struct sockaddr_in *) &sockaddr;
			memcpy(&addrv4->sin_addr, &ai->ai_addr->sa_data[2],
					sizeof(struct in_addr));
			addrv4->sin_family = AF_INET;
			freeaddrinfo(ai);
		} else {
			printf("invalid domain\n");
			Pthread_exit(NULL);
		}
	}

	addrv4->sin_port = htons(atoi(ipv4port));
	//printf("1\n");
	int sfd = Socket(sockaddr.ss_family, SOCK_STREAM, 0);
	//printf("2\n");
	char aa[1000];
	inet_ntop(AF_INET, &addrv4->sin_addr, aa, 16);
	//printf("3\n");
	//printf("v6addr:%s\n", aa);
	Connect(sfd, (SA*) addrv4, sizeof(sockaddr_in));
	read_from_server_arg *rfs_arg = (read_from_server_arg*) Malloc(
			sizeof(read_from_server_arg));
	rfs_arg->sfd = sfd;
	rfs_arg->cfd = arg->fd;
	rfs_arg->server_sent_size = 0;
	rfs_arg->tid = Pthread_self();
	rfs_arg->client_sent_size = 0;
	rfs_arg->closed = 0;
	pthread_t tid;
	Pthread_create(&tid, NULL, read_from_server, rfs_arg);

	int* totsize = &rfs_arg->client_sent_size;
	int readsize;
	while ((readsize = recv(arg->fd, buf, READLENGTH, 0)) > 0) {
		Rio_writen(sfd, buf, readsize);
		//*totsize+=readsize;
	}
	if (shutdown(sfd, SHUT_RDWR))
		;
	if (shutdown(arg->fd, SHUT_RDWR))
		;
	//printf("6\n");
	flockfile(stdout);
	printf("%d:Client End.\n", Pthread_self());
	funlockfile(stdout);
	free(ar);
}

int main(int argc, char** argv) {
	printf(
			"Simple 6to4 transferrer starting.\nAuthor: Zx.MYS\nVersion: 0.11\nWarning: This is not a stable release.\n\
Please make sure the arguments are correct.\n");
	pthread_t tid, tmp;
	sscanf(argv[1], "%s", &listenport);
	sscanf(argv[2], "%s", &ipv4address);
	sscanf(argv[3], "%s", &ipv4port);

	struct addrinfo hints, *res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, listenport, &hints, &res);
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
