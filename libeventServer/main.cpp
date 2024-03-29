#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#define UNIX_SOCK_PATH "/tmp/echo.sock"

static void echo_read_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *input = bufferevent_get_input(bev);
	struct evbuffer *output = bufferevent_get_output(bev);

	size_t length = evbuffer_get_length(input);
	void *data;
	data = malloc(length);
	evbuffer_copyout(input, (char *)data, length);
	printf("data: %s\n", (char *)data);

	evbuffer_add_buffer(output, input);
	free(data);
}

static void echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	if (events & BEV_EVENT_ERROR) {
		perror("Error");
		bufferevent_free(bev);
	}
	if (events & BEV_EVENT_EOF) {
		bufferevent_free(bev);
	}
}

static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd,
		struct sockaddr *address, int socklen, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev =
			bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Error = %d = \"%s\"\n", err,
			evutil_socket_error_to_string(err));
	event_base_loopexit(base, NULL);
}

int main()
{
	struct event_base *base = event_base_new();

	struct sockaddr_un sun;
	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, UNIX_SOCK_PATH);

	struct evconnlistener *listener = evconnlistener_new_bind(base,
			accept_conn_cb, NULL, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
			(struct sockaddr *)&sun, sizeof(sun));
	evconnlistener_set_error_cb(listener, accept_error_cb);

	event_base_dispatch(base);

	return 0;
}