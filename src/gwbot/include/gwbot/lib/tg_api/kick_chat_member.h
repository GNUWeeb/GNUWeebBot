// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/kick_chat_member.h
 *
 *  Send message API for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API__KICK_CHAT_MEMBER_H
#define GWBOT__LIB__TG_API__KICK_CHAT_MEMBER_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_kick_cm_t {
	int64_t		chat_id;
	uint64_t	user_id;
	time_t		until_date;
	bool		revoke_msg;
} tga_kick_cm_t;


int tga_kick_chat_member(tga_handle_t *handle, const tga_kick_cm_t *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__KICK_CHAT_MEMBER_H */
