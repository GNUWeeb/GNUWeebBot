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
#include <sys/time.h>
#include <inttypes.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <gwbot/sqe.h>
#include <gwbot/common.h>
#include <gwbot/gwchan.h>
#include <gwbot/tstack.h>
#include <gwbot/lib/string.h>
#include <gwbot/event_handler.h>


#define EPL_MAP_TO_NOP		(0x0000u)
#define EPL_MAP_SHIFT		(0x0001u)
#define EPL_MAP_SIZE		(0xffffu)
#define EPL_INPUT_EVT		(EPOLLIN | EPOLLPRI)

/* Macros for printing  */
#define W_IP(CHAN) ((CHAN)->src_ip), ((CHAN)->src_port)
#define W_IU(CHAN) W_IP(CHAN)
#define PRWIU "%s:%d"


static struct gwbot_state *g_state;


static int mutex_lock(struct gwlock *lock)
{
	return pthread_mutex_lock(&lock->mutex);
}


static int mutex_unlock(struct gwlock *lock)
{
	return pthread_mutex_unlock(&lock->mutex);
}


static void handle_interrupt(int sig)
{
	struct gwbot_state *state = g_state;
	state->intr_sig = sig;
	state->stop_el = true;
	putchar('\n');
}


static int validate_cfg(struct gwbot_cfg *cfg)
{
	struct gwbot_sock_cfg *sock   = &cfg->sock;
	struct gwbot_cred_cfg *cred   = &cfg->cred;
	struct gwbot_wrk_cfg  *worker = &cfg->worker;

	if (unlikely(sock->bind_addr == NULL || *sock->bind_addr == '\0')) {
		pr_err("sock->bind_addr cannot be empty");
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

	if (unlikely(worker->thread_c == 0)) {
		pr_err("worker->thread_c cannot be zero");
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
		pr_err("calloc_wrp(): " PRERF, PREAR(err));
		return NULL;
	}

	return ret;
}


static void reset_chan(struct gwchan *chan, uint32_t chan_idx)
{
	chan->chan_fd    = -1;
	chan->chan_idx   = chan_idx;
	chan->started_at = 0;
	chan->recv_s     = 0;
}


static void reset_thread(struct gwbot_thread *thread, uint32_t thread_idx,
			 struct gwbot_state *state)
{
	thread->started_at = 0u;
	thread->thread_idx = thread_idx;
	thread->state      = state;
}


static int init_state_chans(struct gwbot_state *state)
{
	struct gwchan *chans;
	uint16_t thread_c = state->cfg->worker.thread_c;

	chans = calloc_wrp(thread_c, sizeof(*chans));
	if (unlikely(chans == NULL))
		return -1;

	for (uint32_t i = 0; i < thread_c; i++)
		reset_chan(&chans[i], i);

	state->chans = chans;
	return 0;
}


static int init_state_threads(struct gwbot_state *state)
{
	struct gwbot_thread *threads;
	uint16_t thread_c = state->cfg->worker.thread_c;

	threads = calloc_wrp(thread_c, sizeof(*threads));
	if (unlikely(threads == NULL))
		return -1;

	for (uint32_t i = 0; i < thread_c; i++)
		reset_thread(&threads[i], i, state);

	state->threads = threads;
	return 0;
}


static int init_epl_map(struct gwbot_state *state)
{
	uint16_t *epl_map_chan;

	epl_map_chan = calloc_wrp(EPL_MAP_SIZE, sizeof(*epl_map_chan));
	if (unlikely(epl_map_chan == NULL))
		return -1;

	for (uint32_t i = 0; i < EPL_MAP_SIZE; i++)
		epl_map_chan[i] = EPL_MAP_TO_NOP;

	state->epl_map_chan = epl_map_chan;
	return 0;
}


static int init_state(struct gwbot_state *state)
{
	int err;
	uint16_t thread_c = state->cfg->worker.thread_c;

	state->stop_el           = false;
	state->intr_sig          = 0;
	state->tcp_fd            = -1;
	state->epl_fd            = -1;
	state->epl_map_chan      = NULL;
	state->chans             = NULL;
	state->threads           = NULL;
	state->chan_stack.arr    = NULL;
	state->thread_stack.arr  = NULL;

	if (unlikely(init_state_chans(state) < 0))
		return -1;
	if (unlikely(init_state_threads(state) < 0))
		return -1;
	if (unlikely(init_epl_map(state) < 0))
		return -1;
	if (unlikely(tss_init(&state->chan_stack, thread_c) == NULL)) 
		return -1;
	if (unlikely(tss_init(&state->thread_stack, thread_c) == NULL))
		return -1;
	if (unlikely(sqe_init(&state->sqes, 1024) == NULL))
		return -1;


	err = pthread_mutex_init(&state->thread_stk_lock.mutex, NULL);
	if (unlikely(err != 0)) {
		err = (err > 0) ? err : -err;
		pr_err("pthread_mutex_init(): " PRERF, PREAR(err));
		return -1;
	}
	state->thread_stk_lock.need_destroy = true;


	for (uint32_t i = thread_c; i--;) {
		int32_t ret;
		uint16_t i16 = (uint16_t)i;

		ret = tss_push(&state->chan_stack, i16);
		TASSERT(ret == (int32_t)i);

		mutex_lock(&state->thread_stk_lock);
		ret = tss_push(&state->thread_stack, i16);
		TASSERT(ret == (int32_t)i);
		mutex_unlock(&state->thread_stk_lock);
	}

	return 0;
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


static int epoll_delete(int epl_fd, int fd)
{
	int err;

	if (unlikely(epoll_ctl(epl_fd, EPOLL_CTL_DEL, fd, NULL) < 0)) {
		err = errno;
		pr_error("epoll_ctl(EPOLL_CTL_DEL): " PRERF, PREAR(err));
		return -1;
	}
	return 0;
}


static int init_epoll(struct gwbot_state *state)
{
	int err;
	int epl_fd;

	prl_notice(0, "Initializing epoll_fd...");
	epl_fd = epoll_create(255);
	if (unlikely(epl_fd < 0)) {
		err = errno;
		pr_err("epoll_create(): " PRERF, PREAR(err));
		return -1;
	}

	state->epl_fd = epl_fd;
	return 0;
}


static int setup_socket(int tcp_fd, struct gwbot_state *state)
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
	ret = setup_socket(tcp_fd, state);
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

	ret = epoll_add(state->epl_fd, tcp_fd, EPL_INPUT_EVT);
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


static const char *convert_addr_ntop(struct sockaddr_in *addr, char *src_ip_buf)
{
	int err;
	const char *ret;
	in_addr_t saddr = addr->sin_addr.s_addr;

	ret = inet_ntop(AF_INET, &saddr, src_ip_buf, IPV4_L);
	if (unlikely(ret == NULL)) {
		err = errno;
		err = err ? err : EINVAL;
		pr_err("inet_ntop(): " PRERF, PREAR(err));
		return NULL;
	}

	return ret;
}


static int run_acceptor(int tcp_fd, struct gwbot_state *state)
{
	int err;
	int chan_fd;
	int32_t pop_ret;
	uint16_t src_port;
	const char *src_ip;
	struct gwchan *chan;
	struct timeval tmvl;
	struct sockaddr_in addr;
	char src_ip_buf[IPV4_L + 1];
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


	src_port = ntohs(addr.sin_port);
	src_ip   = convert_addr_ntop(&addr, src_ip_buf);
	if (unlikely(!src_ip)) {
		pr_err("Cannot parse source address");
		goto out_close;
	}

	pop_ret = tss_pop(&state->chan_stack);
	if (unlikely(pop_ret == -1)) {
		pr_err("Channel slot is full. Cannot accept connection from "
		       "%s:%u", src_ip, src_port);
		goto out_close;
	}


	if (unlikely(gettimeofday(&tmvl, NULL) < 0)) {
		err = errno;
		pr_err("gettimeofday() when accepting %s:%u: " PRERF,
		       src_ip, src_port, PREAR(err));
		goto out_close;
	}


	if (unlikely(epoll_add(state->epl_fd, chan_fd, EPL_INPUT_EVT))) {
		pr_err("Cannot add connection from %s:%u to epoll entries",
		       src_ip, src_port);
		goto out_close;
	}


	chan = &state->chans[pop_ret];
	chan->chan_fd    = chan_fd;
	chan->chan_idx   = (uint32_t)pop_ret;
	chan->started_at = tmvl.tv_sec;
	chan->recv_s     = 0;
	chan->src_port   = src_port;
	sane_strncpy(chan->src_ip, src_ip, sizeof(chan->src_ip));

	state->epl_map_chan[chan_fd] = (uint16_t)pop_ret + EPL_MAP_SHIFT;

	pr_notice("New connection from %s:%u", src_ip, src_port);
	return 0;

out_close:
	close(chan_fd);
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


static void *handle_thread(void *thread_void_p)
{
	struct gwbot_thread *thread = thread_void_p;
	struct gwbot_state  *state  = thread->state;

	gwbot_event_handler(thread);
	reset_thread(thread, thread->thread_idx, state);

	mutex_lock(&state->thread_stk_lock);
	tss_push(&state->thread_stack, (uint16_t)thread->thread_idx);
	mutex_unlock(&state->thread_stk_lock);
	return NULL;
}


static int enqueue_to_sqe(uint16_t fdata_len, struct gwbot_state *state,
			  struct gwchan *chan)
{
	int err;
	struct sqe_node *node;
	size_t cpy_len = sizeof(fdata_len) + fdata_len;
	void  *cpy_src = chan->uni_pkt.raw_buf;

	node = sqe_enqueue(&state->sqes, cpy_src, cpy_len);
	if (unlikely(node == NULL)) {
		err = errno;
		if (err == EAGAIN)
			pr_err("SQE buffer is full");
		return -1;
	}

	return 0;
}


static void submit_sqe(uint16_t fdata_len, struct gwbot_state *state,
		       struct gwchan *chan)
{
	int ret;
	int32_t pop_ret;
	struct gwbot_thread *thread;
	size_t cpy_len = sizeof(fdata_len) + fdata_len;

	mutex_lock(&state->thread_stk_lock);
	pop_ret = tss_pop(&state->thread_stack);
	mutex_unlock(&state->thread_stk_lock);
	if (unlikely(pop_ret == -1)) {

		prl_notice(0, "Thread(s) are all busy, saving SQE...");
		if (unlikely(enqueue_to_sqe(fdata_len, state, chan) == 0))
			return; /* OK */

		goto out_err;
	}

	thread = &state->threads[pop_ret];
	memcpy(&thread->uni_pkt.pkt, &chan->uni_pkt.pkt, cpy_len);

	ret = pthread_create(&thread->thread, NULL, handle_thread, thread);
	if (unlikely(ret != 0)) {
		ret = ret > 0 ? ret : -ret;
		pr_err("pthread_create() when serving " PRWIU ": " PRERF,
		       W_IU(chan), PREAR(ret));
		state->stop_el = true;
		return;
	}
	pthread_detach(thread->thread);

	return;

out_err:
	/* TODO: Send error response to client */
	return;	
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
		 * So, wait a bit longer.
		 *
		 * Bail out!
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

	pkt->len = fdata_len;
	pkt->data[fdata_len] = '\0';
	submit_sqe(fdata_len, state, chan);

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


static int handle_client_event2(int chan_fd, struct gwbot_state *state,
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
	recv_ret = recv(chan_fd, recv_buf, recv_len, 0);

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
	prl_notice(0, "Closing connection from " PRWIU "...", W_IU(chan));
	tss_push(&state->chan_stack, (uint16_t)chan->chan_idx);
	epoll_delete(state->epl_fd, chan_fd);
	reset_chan(chan, chan->chan_idx);
	close(chan_fd);

	state->epl_map_chan[chan_fd] = EPL_MAP_TO_NOP;
	return 0;
}


static int handle_client_event(int chan_fd, struct gwbot_state *state,
			       uint32_t revents)
{
	uint16_t map_to;
	struct gwchan *chan;

	map_to = state->epl_map_chan[chan_fd];
	if (unlikely(map_to == EPL_MAP_TO_NOP)) {
		pr_err("Bug: sqe_epoll_map[%d] value is EPL_MAP_TO_NOP",
		       chan_fd);
		return -1;
	}

	map_to -= EPL_MAP_SHIFT;
	chan    = &state->chans[map_to];
	return handle_client_event2(chan_fd, state, chan, revents);
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
	int maxevents = 32;
	int epl_fd = state->epl_fd;
	struct epoll_event events[32];

	while (likely(!state->stop_el)) {
		epoll_ret = epoll_wait(epl_fd, events, maxevents, timeout);
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

		/* TODO: Timeout monitor */
	}


	return ret;
}


static void close_file_descriptors(struct gwbot_state *state)
{
	int tcp_fd = state->tcp_fd;
	int epl_fd = state->epl_fd;

	if (likely(tcp_fd != -1)) {
		pr_notice("Closing state->tcp_fd (%d)", tcp_fd);
		close(tcp_fd);
		state->tcp_fd = -1;
	}

	if (likely(epl_fd != -1)) {
		pr_notice("Closing state->epl_fd (%d)", epl_fd);
		close(epl_fd);
		state->epl_fd = -1;
	}
}


static void destroy_state(struct gwbot_state *state)
{
	/* TODO: Wait for threads */

	close_file_descriptors(state);
	tss_destroy(&state->chan_stack);
	tss_destroy(&state->thread_stack);
	sqe_destroy(&state->sqes);

	if (state->thread_stk_lock.need_destroy)
		pthread_mutex_destroy(&state->thread_stk_lock.mutex);

	free(state->epl_map_chan);
	free(state->threads);
	free(state->chans);
}


int gwbot_run(struct gwbot_cfg *cfg)
{
	int ret;
	struct gwbot_state state;

	memset(&state, 0, sizeof(state));

	state.cfg = cfg;
	g_state = &state;
	signal(SIGHUP, handle_interrupt);
	signal(SIGINT, handle_interrupt);
	signal(SIGTERM, handle_interrupt);
	signal(SIGQUIT, handle_interrupt);
	signal(SIGPIPE, SIG_IGN);

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
	destroy_state(&state);
	return ret;
}
