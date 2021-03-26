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


#include <unistd.h>
#include <gwbot/base.h>
#include <json-c/json.h>
#include <gwbot/gwbot.h>
#include <gwbot/lib/tg_event.h>
#include <gwbot/event_handler.h>
#include <gwbot/lib/tg_api/send_message.h>

int gwbot_event_handler(const struct gwbot_thread *thread);

#endif /* #ifndef GWBOT__EVENT_HANDLER_H */
