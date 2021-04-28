// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/send_message.h
 *
 *  Send message API for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API__SEND_MESSAGE_H
#define GWBOT__LIB__TG_API__SEND_MESSAGE_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_send_msg_t {
	int64_t		chat_id;
	uint64_t	reply_to_msg_id;
	const char	*text;
	parse_mode_t	parse_mode;
} tga_send_msg_t;


int tga_send_msg(tga_handle_t *handle, const tga_send_msg_t *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__SEND_MESSAGE_H */
