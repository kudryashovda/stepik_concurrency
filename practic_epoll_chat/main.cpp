#include <iostream>
#include <algorithm>
#include <set>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_EVENTS 32

int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		flags = 0;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIONBIO, &flags);
#endif
}

int main()
{
	int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(12345);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(master_socket, reinterpret_cast<struct sockaddr *>(&sockaddr),
			sizeof(sockaddr));

	set_nonblock(master_socket);

	listen(master_socket, SOMAXCONN);

	int epoll = epoll_create1(0);
	struct epoll_event event_master;
	event_master.data.fd = master_socket;
	event_master.events = EPOLLIN;
	epoll_ctl(epoll, EPOLL_CTL_ADD, master_socket, &event_master);


	while (true) {
		struct epoll_event events[MAX_EVENTS];
		int N = epoll_wait(epoll, events, MAX_EVENTS, -1);
		for (uint i = 0; i < N; i++) {
			if (events[i].data.fd == master_socket) {
				int slave_socket = accept(master_socket, nullptr, nullptr);
				set_nonblock(slave_socket);
				struct epoll_event event;
				event.data.fd = slave_socket;
				event.events = EPOLLIN;
				epoll_ctl(epoll, EPOLL_CTL_ADD, slave_socket, &event);

			} else {
				static char buffer[1024];
				int recv_result =
						recv(events[i].data.fd, buffer, 1024, MSG_NOSIGNAL);
				if (recv_result == 0 && errno != EAGAIN) {
					shutdown(events[i].data.fd, SHUT_RDWR);
					close(events[i].data.fd);
				} else if (recv_result > 0) {
					send(events[i].data.fd, buffer, recv_result, MSG_NOSIGNAL);
				}
			}
		}
	}

	return 0;
}