// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/gwbot.h
 *
 *  
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__GWBOT_H
#define GWBOT__GWBOT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <gwbot/base.h>
#include <gwbot/config.h>
#include <gwbot/gwthread.h>
#include <gwbot/lib/que.h>
#include <gwbot/lib/tstack.h>

#define QWE 0
#if QWE
struct gwbot_thread {
	time_t			started_at;
	uint32_t		thread_idx;
	bool			is_online;
	bool			has_event;
	struct_pad(0, 2);
	pthread_t		thread;
	struct gwbot_state	*state;
	union {
		struct chan_pkt		pkt;
		char			raw_buf[sizeof(struct chan_pkt)];
	} uni_pkt;
	gwcond_t		ev_cond;
	gwlock_t		ev_lock;
};


struct gwbot_state {
	bool			stop_el;
	struct_pad(0, 3);

	int			intr_sig;
	int			tcp_fd;
	int			epl_fd;

	struct gwbot_cfg	*cfg;
	uint16_t		*epl_map_chan;
	struct gwchan		*chans;
	struct gwbot_thread	*threads;
	struct que_master	ques;
	struct que_master	cqes;
	gwlock_t		cqe_lock;
	gwcond_t		cqe_cond;
	gwlock_t		log_lock;
	pthread_t		log_thread;

	struct tstack		chan_stack;
	gwlock_t		thread_stk_lock;
	struct tstack		thread_stack;
};
#endif

#define CHPKT_DATA_BUFSIZ 0x6000ull

struct chan_pkt {
	uint16_t		len;
	char			data[CHPKT_DATA_BUFSIZ];
};

static_assert(CHPKT_DATA_BUFSIZ <= 0xffffull, "Bad struct chan_pkt");


union chan_pkt_uni {
	char			raw_buf[sizeof(struct chan_pkt)];
	struct chan_pkt		pkt;
};


struct gwbot_chan {
	bool			is_online;
	int			chan_fd;
	uint16_t		chan_idx;
	uint16_t		src_port;
	char			src_ip[IPV4_L + 1];
	time_t			accepted_at;
	size_t			recv_s;
	union chan_pkt_uni	pkt;
};


struct gwbot_thread {
	struct gwbot_state	*state;
	time_t			last_work;
	pthread_t		thread;
	gwcond_t		ev_cond;
	gwlock_t		ev_lock;
	uint16_t		thread_idx;
	bool			is_online;
	bool			has_event;
	bool			ev_from_sqe;
	size_t			pkt_len;
	struct que_node		*sqe_node; /* Zero copy from SQE */
	union chan_pkt_uni	pkt;
};


struct gwbot_state {
	bool			stop_el;  /* Stop event loop  */
	int			intr_sig; /* Interrupt signal */

	int			tcp_fd;
	int			epl_fd;

	struct gwbot_cfg	*cfg;
	uint16_t		*epl_map; /* Epoll map        */
	struct gwbot_chan 	*chans;   /* Channel array    */
	struct gwbot_thread	*threads; /* Thread array     */
	uint32_t		active_chan;

	gwlock_t		sqe_lock;
	struct que_master	sqes;
	gwlock_t		cqe_lock;
	struct que_master	cqes;
	pthread_t		log_thread;
	gwlock_t		log_lock;
	gwcond_t		log_cond;

	struct tstack		chan_stk;     /* Channel stack (no lock) */
	struct tstack		thread_stk;   /* Thread stack            */
	gwlock_t		thread_stk_lock;
	_Atomic(uint32_t)	online_thread;
};


int gwbot_run(struct gwbot_cfg *cfg);


#endif /* #ifndef GWBOT__GWBOT_H */
