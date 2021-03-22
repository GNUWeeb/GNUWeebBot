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


typedef struct _tg_api_smsg {
	int64_t		chat_id;
	uint64_t	reply_to_msg_id;
	const char	*parse_mode;
	const char	*text;
} tg_api_smsg;


int tga_send_msg(tg_api_handle *handle, tg_api_smsg *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__SEND_MESSAGE_H */
