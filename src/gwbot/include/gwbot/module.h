// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/event_handler.h
 *
 *  Hexdump macro for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__MODULE_H
#define GWBOT__MODULE_H

#include <gwbot/gwbot.h>
#include <gwbot/event_handler.h>


struct module_entry {
	const char	*mod_name;
	int		(*startup)(struct gwbot_state *state);
	int		(*shutdown)(struct gwbot_state *state);
	int 		(*entry)(const struct gwbot_thread *thread,
				 struct tgev *evt);
	uint64_t	event_mask;
};


#define GWMOD(NAME, PREFIX) gw_module_##PREFIX##__##NAME

#define GWMOD_NAME_DECLARE(NAME)	\
	extern const char GWMOD(NAME, mod_name)[]

#define GWMOD_STARTUP_DECLARE(NAME)	\
	int GWMOD(NAME, startup)(struct gwbot_state *state)

#define GWMOD_SHUTDOWN_DECLARE(NAME)	\
	int GWMOD(NAME, shutdown)(struct gwbot_state *state)

#define GWMOD_ENTRY_DECLARE(NAME)	\
	int GWMOD(NAME, entry)(const struct gwbot_thread *thread,	\
			       struct tgev *evt)

#define GWMOD_NAME_DEFINE(NAME, MOD_DESC_NAME)	\
	const char GWMOD(NAME, mod_name)[] = MOD_DESC_NAME

#define GWMOD_STARTUP_DEFINE(NAME, STATE_ARG)	\
	GWMOD(NAME, startup)(__maybe_unused STATE_ARG)

#define GWMOD_SHUTDOWN_DEFINE(NAME, STATE_ARG)	\
	GWMOD(NAME, shutdown)(__maybe_unused STATE_ARG)

#define GWMOD_ENTRY_DEFINE(NAME, THREAD_ARG, TGEV_ARG)	\
	GWMOD(NAME, entry)(__maybe_unused THREAD_ARG, __maybe_unused TGEV_ARG)

#define GWMOD_REG(NAME, MASK)	\
{				\
	GWMOD(NAME, mod_name),	\
	GWMOD(NAME, startup),	\
	GWMOD(NAME, shutdown),	\
	GWMOD(NAME, entry),	\
	(MASK)			\
}

#define GWMOD_REG_END {NULL, NULL, NULL, NULL, 0}

extern const struct module_entry mod_tbl[];

#endif /* #ifndef GWBOT__MODULE_H */
