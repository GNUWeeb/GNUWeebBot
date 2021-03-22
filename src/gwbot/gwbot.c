// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/gwbot.c
 *
 *  Core GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <time.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <gwbot/common.h>
#include <gwbot/gwchan.h>
#include <gwbot/lib/tg_api/send_message.h>


#define EPOLL_INPUT_EVT		(EPOLLIN | EPOLLPRI)


struct gwbot_state {
	bool			stop_event_loop;
	struct_pad(0, 3);

	/* Interrupt signal */
	int			intr_sig;

	int			tcp_fd;
	int			epoll_fd;
	struct gwbot_cfg	*cfg;
	struct gwchan		*channels;
};


static struct gwbot_state *g_state;


static void handle_interrupt(int sig)
{
	struct gwbot_state *state = g_state;
	state->intr_sig = sig;
	state->stop_event_loop = true;
	putchar('\n');
}


static int validate_cfg(struct gwbot_cfg *cfg)
{
	struct gwbot_sock_cfg *sock = &cfg->sock;
	struct gwbot_cred_cfg *cred = &cfg->cred;

	if (unlikely(sock->bind_addr == NULL || *sock->bind_addr == '\0')) {
		pr_err("sock->bind_addr cannot be empty!");
		return -EINVAL;
	}

	if (unlikely(sock->bind_port == 0)) {
		pr_err("sock->bind_port cannot be empty");
		return -EINVAL;
	}

	if (unlikely(cred->token == NULL || *cred->token == '\0')) {
		pr_err("cred->token cannot be empty");
		return -EINVAL;
	}

	if (unlikely(sock->channels_n == 0)) {
		pr_err("sock->channels_n cannot be zero");
		return -EINVAL;
	}

	return 0;
}


static void *calloc_wrp(size_t nmemb, size_t size)
{
	void *ret = calloc(nmemb, size);
	if (unlikely(ret == NULL)) {
		int err = errno;
		pr_err("calloc(): " PRERF, PREAR(err));
		return NULL;
	}

	return ret;
}


static int init_channels(struct gwbot_state *state)
{
	struct gwchan *channels;

	channels = calloc_wrp(state->cfg->sock.channels_n, sizeof(*channels));
	if (unlikely(channels == NULL))
		return -1;

	state->channels = channels;
	return 0;
}


static int init_state(struct gwbot_state *state)
{
	state->stop_event_loop = false;
	state->intr_sig        = 0;
	state->tcp_fd          = -1;
	state->epoll_fd        = -1;

	if (unlikely(init_channels(state) < 0))
		return -1;

	return 0;
}


static int socket_setup(int tcp_fd, struct gwbot_state *state)
{
	int y;
	int err;
	int retval;
	const char *lv, *on; /* level and optname */
	socklen_t len = sizeof(y);
	struct gwbot_cfg *cfg = state->cfg;
	const void *py = (const void *)&y;

	y = 1;
	retval = setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, py, len);
	if (unlikely(retval < 0)) {
		lv = "SOL_SOCKET";
		on = "SO_REUSEADDR";
		goto out_err;
	}

	y = 1;
	retval = setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, py, len);
	if (unlikely(retval < 0)) {
		lv = "SOL_SOCKET";
		on = "SO_REUSEADDR";
		goto out_err;
	}

	y = 1;
	retval = setsockopt(tcp_fd, IPPROTO_TCP, TCP_NODELAY, py, len);
	if (unlikely(retval < 0)) {
		lv = "IPPROTO_TCP";
		on = "TCP_NODELAY";
		goto out_err;
	}

	/*
	 * Use cfg to set some socket options.
	 */
	(void)cfg;
	return retval;

out_err:
	err = errno;
	pr_err("setsockopt(tcp_fd, %s, %s): " PRERF, lv, on, PREAR(err));
	return retval;
}


static int epoll_add(int epl_fd, int fd, uint32_t events)
{
	int err;
	struct epoll_event event;

	/* Shut the valgrind up! */
	memset(&event, 0, sizeof(struct epoll_event));

	event.events  = events;
	event.data.fd = fd;
	if (unlikely(epoll_ctl(epl_fd, EPOLL_CTL_ADD, fd, &event) < 0)) {
		err = errno;
		pr_err("epoll_ctl(EPOLL_CTL_ADD): " PRERF, PREAR(err));
		return -1;
	}
	return 0;
}


