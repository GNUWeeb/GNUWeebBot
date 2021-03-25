// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/event_handler.h
 *
 *  Hexdump macro for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__EVENT_HANDLER_H
#define GWBOT__EVENT_HANDLER_H

#include <gwbot/base.h>

int gwbot_event_handler(struct gwbot_cfg *cfg, const char *json_str);

#endif /* #ifndef GWBOT__EVENT_HANDLER_H */
