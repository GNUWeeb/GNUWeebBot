// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/get_chat_administrators.h
 *
 *  Get chat administrators API for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API__GET_CHAT_ADMINISTRATORS_H
#define GWBOT__LIB__TG_API__GET_CHAT_ADMINISTRATORS_H

#include <gwbot/lib/tg_api.h>


typedef enum _parse_mode_t {
	PARSE_MODE_OFF = 0,
	PARSE_MODE_HTML = 1,
	PARSE_MODE_MARKDOWN = 2
} parse_mode_t;


struct tga_send_msg {
	int64_t		chat_id;
	uint64_t	reply_to_msg_id;
	const char	*text;
	struct_pad(0, 4);
	parse_mode_t	parse_mode;
};


int ftga_get_chat_administrators(tg_api_handle *handle,
				 struct tga_chat_member **admins);


#define ftga_get_chat_admins ftga_get_chat_administrators


#endif /* #ifndef GWBOT__LIB__TG_API__GET_CHAT_ADMINISTRATORS_H */
