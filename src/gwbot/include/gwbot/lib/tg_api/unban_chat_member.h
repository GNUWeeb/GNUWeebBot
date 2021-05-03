// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/lib/tg_api/unban_chat_member.h
 *
 *  Unban chat member API for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__LIB__TG_API__UNBAN_CHAT_MEMBER_H
#define GWBOT__LIB__TG_API__UNBAN_CHAT_MEMBER_H

#include <gwbot/lib/tg_api.h>


typedef struct _tga_unban_cm_t {
	int64_t		chat_id;
	uint64_t	user_id;
	bool		only_if_banned;
} tga_unban_cm_t;


int tga_unban_chat_member(tga_handle_t *handle, const tga_unban_cm_t *ctx);


#endif /* #ifndef GWBOT__LIB__TG_API__UNBAN_CHAT_MEMBER_H */
