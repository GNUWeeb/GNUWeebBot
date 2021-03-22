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


struct tg_api_smsg {
	int64_t		chat_id;
	char		*text;
	char		*parse_mode;
	uint64_t	reply_to_message_id;
};


int tga_send_message(struct tg_api_res *res, const struct tg_api_smsg *req);


#endif /* #ifndef GWBOT__LIB__TG_API__SEND_MESSAGE_H */
