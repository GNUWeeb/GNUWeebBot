// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/pin_chat_msg.h
 *
 *  Pin chat message API for GNUWeebBot
 *
 *  Copyright (C) 2021  Komori Kuzuyu
 */

#ifndef GWBOT__LIB__TG_API__PIN_CHAT_MESSAGE_H
#define GWBOT__LIB__TG_API__PIN_CHAT_MESSAGE_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_pin_cm_t {
	int64_t		chat_id;
	uint64_t	message_id;
	bool		disable_notification;
} tga_pin_cm_t;


int tga_pin_chat_msg(tga_handle_t *handle, const tga_pin_cm_t *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__PIN_CHAT_MESSAGE_H */
