// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/send_message.h
 *
 *  String helpers header for TeaVPN2
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API__SEND_MESSAGE_H
#define GWBOT__LIB__TG_API__SEND_MESSAGE_H

#include <gwbot/lib/tg_api.h>


typedef enum _parse_mode_t {
	PARSE_MODE_OFF = 0,
	PARSE_MODE_HTML = 1,
	PARSE_MODE_MARKDOWN = 2
} parse_mode_t;


typedef struct _tg_api_smsg {
	int64_t		chat_id;
	uint64_t	reply_to_msg_id;
	parse_mode_t	parse_mode;
	struct_pad(0, 4);
	const char	*text;
} tg_api_smsg;


int tga_send_msg(tg_api_handle *handle, tg_api_smsg *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__SEND_MESSAGE_H */
