// for a complete and correct variant see https://www.ibm.com/docs/en/i/7.1?topic=designs-example-nonblocking-io-select

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <set>
#include <poll.h>

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
	return ioctl(fd, FIOBIO, &flags); // check if FIOBIO is correct
#endif
}

int main()
{
	int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::set<int> slave_sockets;

	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(12345);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(master_socket, reinterpret_cast<struct sockaddr *>(&sockaddr),
			sizeof(sockaddr));

	set_nonblock(master_socket);

	listen(master_socket, SOMAXCONN);

	const int POLL_SIZE = 2048;
	struct pollfd set[POLL_SIZE];
	set[0].fd = master_socket;
	set[0].events = POLLIN;

	while (true) {
		unsigned int idx = 1;
		for (auto iter = slave_sockets.begin(); iter != slave_sockets.end();
				++iter) {
			set[idx].fd = *iter;
			set[idx].events = POLLIN;
			idx++;
		}

		unsigned int set_size = 1 + slave_sockets.size();

		const int timeout_wait = -1;
		poll(set, set_size, timeout_wait);

		for (uint i = 0; i < set_size; ++i) {
			if (set[i].revents & POLLIN) {
				if (i > 0) {
					static char buffer[1024];
					int recv_size = recv(set[i].fd, buffer, 1024, MSG_NOSIGNAL);
					if (recv_size == 0 && errno != EAGAIN) {
						shutdown(set[i].fd, SHUT_RDWR);
						close(set[i].fd);
						slave_sockets.erase(set[i].fd);
					} else if (recv_size > 0) {
						send(set[i].fd, buffer, recv_size, MSG_NOSIGNAL);
					}
				} else {
					int slave_socket = accept(master_socket, 0, 0);
					set_nonblock(slave_socket);
					slave_sockets.insert(slave_socket);
				}
			}
		}
	}

	return 0;
}