// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/gwbot.c
 *
 *  Event logger GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <gwbot/gwbot.h>
#include <gwbot/lib/que.h>
#include <gwbot/gwthread.h>
#include <gwbot/lib/string.h>
#include <gwbot/event_logger.h>


static __always_inline void *internal_gw_event_logger(struct gwbot_state *state)
{
	atomic_fetch_add_explicit(&state->online_thread, 1,
				  memory_order_acquire);

	/*
	 * Tell the main thread that we have been spawned.
	 */
	gw_mutex_lock(&state->log_lock);
	gw_cond_signal(&state->log_cond);
	gw_mutex_unlock(&state->log_lock);


again:
	gw_mutex_lock(&state->log_lock);
	gw_cond_timedwait_rel(&state->log_cond, &state->log_lock, 1);






	gw_mutex_unlock(&state->log_lock);

	if (!state->stop_el)
		goto again;

	prl_notice(0, "Logger thread is shutting down...");
	atomic_fetch_sub_explicit(&state->online_thread, 1,
				  memory_order_acquire);
	return NULL;
}


void *gw_event_logger(void *state_p)
{
	struct gwbot_state *state = state_p;
	return internal_gw_event_logger(state);
}
