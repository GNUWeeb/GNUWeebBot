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


#define SQE_MAP_TO_NOP		(0x0000u)
#define SQE_MAP_SHIFT		(0x0001u)
#define SQE_EPOLL_MAP_SIZE	(0xffffu)
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
	uint16_t		*sqe_epoll_map;
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
	void *ret;

	ret = calloc(nmemb, size);
	if (unlikely(ret == NULL)) {
		int err = errno;
		pr_err("calloc(): " PRERF, PREAR(err));
		return NULL;
	}

	return ret;
}


static void reset_chan(struct gwchan *chan, uint16_t chan_idx)
{
	chan->chan_fd    = -1;
	chan->chan_idx   = chan_idx;
	chan->started_at = 0;
	chan->recv_s     = 0;
}


static int init_channels(struct gwbot_state *state)
{
	struct gwchan *channels;
	uint16_t channels_n = state->cfg->sock.channels_n;

	channels = calloc_wrp(channels_n, sizeof(*channels));
	if (unlikely(channels == NULL))
		return -1;

	for (uint16_t i = 0; i < channels_n; i++)
		reset_chan(&channels[i], i);

	state->channels = channels;
	return 0;
}


static int init_sqe_epoll_map(struct gwbot_state *state)
{
	uint16_t *sqe_epoll_map;

	sqe_epoll_map = calloc_wrp(SQE_EPOLL_MAP_SIZE, sizeof(*sqe_epoll_map));
	if (unlikely(sqe_epoll_map == NULL))
		return -1;

	for (uint32_t i = 0; i < SQE_EPOLL_MAP_SIZE; i++) {
		sqe_epoll_map[i] = SQE_MAP_TO_NOP;
	}

	state->sqe_epoll_map = sqe_epoll_map;
	return 0;
}


static int init_state(struct gwbot_state *state)
{
	state->stop_event_loop = false;
	state->intr_sig        = 0;
	state->tcp_fd          = -1;
	state->epoll_fd        = -1;
	state->channels        = NULL;
	state->sqe_epoll_map   = NULL;

	if (unlikely(init_channels(state) < 0))
		return -1;
	if (unlikely(init_sqe_epoll_map(state) < 0))
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


static int run_acceptor(int tcp_fd, struct gwbot_state *state)
{
	int err;
	int chan_fd;
	struct gwchan *chan;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	chan_fd = accept(tcp_fd, (struct sockaddr *)&addr, &addr_len);
	if (unlikely(chan_fd < 0)) {
		err = errno;
		if (err == EAGAIN)
			return 0;
		pr_err("accept(): " PRERF, PREAR(err));
		return -1;
	}

	/*
	 * TODO: Create stack to retrieve channel in O(1).
	 */
	(void)chan;
	(void)state;

	return 0;
}


static int handle_tcp_event(int tcp_fd, struct gwbot_state *state,
			    uint32_t revents)
{
	const uint32_t err_mask = EPOLLERR | EPOLLHUP;

	if (unlikely(revents & err_mask)) {
		pr_err("TCP event error");
		return -1;
	}

	return run_acceptor(tcp_fd, state);
}


static void submit_sqe(uint16_t fdata_len, struct chan_pkt *pkt,
		       struct gwbot_state *state)
{
	/*
	 * TODO: Fix submit_sqe and dispatcher.
	 */
	(void)fdata_len;
	(void)pkt;
	(void)state;
}


static int handle_client_event3(size_t recv_s, struct gwbot_state *state,
				struct gwchan *chan)
{
	uint16_t fdata_len; /* Expected full data length    */
	uint16_t cdata_len; /* Current received data length */
	struct chan_pkt *pkt;


	pkt = &chan->uni_pkt.pkt;
	if (unlikely(recv_s < offsetof(struct chan_pkt, data))) {
		/*
		 * We haven't received the packet length, must
		 * wait a bit longer.
		 *
		 * Bail out!
		 */
		goto out;
	}


	fdata_len = ntohs(pkt->len);
	cdata_len = (uint16_t)recv_s - offsetof(struct chan_pkt, data);


	if (unlikely(fdata_len > sizeof(pkt->data))) {
		/*
		 * The client says that the data length is `fdata_len`.
		 *
		 * However, the server only supports to contain buffer
		 * with sizeof(pkt->data). And at this point, `fdata_len`
		 * is too big to be received, so we drop the connection.
		 *
		 * Close the connection!
		 */
		pr_err("Data length too big: (expected: %u, max_allowed: %zu)",
		       fdata_len, sizeof(pkt->data));
		return -1;
	}


	if (unlikely(cdata_len < fdata_len)) {
		/*
		 * Current received data length is still not enough.
		 * The client says that the data length is `fdata_len`.
		 *
		 * So, wait a bit longer, bail out!
		 */
		goto out;
	}


	if (unlikely(cdata_len > fdata_len)) {
		/*
		 * The client says that the data length is `fdata_len`.
		 * However, we received more than `fdata_len`, this
		 * means it's invalid packet.
		 *
		 * Close the connection!
		 */
		pr_err("Invalid data length: (received: %u; expected: %u)",
		       cdata_len, fdata_len);
		return -1;
	}


	submit_sqe(fdata_len, pkt, state);

	/*
	 * The SQE has been submitted, now we can drop the connection.
	 *
	 * Close the connection!
	 */
	return -1;

out:
	chan->recv_s = recv_s;
	return 0;
}


static int handle_client_event2(int cli_fd, struct gwbot_state *state,
				struct gwchan *chan, uint32_t revents)
{
	int err;
	char *recv_buf;
	size_t recv_s;
	size_t recv_len;
	ssize_t recv_ret;
	const uint32_t err_mask = EPOLLERR | EPOLLHUP;

	if (unlikely(revents & err_mask)) {
		pr_notice("Client has closed its connection");
		goto out_close;
	}

	recv_s   = chan->recv_s;
	recv_buf = &chan->uni_pkt.raw_buf[recv_s];
	recv_len = sizeof(chan->uni_pkt.raw_buf) - recv_s;
	recv_ret = recv(cli_fd, recv_buf, recv_len, 0);

	if (unlikely(recv_ret == 0)) {
		pr_notice("Client has closed its connection");
		goto out_close;
	}

	if (unlikely(recv_ret < 0)) {
		err = errno;
		if (err == EAGAIN)
			return 0;
		pr_err("recv(): " PRERF, PREAR(err));
		goto out_close;
	}

	recv_s += (size_t)recv_ret;
	if (unlikely(handle_client_event3(recv_s, state, chan) < 0))
		goto out_close;

	return 0;
out_close:
	// epoll_delete
	reset_chan(chan, chan->chan_idx);
	close(cli_fd);
	return 0;
}


static int handle_client_event(int cli_fd, struct gwbot_state *state,
			       uint32_t revents)
{
	uint16_t map_to;
	struct gwchan *chan;

	map_to = state->sqe_epoll_map[cli_fd];
	if (unlikely(map_to == SQE_MAP_TO_NOP)) {
		pr_err("Bug: sqe_epoll_map[%d] value is SQE_MAP_TO_NOP",
		       cli_fd);
		return -1;
	}

	chan = &state->channels[map_to];
	return handle_client_event2(cli_fd, state, chan, revents);
}


static int handle_event(struct epoll_event *event, struct gwbot_state *state)
{
	int fd;
	uint32_t revents;

	fd      = event->data.fd;
	revents = event->events;
	if (unlikely(fd == state->tcp_fd))
		return handle_tcp_event(fd, state, revents);

	return handle_client_event(fd, state, revents);
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
	free(state->sqe_epoll_map);
	free(state->channels);
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
