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
#include <stdatomic.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <gwbot/gwbot.h>
#include <gwbot/mysql.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/event_logger.h>
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


static void handle_interrupt(int sig)
{
	struct gwbot_state *state = g_state;

	state->intr_sig = sig;
	state->stop_el  = true;
	putchar('\n');
}


static int validate_mysql_cfg(struct gwbot_mysql_cfg *mysql)
{
	if (unlikely(mysql->host == NULL)) {
		pr_err("mysql->host cannot be empty");
		return -EINVAL;
	}

	if (unlikely(mysql->user == NULL)) {
		pr_err("mysql->user cannot be empty");
		return -EINVAL;
	}

	if (unlikely(mysql->db == NULL)) {
		pr_err("mysql->db cannot be empty");
		return -EINVAL;
	}

	return 0;
}


static int validate_cfg(struct gwbot_cfg *cfg)
{
	struct gwbot_sock_cfg	*sock	= &cfg->sock;
	struct gwbot_cred_cfg	*cred	= &cfg->cred;
	struct gwbot_wrk_cfg	*worker	= &cfg->worker;
	struct gwbot_mysql_cfg	*mysql	= &cfg->mysql;

	if (unlikely(sock->bind_addr == NULL || *sock->bind_addr == '\0')) {
		pr_err("sock->bind_addr cannot be empty");
		return -EINVAL;
	}

	if (unlikely(sock->bind_port == 0)) {
		pr_err("sock->bind_port cannot be zero");
		return -EINVAL;
	}

	if (unlikely(sock->backlog == 0)) {
		pr_err("sock->backlog cannot be zero");
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

	if (likely(cfg->use_db)) {
		int ret = validate_mysql_cfg(mysql);
		if (unlikely(ret))
			return ret;
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


static void reset_chan(struct gwbot_chan *chan, uint32_t chan_idx)
{
	chan->chan_fd     = -1;
	chan->chan_idx    = (uint16_t)chan_idx;
	chan->accepted_at = 0u;
	chan->recv_s      = 0u;
	chan->is_online   = false;
}


static int init_state_chans(struct gwbot_state *state)
{
	struct gwbot_chan *chans;
	uint16_t thread_c = state->cfg->worker.thread_c;

	chans = calloc_wrp(thread_c, sizeof(*chans));
	if (unlikely(chans == NULL))
		return -1;

	for (uint32_t i = 0; i < thread_c; i++)
		reset_chan(&chans[i], i);

	state->chans = chans;
	return 0;
}


static int reset_thread(struct gwbot_thread *thread, uint32_t thread_idx,
			struct gwbot_state *state)
{
	int err;
	thread->state       = state;
	thread->last_work   = 0u;
	thread->thread_idx  = (uint16_t)thread_idx;
	thread->is_online   = false;
	thread->has_event   = false;
	thread->ev_from_sqe = false;
	thread->pkt_len     = 0u;
	thread->sqe_node    = NULL;


	err = gw_mutex_init(&thread->ev_lock, NULL);
	if (unlikely(err != 0)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_mutex_init(): " PRERF, PREAR(err));
		return -err;
	}


	err = gw_cond_init(&thread->ev_cond, NULL);
	if (unlikely(err != 0)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_cond_init(): " PRERF, PREAR(err));
		return -err;
	}

	return 0;
}


static int init_state_threads(struct gwbot_state *state)
{
	struct gwbot_thread *threads;
	uint16_t thread_c = state->cfg->worker.thread_c;

	threads = calloc_wrp(thread_c, sizeof(*threads));
	if (unlikely(threads == NULL))
		return -ENOMEM;

	for (uint32_t i = 0; i < thread_c; i++) {
		int ret = reset_thread(&threads[i], i, state);
		if (unlikely(ret))
			return ret;
	}

	state->threads = threads;
	return 0;
}


static int init_epl_map(struct gwbot_state *state)
{
	uint16_t *epl_map;

	epl_map = calloc_wrp(EPL_MAP_SIZE, sizeof(*epl_map));
	if (unlikely(epl_map == NULL))
		return -ENOMEM;

	for (uint32_t i = 0; i < EPL_MAP_SIZE; i++)
		epl_map[i] = EPL_MAP_TO_NOP;

	state->epl_map = epl_map;
	return 0;
}


static int init_state(struct gwbot_state *state)
{
	int err;
	uint16_t thread_c = state->cfg->worker.thread_c;

	state->stop_el		= false;
	state->intr_sig		= -1;
	state->tcp_fd		= -1;
	state->epl_fd		= -1;
	state->epl_map		= NULL;
	state->chans		= NULL;
	state->threads		= NULL;
	state->active_chan      = 0;
	atomic_store(&state->online_thread, 0);

	err = init_state_chans(state);
	if (unlikely(err))
		return err;

	err = init_state_threads(state);
	if (unlikely(err))
		return err;

	err = init_epl_map(state);
	if (unlikely(err))
		return err;

	if (unlikely(tss_init(&state->chan_stk, thread_c) == NULL)) 
		return -ENOMEM;

	if (unlikely(tss_init(&state->thread_stk, thread_c) == NULL))
		return -ENOMEM;

	if (unlikely(que_init(&state->sqes, 100000) == NULL))
		return -ENOMEM;

	if (unlikely(que_init(&state->cqes, 100000) == NULL))
		return -ENOMEM;

	err = gw_mutex_init(&state->sqe_lock, NULL);
	if (unlikely(err)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_mutex_init(): " PRERF, PREAR(err));
		return -err;
	}

	err = gw_mutex_init(&state->cqe_lock, NULL);
	if (unlikely(err)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_mutex_init(): " PRERF, PREAR(err));
		return -err;
	}

	err = gw_mutex_init(&state->log_lock, NULL);
	if (unlikely(err)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_mutex_init(): " PRERF, PREAR(err));
		return -err;
	}

	err = gw_cond_init(&state->log_cond, NULL);
	if (unlikely(err)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_mutex_init(): " PRERF, PREAR(err));
		return -err;
	}

	err = gw_mutex_init(&state->thread_stk_lock, NULL);
	if (unlikely(err)) {
		err = (err > 0) ? err : -err;
		pr_err("gw_mutex_init(): " PRERF, PREAR(err));
		return -err;
	}

	for (uint32_t i = thread_c; i--;) {
		int64_t __maybe_unused ret;
		uint16_t i16 = (uint16_t)i;

		ret = tss_push(&state->chan_stk, i16);
		TASSERT(ret == (int32_t)i);

		gw_mutex_lock(&state->thread_stk_lock);
		ret = tss_push(&state->thread_stk, i16);
		TASSERT(ret == (int32_t)i);
		gw_mutex_unlock(&state->thread_stk_lock);
	}


	if (likely(state->cfg->use_db)) {
		/*
		 *
		 * `mysql_thread_init()` should be called before
		 * setting the interrupt handler.
		 *
		 */
		mysql_thread_init();
	}

	signal(SIGHUP, handle_interrupt);
	signal(SIGINT, handle_interrupt);
	signal(SIGTERM, handle_interrupt);
	signal(SIGQUIT, handle_interrupt);
	signal(SIGPIPE, SIG_IGN);

	tg_api_global_init();
	return 0;
}


static int init_logger_thread(struct gwbot_state *state)
{
	int err;
	uint32_t otv_old;
	_Atomic(uint32_t) *ot;

	ot = &state->online_thread;
	otv_old = atomic_load_explicit(ot, memory_order_acquire);

	pr_notice("Initializing logger thread...");
	err = pthread_create(&state->log_thread, NULL, gw_event_logger, state);
	if (unlikely(err != 0)) {
		err = (err > 0) ? err : -err;
		pr_err("pthread_create(): " PRERF, PREAR(err));
		return -err;
	}
	pthread_detach(state->log_thread);

	/*
	 * Wait for the logger thread to signal us.
	 */
	gw_mutex_lock(&state->log_lock);
	while (atomic_load_explicit(ot, memory_order_acquire) == otv_old)
		gw_cond_wait(&state->log_cond, &state->log_lock);
	gw_mutex_unlock(&state->log_lock);
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
		return -err;
	}

	state->epl_fd = epl_fd;
	return 0;
}


static int epoll_delete(int epl_fd, int fd)
{
	int err;

	if (unlikely(epoll_ctl(epl_fd, EPOLL_CTL_DEL, fd, NULL) < 0)) {
		err = errno;
		pr_error("epoll_ctl(EPOLL_CTL_DEL): " PRERF, PREAR(err));
		return -err;
	}
	return 0;
}


static int epoll_add(int epl_fd, int fd, uint32_t events)
{
	int err;
	struct epoll_event event;

	/* Shut the valgrind up! */
	memset(&event, 0, sizeof(event));

	event.events  = events;
	event.data.fd = fd;
	if (unlikely(epoll_ctl(epl_fd, EPOLL_CTL_ADD, fd, &event) < 0)) {
		err = errno;
		pr_err("epoll_ctl(EPOLL_CTL_ADD): " PRERF, PREAR(err));
		return -err;
	}
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
	 * TODO: Use cfg to set some socket options.
	 */
	(void)cfg;
	return retval;

out_err:
	err = errno;
	pr_err("setsockopt(tcp_fd, %s, %s): " PRERF, lv, on, PREAR(err));
	return -err;
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
		return -err;
	}

	prl_notice(0, "Setting socket file descriptor up...");
	ret = setup_socket(tcp_fd, state);
	if (unlikely(ret))
		goto out;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(state->cfg->sock.bind_port);
	addr.sin_addr.s_addr = inet_addr(state->cfg->sock.bind_addr);

	ret = bind(tcp_fd, (struct sockaddr *)&addr, addr_len);
	if (unlikely(ret)) {
		err = errno;
		ret = -err;
		pr_err("bind(): " PRERF, PREAR(err));
		goto out;
	}

	ret = listen(tcp_fd, state->cfg->sock.backlog);
	if (unlikely(ret)) {
		err = errno;
		ret = -err;
		pr_err("listen(): " PRERF, PREAR(err));
		goto out;
	}

	ret = epoll_add(state->epl_fd, tcp_fd, EPL_INPUT_EVT);
	if (unlikely(ret))
		goto out;

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


static int resolve_src_info(struct sockaddr_in *addr, char *src_ip,
			    uint16_t *src_port)
{
	*src_port = ntohs(addr->sin_port);
	if (unlikely(convert_addr_ntop(addr, src_ip) == NULL))
		return -EINVAL;

	return 0;
}


static int run_acceptor(int tcp_fd, struct gwbot_state *state)
{
	int ret;
	int chan_fd;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	struct timeval tmvl;

	uint16_t src_port;
	char src_ip[IPV4_L + 1];

	int64_t pop_ret;
	struct gwbot_chan *chan;
	uint32_t epl_map_val;


	memset(&addr, 0, sizeof(addr));
	chan_fd = accept(tcp_fd, (struct sockaddr *)&addr, &addr_len);
	if (unlikely(chan_fd < 0)) {
		ret = errno;
		if (ret != EAGAIN)
			pr_err("accept(): " PRERF, PREAR(ret));
		return -ret;
	}

	if (unlikely(addr_len > sizeof(addr))) {
		pr_err("accept(): " PRERF, PREAR(EOVERFLOW));
		ret = -EOVERFLOW;
		goto out_close;
	}

	ret = resolve_src_info(&addr, src_ip, &src_port);
	if (unlikely(ret))
		goto out_close;


	pop_ret = tss_pop(&state->chan_stk);
	if (unlikely(pop_ret == -1)) {
		pr_err("Channel slot is full. Cannot accept new connection from"
		       " %s:%u", src_ip, src_port);
		ret = -EAGAIN;
		goto out_close;
	}


	epl_map_val = (uint32_t)pop_ret + EPL_MAP_SHIFT;
	if (unlikely(epl_map_val >= EPL_MAP_SIZE)) {
		pr_err("epl_map_val yielded too big value: "
		       "(epl_map_val = %u; max_allowed_size = %u)",
		       epl_map_val, EPL_MAP_SIZE);
		ret = -EAGAIN;
		goto out_close;
	}


	if (unlikely(gettimeofday(&tmvl, NULL) < 0)) {
		ret = errno;
		pr_err("gettimeofday() when accepting %s:%u: " PRERF, src_ip,
		       src_port, PREAR(ret));
		ret = -ret;
		goto out_close;
	}


	ret = epoll_add(state->epl_fd, chan_fd, EPL_INPUT_EVT);
	if (unlikely(ret)) {
		pr_err("Cannot add connection from %s:%u to epoll entries",
		       src_ip, src_port);
		goto out_close;
	}


	chan = &state->chans[pop_ret];
	chan->chan_fd     = chan_fd;
	chan->chan_idx    = (uint16_t)pop_ret;
	chan->accepted_at = tmvl.tv_sec;
	chan->recv_s      = 0;
	chan->src_port    = src_port;
	chan->is_online   = true;
	sane_strncpy(chan->src_ip, src_ip, sizeof(chan->src_ip));
	state->epl_map[chan_fd] = (uint16_t)epl_map_val;
	pr_notice("New connection from %s:%u", src_ip, src_port);
	state->active_chan++;
	return 0;
out_close:
	close(chan_fd);
	return ret;
}


static int handle_tcp_event(int tcp_fd, struct gwbot_state *state,
			    uint32_t revents)
{
	const uint32_t err_mask = EPOLLERR | EPOLLHUP;

	if (unlikely(revents & err_mask)) {
		pr_err("TCP event error");
		return -ENETDOWN;
	}

	return run_acceptor(tcp_fd, state);
}


static void *handle_thread(void *thread_void_p)
{
	uint32_t counter;
	struct gwbot_thread *thread = thread_void_p;
	struct gwbot_state  *state  = thread->state;

	atomic_fetch_add_explicit(&state->online_thread, 1,
				  memory_order_acquire);

	prl_notice(5, "Starting thread %u...", thread->thread_idx);
again:
	gw_mutex_lock(&thread->ev_lock);
	thread->is_online = true;

	counter = 0;
	while (likely(!thread->has_event)) {
		int ret;
		/*
		 * We don't have event to be processed here.
		 */
		ret = gw_cond_timedwait_rel(&thread->ev_cond,
					    &thread->ev_lock, 1);
		if ((ret && counter++ == 300) || state->stop_el)
			goto out;
	}


	/*
	 * Process the event
	 */
	gwbot_event_handler(thread);
	prl_notice(5, "Thread %u has finished its job", thread->thread_idx);

	if (thread->ev_from_sqe) {
		que_node_destroy(thread->sqe_node);
		thread->sqe_node = NULL;
	}

	if (unlikely(state->stop_el))
		goto out;


	thread->has_event = false;
	/*
	 *
	 * Restore the index to the stack.
	 *
	 * We tell the main thread that we don't have
	 * event to be processed. So we accept SQE.
	 *
	 */
	gw_mutex_lock(&state->thread_stk_lock);
	tss_push(&state->thread_stk, thread->thread_idx);
	gw_mutex_unlock(&state->thread_stk_lock);


	gw_mutex_unlock(&thread->ev_lock);
	goto again;
out:
	prl_notice(3, "Worker thread %u is shutting down...",
		   thread->thread_idx);
	gw_mutex_unlock(&thread->ev_lock);
	gw_mutex_destroy(&thread->ev_lock);
	gw_cond_destroy(&thread->ev_cond);
	reset_thread(thread, thread->thread_idx, state);
	atomic_fetch_sub_explicit(&state->online_thread, 1,
				  memory_order_acquire);
	return NULL;
}


static int spawn_thread(struct gwbot_thread *thread)
{
	int ret;
	ret = pthread_create(&thread->thread, NULL, handle_thread, thread);
	if (unlikely(ret)) {
		ret = (ret > 0) ? ret : -ret;
		pr_err("pthread_create(): " PRERF, PREAR(ret));
		return -ret;
	}
	pthread_detach(thread->thread);
	return 0;
}


static int submit_to_thread(struct gwbot_thread *thread, struct gwbot_chan *chan,
			    size_t cpy_len)
{

	gw_mutex_lock(&thread->ev_lock);
	thread->ev_from_sqe = false;
	thread->has_event   = true;
	thread->pkt_len     = cpy_len;
	memcpy(&thread->pkt.pkt, &chan->pkt.pkt, cpy_len);
	thread->pkt.raw_buf[cpy_len] = '\0';

	if (likely(thread->is_online)) {
		/*
		 * Thread is online, let's signal it.
		 */
		gw_cond_signal(&thread->ev_cond);
		gw_mutex_unlock(&thread->ev_lock);
		return 0;
	}
	gw_mutex_unlock(&thread->ev_lock);

	/*
	 * The thread is offline, let's spawn it.
	 */
	return spawn_thread(thread);
}


static int enqueue_to_sqe(struct gwbot_state *state, struct gwbot_chan *chan,
			  size_t cpy_len)
{
	int err;
	uint32_t qcount;
	struct que_node *node;
	void *cpy_src = &chan->pkt.pkt;

	node   = que_enqueue(&state->sqes, cpy_src, cpy_len);
	qcount = que_count(&state->sqes);
	if (unlikely(node == NULL)) {
		err = errno;
		if (err == EAGAIN)
			pr_err("SQE buffer is full (sqe_count: %u)", qcount);
		return -err;
	}

	prl_notice(3, "Saving SQE... (sqe_count: %u)", que_count(&state->sqes));
	return 0;
}


static int submit_event(uint16_t fdata_len, struct gwbot_state *state,
			struct gwbot_chan *chan)
{
	int64_t pop_ret;
	size_t cpy_len = sizeof(fdata_len) + fdata_len;

	/*
	 * Find unused thread
	 */
	gw_mutex_lock(&state->thread_stk_lock);
	pop_ret = tss_pop(&state->thread_stk);
	gw_mutex_unlock(&state->thread_stk_lock);

	if (unlikely(pop_ret == -1)) {
		/*
		 * All threads are busy, let's enqueue this to SQE.
		 */
		return enqueue_to_sqe(state, chan, cpy_len);
	}

	return submit_to_thread(&state->threads[pop_ret], chan, cpy_len);
}


static int handle_client_event3(size_t recv_s, struct gwbot_state *state,
				struct gwbot_chan *chan)
{
	int ret;
	uint16_t fdata_len; /* Expected full data length    */
	uint16_t cdata_len; /* Current received data length */
	struct chan_pkt *pkt;

	pkt = &chan->pkt.pkt;
	chan->recv_s = recv_s;
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
	cdata_len = (uint16_t)(
		(recv_s - offsetof(struct chan_pkt, data)) & 0xffffu
	);

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
		pr_err("Data length is too big: (expected: %u, max_allowed: %zu)",
		       fdata_len, sizeof(pkt->data));
		return -E2BIG;
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
		return -EINVAL;
	}

	pkt->len = fdata_len;
	pkt->data[fdata_len] = '\0';

	ret = submit_event(fdata_len, state, chan);
	if (unlikely(ret)) {
		/*
		 * TODO: Send error message to client
		 */
		return ret;
	}

	return -EINPROGRESS;
out:
	return 0;
}


static int handle_client_event2(int chan_fd, struct gwbot_state *state,
				struct gwbot_chan *chan, uint32_t revents)
{
	int err;
	char *recv_buf;
	size_t recv_s;
	size_t recv_len;
	ssize_t recv_ret;
	const uint32_t err_mask = EPOLLERR | EPOLLHUP;

	if (unlikely((revents & err_mask) || (chan->chan_fd == -1)))
		goto out_close;

	recv_s   = chan->recv_s;
	recv_buf = &chan->pkt.raw_buf[recv_s];
	recv_len = sizeof(chan->pkt.raw_buf) - recv_s;
	recv_ret = recv(chan_fd, recv_buf, recv_len, 0);

	if (unlikely(recv_ret == 0))
		goto out_close;


	if (unlikely(recv_ret < 0)) {
		err = errno;
		if (err == EAGAIN)
			return 0;
		pr_err("recv(): " PRERF, PREAR(err));
		goto out_close;
	}


	prl_notice(5, "recv() %zd bytes from " PRWIU, recv_ret, W_IU(chan));

	recv_s += (size_t)recv_ret;
	if (unlikely(handle_client_event3(recv_s, state, chan) < 0))
		goto out_close;

	return 0;
out_close:
	prl_notice(0, "Closing connection from " PRWIU "...", W_IU(chan));
	tss_push(&state->chan_stk, chan->chan_idx);
	state->epl_map[chan_fd] = EPL_MAP_TO_NOP;
	epoll_delete(state->epl_fd, chan_fd);
	reset_chan(chan, chan->chan_idx);
	close(chan_fd);
	return 0;
}


static int handle_client_event(int chan_fd, struct gwbot_state *state,
			       uint32_t revents)
{
	uint16_t map_to;
	struct gwbot_chan *chan;

	map_to = state->epl_map[chan_fd];
	if (unlikely(map_to == EPL_MAP_TO_NOP)) {
		panic("Bug: epl_map[%d] value is EPL_MAP_TO_NOP", chan_fd);
		abort();
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
	int ret;
	for (int i = 0; i < num_of_events; i++) {
		ret = handle_event(&events[i], state);
		if (unlikely(ret))
			return ret;
	}
	return 0;
}


static int monitor_client_time(struct gwbot_state *state)
{
	int ret;
	time_t diff, now;
	struct timeval tmvl;
	struct gwbot_chan *chans;
	uint16_t thread_c;

	if (unlikely(gettimeofday(&tmvl, NULL) < 0)) {
		ret = errno;
		pr_err("gettimeofday(): " PRERF, PREAR(ret));
		return -ret;
	}

	now      = tmvl.tv_sec;
	chans    = state->chans;
	thread_c = state->cfg->worker.thread_c;
	for (uint16_t i = 0; i < thread_c; i++) {
		struct gwbot_chan *chan = &chans[i];

		if (!chan->is_online)
			continue;

		diff = now - chan->accepted_at;
		if (diff > 10) {
			shutdown(chan->chan_fd, SHUT_RDWR);
			prl_notice(0, "Client " PRWIU
				   " has reached its timeout", W_IU(chan));
		}
	}

	return 0;
}


static void dispatch_sqe(struct gwbot_state *state)
{
	int64_t pop_ret;
	struct que_node *node;
	struct gwbot_thread *thread;
	uint16_t pkt_len;

	gw_mutex_lock(&state->thread_stk_lock);
	pop_ret = tss_pop(&state->thread_stk);
	gw_mutex_unlock(&state->thread_stk_lock);
	if (unlikely(pop_ret == -1)) {
		/*
		 *
		 * We don't have free thread, they're all still busy
		 *
		 */
		return;
	}

	thread = &state->threads[pop_ret];
	prl_notice(4, "Dispatching SQE to thread %u (sqe_count = %u)...",
		   thread->thread_idx, que_count(&state->sqes));


	gw_mutex_lock(&thread->ev_lock);

	node = que_dequeue(&state->sqes);
	if (unlikely(node == NULL)) {
		panic("Bug: dispatch_sqe is called when there is no SQE");
		abort();
	}

	memcpy(&pkt_len, node->data, sizeof(pkt_len));

	thread->ev_from_sqe = true;
	thread->has_event   = true;
	thread->pkt_len     = pkt_len;
	thread->sqe_node    = node;

	if (likely(thread->is_online)) {
		/*
		 * Thread is online, let's signal it.
		 */
		gw_cond_signal(&thread->ev_cond);
		gw_mutex_unlock(&thread->ev_lock);
		return;
	}
	gw_mutex_unlock(&thread->ev_lock);

	/*
	 * The thread is offline, let's spawn it.
	 */
	spawn_thread(thread);
}


static int run_event_loop(struct gwbot_state *state)
{
	int err;
	int ret = 0;
	int timeout; /* in milliseconds */
	int epoll_ret;
	int maxevents = 32;
	int epl_fd = state->epl_fd;
	struct epoll_event events[32];


	while (likely(!state->stop_el)) {

		if (que_count(&state->sqes) > 0) {
			/*
			 * We need to move faster, as we have pending SQE(s).
			 */
			dispatch_sqe(state);
			timeout = 20;
		} else {
			timeout = 500;
		}


		if (tss_count(&state->chan_stk) != state->chan_stk.capacity)
			monitor_client_time(state);

		epoll_ret = epoll_wait(epl_fd, events, maxevents, timeout);
		if (unlikely(epoll_ret == 0)) {
			/*
			 * Epoll reached its timeout.
			 */
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

		ret = handle_events(epoll_ret, events, state);
		if (unlikely(ret)) {
			if (ret == -EAGAIN)
				continue;
			state->stop_el = true;
		}
	}

	return ret;
}


static void close_fds(struct gwbot_state *state)
{
	int tcp_fd = state->tcp_fd;
	int epl_fd = state->epl_fd;
	uint16_t thread_c = state->cfg->worker.thread_c;
	struct gwbot_chan *chans;

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

	chans = state->chans;
	for (uint16_t i = 0; i < thread_c; i++) {
		struct gwbot_chan *chan = &chans[i];
		int chan_fd = chan->chan_fd;

		if (!chan->is_online)
			continue;

		pr_notice("Closing state->chans[%u].chan_fd (%d)", i, chan_fd);
		shutdown(chan_fd, SHUT_RDWR);
		close(chan_fd);
		reset_chan(chan, chan->chan_idx);
	}
}


static void wait_for_threads(struct gwbot_state *state)
{
	uint32_t i, last_i, counter = 1;
	_Atomic(uint32_t) *ot = &state->online_thread;


	i = atomic_load_explicit(ot, memory_order_acquire);
	if (!i)
		return;

	prl_notice(0, "Waiting for %u thread(s) to exit...", i);
	last_i = i;
	while (i > 0) {
		usleep(1000);
		if (counter++ > 300000) {
			/*
			 * Threads won't exit soon, force close!
			 */
			break;
		}

		i = atomic_load_explicit(ot, memory_order_acquire);
		if ((last_i != i) && i) {
			last_i = i;
			prl_notice(0, "Waiting for %u thread(s) to exit...", i);
		}
	}
	usleep(1000000);
}


static void destroy_state(struct gwbot_state *state)
{
	uint16_t thread_c = state->cfg->worker.thread_c;
	struct gwbot_thread *threads = state->threads;

	state->stop_el = true;
	wait_for_threads(state);
	close_fds(state);

	gwbot_module_shutdown(state);

	tss_destroy(&state->chan_stk);
	tss_destroy(&state->thread_stk);
	que_destroy(&state->sqes);
	que_destroy(&state->cqes);

	gw_mutex_destroy(&state->log_lock);
	gw_mutex_destroy(&state->sqe_lock);
	gw_mutex_destroy(&state->cqe_lock);
	gw_mutex_destroy(&state->thread_stk_lock);

	gw_cond_destroy(&state->log_cond);

	for (uint16_t i = 0; i < thread_c; i++) {
		gw_mutex_destroy(&threads[i].ev_lock);
		gw_cond_destroy(&threads[i].ev_cond);
	}

	free(state->epl_map);
	free(state->threads);
	free(state->chans);

	tg_api_global_destroy();
	if (likely(state->cfg->use_db))
		mysql_thread_end();

}


int gwbot_run(struct gwbot_cfg *cfg)
{
	int ret;
	struct gwbot_state *state;

	state = malloc(sizeof(*state));
	if (unlikely(state == NULL)) {
		pr_err("malloc(): " PRERF, PREAR(ENOMEM));
		return -ENOMEM;
	}
	memset(state, 0, sizeof(*state));

	state->cfg = cfg;
	g_state    = state;

	ret = validate_cfg(cfg);
	if (unlikely(ret))
		return ret;

	ret = init_state(state);
	if (unlikely(ret))
		goto out;

	ret = init_logger_thread(state);
	if (unlikely(ret))
		goto out;

	ret = init_epoll(state);
	if (unlikely(ret))
		goto out;

	ret = gwbot_module_startup(state);
	if (unlikely(ret))
		goto out;

	ret = init_socket(state);
	if (unlikely(ret))
		goto out;

	ret = run_event_loop(state);
	prl_notice(0, "Event loop return value = %d", ret);
out:
	destroy_state(state);
	free(state);
	return ret;
}
