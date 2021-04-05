// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/gwbot.c
 *
 *  Event logger GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <gwbot/gwbot.h>
#include <gwbot/gwchan.h>
#include <gwbot/lib/sqe.h>
#include <gwbot/gwthread.h>
#include <gwbot/lib/string.h>
#include <gwbot/event_handler.h>

void *gw_event_logger(struct gwbot_state *state)
{
	gw_mutex_lock(&state->cqe_lock);

	while (sqe_count(&state->cqes) == 0) {
		gw_cond_wait(&state->cqe_cond, &state->cqe_lock);
	}

	gw_mutex_unlock(&state->cqe_lock);
	return NULL;
}
