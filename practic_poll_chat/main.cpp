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
	return ioctl(fd, FIONBIO, &flags);
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
	struct pollfd poll_fds[POLL_SIZE];
	poll_fds[0].fd = master_socket;
	poll_fds[0].events = POLLIN;

	while (true) {
		unsigned int idx = 1;
		for (int slave_socket_fd : slave_sockets) {
			poll_fds[idx].fd = slave_socket_fd;
			poll_fds[idx].events = POLLIN;
			idx++;
		}

		unsigned int set_size = 1 + slave_sockets.size();

		const int timeout_wait = -1;
		poll(poll_fds, set_size, timeout_wait);

		for (uint i = 0; i < set_size; ++i) {
			if ((poll_fds[i].revents & POLLIN) != 0) {
				if (i > 0) {
					auto &poll_fd = poll_fds[i].fd;
					static const size_t buf_size = 1024;
					static char buffer[buf_size];
					size_t recv_size =
							recv(poll_fd, buffer, buf_size, MSG_NOSIGNAL);
					if (recv_size == 0 && errno != EAGAIN) {
						shutdown(poll_fd, SHUT_RDWR);
						close(poll_fd);
						slave_sockets.erase(poll_fd);
					} else if (recv_size > 0) {
						send(poll_fd, buffer, recv_size, MSG_NOSIGNAL);
					}
				} else {
					int slave_socket = accept(master_socket, nullptr, nullptr);
					set_nonblock(slave_socket);
					slave_sockets.insert(slave_socket);
				}
			}
		}
	}

	return 0;
}