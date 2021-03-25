// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/event_handler.h
 *
 *  Hexdump macro for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__MODULE_H_H
#define GWBOT__MODULE_H_H

#include <gwbot/event_handler.h>

#define GWMOD(MODULE_NAME) gnuweb_bot_module_entry_##MODULE_NAME
#define GWMOD_ENTRY_DEF(MODULE_NAME) \
	int gnuweb_bot_module_entry_##MODULE_NAME(	\
		const struct gwbot_thread *thread)

#endif /* #ifndef GWBOT__MODULE_H_H */
