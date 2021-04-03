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

#include <gwbot/event_handler.h>


struct module_table {
	int 		(*entry)(const struct gwbot_thread *thread,
				 struct tgev *evt);
	uint64_t	event_mask;
};

#define GWMOD(NAME) gnuweb_bot_module_entry_##NAME

#define GWMOD_ENTRY_DECLARE(NAME) \
	int GWMOD(NAME)(const struct gwbot_thread *thread, struct tgev *evt)

#define GWMOD_ENTRY_DEFINE(NAME, THREAD_ARG, EVT_ARG) \
	GWMOD(NAME)(THREAD_ARG, EVT_ARG)

#define GWMOD_REG(NAME, MASK) {(GWMOD(NAME)), (MASK)}

#define GWMOD_REG_END {NULL, 0}

#endif /* #ifndef GWBOT__MODULE_H */