static int init_socket(struct gwbot_state *state)
{
	int err;
	int tcp_fd;
	int ret = 0;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	prl_notice(0, "Creating TCP socket...");
	tcp_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (unlikely(tcp_fd < 0)) {
		err = errno;
		pr_err("socket(): " PRERF, PREAR(err));
		return -1;
	}

	prl_notice(0, "Setting socket file descriptor up...");
	ret = socket_setup(tcp_fd, state);
	if (unlikely(ret < 0)) {
		ret = -1;
		goto out;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(state->cfg->sock.bind_port);
	addr.sin_addr.s_addr = inet_addr(state->cfg->sock.bind_addr);

	ret = bind(tcp_fd, (struct sockaddr *)&addr, addr_len);
	if (unlikely(ret < 0)) {
		ret = -1;
		err = errno;
		pr_err("bind(): " PRERF, PREAR(err));
		goto out;
	}

	ret = listen(tcp_fd, state->cfg->sock.backlog);
	if (unlikely(ret < 0)) {
		ret = -1;
		err = errno;
		pr_err("listen(): " PRERF, PREAR(err));
		goto out;
	}

	ret = epoll_add(state->epoll_fd, tcp_fd, EPOLL_INPUT_EVT);
	if (unlikely(ret < 0)) {
		ret = -1;
		goto out;
	}

	pr_notice("Listening on %s:%d...", state->cfg->sock.bind_addr,
		  state->cfg->sock.bind_port);

	state->tcp_fd = tcp_fd;
out:
	if (unlikely(ret != 0))
		close(tcp_fd);
	return ret;
}


static int init_epoll(struct gwbot_state *state)
{
	int err;
	int epoll_fd;

	prl_notice(0, "Initializing epoll_fd...");
	epoll_fd = epoll_create(255);
	if (unlikely(epoll_fd < 0)) {
		err = errno;
		pr_err("epoll_create(): " PRERF, PREAR(err));
		return -1;
	}

	state->epoll_fd = epoll_fd;
	return 0;
}


static int handle_event(struct epoll_event *event, struct gwbot_state *state)
{
	int ret = 0;

	(void)event;
	(void)state;
	return ret;
} 


static int handle_events(int num_of_events, struct epoll_event *events,
			 struct gwbot_state *state)
{
	for (int i = 0; i < num_of_events; i++) {
		if (unlikely(handle_event(&events[i], state) < 0))
			return -1;
	}
	return 0;
}


static int run_event_loop(struct gwbot_state *state)
{
	int err;
	int ret = 0;
	int epoll_ret;
	int timeout = 500; /* in milliseconds */
	int maxevents = 30;
	int epoll_fd = state->epoll_fd;
	struct epoll_event events[30];

	while (likely(!state->stop_event_loop)) {
		epoll_ret = epoll_wait(epoll_fd, events, maxevents, timeout);
		if (unlikely(epoll_ret == 0)) {
			continue;
		}

		if (unlikely(epoll_ret < 0)) {
			err = errno;
			if (err == EINTR) {
				pr_notice("Interrupted!");
				continue;
			}

			pr_err("epoll_wait(): " PRERF, PREAR(err));
			break;
		}

		if (unlikely(handle_events(epoll_ret, events, state) < 0))
			break;
	}


	return ret;
}


static void close_file_descriptors(struct gwbot_state *state)
{
	int tcp_fd = state->tcp_fd;

	if (likely(tcp_fd != -1)) {
		pr_notice("Closing state->tcp_fd (%d)", tcp_fd);
		close(tcp_fd);
	}
}


static void destroy_state(struct gwbot_state *state)
{
	close_file_descriptors(state);
}


int gwbot_run(struct gwbot_cfg *cfg)
{
	int ret;
	struct gwbot_state state;

	/* Shut the valgrind up! */
	memset(&state, 0, sizeof(state));

	state.cfg = cfg;
	g_state = &state;
	signal(SIGHUP, handle_interrupt);
	signal(SIGINT, handle_interrupt);
	signal(SIGTERM, handle_interrupt);
	signal(SIGQUIT, handle_interrupt);
	signal(SIGPIPE, SIG_IGN);

	tg_api_global_init();

	ret = validate_cfg(cfg);
	if (unlikely(ret < 0))
		goto out;
	ret = init_state(&state);
	if (unlikely(ret < 0))
		goto out;
	ret = init_epoll(&state);
	if (unlikely(ret < 0))
		goto out;
	ret = init_socket(&state);
	if (unlikely(ret < 0))
		goto out;
	ret = run_event_loop(&state);
out:
	tg_api_global_destroy();
	destroy_state(&state);
	return ret;
}
