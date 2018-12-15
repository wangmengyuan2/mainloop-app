#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "mainloop.h"

#define MAX_EVENTS	128
#define PORT		8888 
#define BUF_SIZE	1024

static void cfd_callback(int cfd, uint32_t events, void *user_data)
{
	int n;
	uint8_t buf[BUF_SIZE];

	bzero(buf, BUF_SIZE);
	n = read(cfd, buf, BUF_SIZE);
	if (n < 0) {
		printf("read error: %s\n", strerror(errno));
	} else if (n == 0) { // Disconnect from IP.
			printf("client disconnected\n");
			mainloop_remove_fd(cfd);
	} else 
			write(cfd, buf, n);
}
static void lfd_callback(int lfd, uint32_t events, void *user_data)
{
	int cfd;
	struct sockaddr_in client_addr;
	socklen_t len;
	len = sizeof(client_addr);

	cfd = accept(lfd, (struct sockaddr *)&client_addr, &len);
	if (cfd < 0) {
		printf("accept error: %s\n", strerror(errno));
	}
	printf("Connect from %s\n", inet_ntoa(client_addr.sin_addr));

	mainloop_add_fd(cfd, EPOLLIN, cfd_callback, NULL, NULL);
}


int main()
{
	int ret, lfd, cfd, efd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t len;
	struct epoll_event temp;
	len = sizeof(client_addr);
	int reuse;

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < 0) {
		printf("socket error: %s\n", strerror(errno));
		exit(1);
	}

	reuse = 1;
	ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (ret < 0) {
		printf("setsockopt error: %s\n", strerror(errno));
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		printf("bind error: %s\n", strerror(errno));
		exit(1);
	}

	ret = listen(lfd, 128);
	if (ret < 0) {
		printf("listen error: %s\n", strerror(errno));
		exit(1);
	}
	mainloop_init();
	mainloop_add_fd(lfd, EPOLLIN, lfd_callback, NULL, NULL);
	mainloop_run();
}
