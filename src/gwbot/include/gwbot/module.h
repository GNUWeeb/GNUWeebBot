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

#define GWMOD(NAME) gnuweb_bot_module_entry_##NAME

#define GWMOD_ENTRY_DECLARE(NAME) \
	int GWMOD(NAME)(const struct gwbot_thread *thread)

#define GWMOD_ENTRY_DEFINE(NAME, ARG) \
	GWMOD(NAME)(ARG)

#define GWMOD_REG(NAME, MASK) {(GWMOD(NAME)), (MASK)}

#define GWMOD_REG_END {NULL, 0}

#endif /* #ifndef GWBOT__MODULE_H_H */
