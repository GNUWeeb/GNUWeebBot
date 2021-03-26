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
#include <gwbot/sqe.h>
#include <gwbot/base.h>
#include <gwbot/config.h>
#include <gwbot/gwchan.h>
#include <gwbot/tstack.h>

struct gwlock {
	bool			need_destroy;
	pthread_mutex_t		mutex;
};


struct gwcond {
	bool			need_destroy;
	pthread_cond_t		cond;
};


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
	struct gwcond		cond;
	struct gwlock		lock;
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
	struct sqe_master	sqes;

	struct tstack		chan_stack;
	struct gwlock		thread_stk_lock;
	struct tstack		thread_stack;
};


int gwbot_run(struct gwbot_cfg *cfg);

#endif /* #ifndef GWBOT__GWBOT_H */
