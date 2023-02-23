#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstring>

int main()
{
	const int domain = AF_INET; // IPv4
	const int type = SOCK_STREAM; // TCP
	const int protocol = 0; // let socket choose it base on type
	const int sockfd = socket(domain, type, protocol);
	if (sockfd == -1) {
		printf("Can't create socket\n");
		return 1;
	}

	sockaddr_in sa;
	sa.sin_family = domain;
	sa.sin_port = htons(12345);
	sa.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0

	const int bind_status =
			bind(sockfd, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
	if (bind_status == -1) {
		printf("Can't bind addr to socket\n");
		return 1;
	}

	const int listen_status = listen(sockfd, SOMAXCONN);
	if (listen_status == -1) {
		printf("Can't start listen to socket\n");
		return 1;
	}

	const int derived_sockfd = accept(sockfd, nullptr, nullptr);
	if (derived_sockfd == -1) {
		printf("Can't accept incoming connection\n");
		return 1;
	}

	const int in_buf_bytes = 256;
	unsigned char buffer[in_buf_bytes];

	while (true) {
		memset(buffer, 0, in_buf_bytes);
		const unsigned read_status =
				recv(derived_sockfd, buffer, in_buf_bytes, 0);
		if (read_status < 1) {
			printf("Error while reading from socket\n");
			return 1;
		}

		if (buffer[0] == 'q') {
			shutdown(derived_sockfd, SHUT_RDWR);
			close(derived_sockfd);
			break;
		}
		send(derived_sockfd, buffer, read_status, 0);

		printf("in buffer: %s\n", buffer);
		printf("bytes received: %u\n", read_status);
	}
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);

	return 0;
}